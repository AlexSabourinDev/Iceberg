// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#include "ib_rendergraph.h"
#include <string.h>

typedef struct
{
	ibr_ResourceStateRange States;
	ib_range(VkImageMemoryBarrier2) OutImageBarriers;
	uint32_t* OutImageBarrierCount;
	ib_range(VkBufferMemoryBarrier2) OutMemoryBarriers;
	uint32_t* OutMemoryBarrierCount;
} CreatePipelineBarriersDesc;

static void pushProfilingScope(ibr_RenderGraph* graph, VkCommandBuffer cmd, char const* passName)
{
	ibr_ProfilingScope* scope = ib_fsqPush(&graph->ActiveProfilingScopes);
	*scope = (ibr_ProfilingScope)
	{
		.Timer = ib_beginTimer(&graph->TimerManager, cmd),
		.Name = passName,
	};

	graph->CurrentProfilingDepth++;
	ib_assert(graph->CurrentProfilingDepth == 1); // We don't currently support nested scopes.
}

static void popProfilingScope(ibr_RenderGraph* graph, VkCommandBuffer cmd)
{
	ibr_ProfilingScope* currentScope = ib_fsqPeek(&graph->ActiveProfilingScopes);
	ib_assert(currentScope != NULL);
	ib_endTimer(&graph->TimerManager, cmd, &currentScope->Timer);

	ibr_ProfilingScope* completedScope = ib_fsqPush(&graph->CompletedScopes);
	*completedScope = *currentScope;

	ib_fsqPop(&graph->ActiveProfilingScopes);
	graph->CurrentProfilingDepth--;
}

static void getAcquireAndReleaseMask(ibr_ResourceState state, VkPipelineStageFlags* acquire, VkPipelineStageFlags* release)
{
	bool acquireAndReleaseMaskValid = state.AcquireAndReleaseStageMask != 0;
	bool acquireOrReleaseMaskValid = state.AcquireStageMask != 0 || state.ReleaseStageMask != 0;

	// At least one, but not both, have to be valid. (I.e. xor)
	ib_potentiallyUnused(acquireAndReleaseMaskValid);
	ib_assert(acquireAndReleaseMaskValid ^ acquireOrReleaseMaskValid);

	if (acquireOrReleaseMaskValid)
	{
		ib_assert(state.AcquireStageMask != 0 && state.ReleaseStageMask != 0);
		*acquire = state.AcquireStageMask;
		*release = state.ReleaseStageMask;
	}
	else
	{
		*acquire = state.AcquireAndReleaseStageMask;
		*release = state.AcquireAndReleaseStageMask;
	}
}

static void createPipelineBarriers(ib_Core* core, CreatePipelineBarriersDesc desc)
{
	uint32_t imageBarrierCount = 0;
	uint32_t memoryBarrierCount = 0;

	for(uint32_t i = 0; i < desc.States.Count; i++)
	{
		ibr_ResourceState state = desc.States.Data[i];

		VkPipelineStageFlags acquireStageMask;
		VkPipelineStageFlags releaseStageMask;
		getAcquireAndReleaseMask(state, &acquireStageMask, &releaseStageMask);

		if(state.Resource->Type == ibr_ResourceType_Texture)
		{
			ib_assert(imageBarrierCount < desc.OutMemoryBarriers.Count);
			if(imageBarrierCount < desc.OutMemoryBarriers.Count)
			{
				desc.OutImageBarriers.Data[imageBarrierCount] = ib_createTextureBarrier(core, (ib_TextureBarrierDesc)
				{
					.Texture = state.Resource->Texture,
					.OldLayout = state.Resource->TextureLayout,
					.NewLayout = state.Layout,
					.SourceStageMask = state.Resource->LastReleaseStageMask,
					.DestStageMask = acquireStageMask,
					.SourceAccessMask = state.Resource->LastReleaseAccessMask,
					.DestAccessMask = state.AcquireAccessMask
				});
				imageBarrierCount++;

				// ASSUMPTION: Assume that what where we acquire is where we release for now.
				state.Resource->LastReleaseAccessMask = state.ReleaseAccessMask;
				state.Resource->LastReleaseStageMask = releaseStageMask;
				state.Resource->TextureLayout = state.Layout;
			}
		}
		else if(state.Resource->Type == ibr_ResourceType_Buffer)
		{
			ib_assert(memoryBarrierCount < desc.OutMemoryBarriers.Count);
			if(memoryBarrierCount < desc.OutMemoryBarriers.Count)
			{
				desc.OutMemoryBarriers.Data[memoryBarrierCount] = (VkBufferMemoryBarrier2)
				{
					.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
					.buffer = state.Resource->Buffer->VulkanBuffer,
					.size = VK_WHOLE_SIZE,
					.srcStageMask = state.Resource->LastReleaseStageMask,
					.dstStageMask = acquireStageMask,
					.srcAccessMask = state.Resource->LastReleaseAccessMask,
					.dstAccessMask = state.AcquireAccessMask,
				};
				memoryBarrierCount++;

				// ASSUMPTION: Assume that what where we acquire is where we release for now.
				state.Resource->LastReleaseAccessMask = state.ReleaseAccessMask;
				state.Resource->LastReleaseStageMask = releaseStageMask;
			}
		}
		else
		{
			ib_assert(false);
		}
	}
	*desc.OutImageBarrierCount = imageBarrierCount;
	*desc.OutMemoryBarrierCount = memoryBarrierCount;
}

ibr_RenderGraphPool ibr_allocRenderGraphPool(ib_Core* core)
{
	ibr_RenderGraphPool pool = (ibr_RenderGraphPool){0};
	for(uint32_t i = 0; i < ib_FramebufferCount; i++)
	{
		pool.Graphs[i].Core = core;

		ib_initTimerManager((ib_TimerManagerDesc)
							{
								.Core = core,
								.MaxTimerCount = 1024,
							}, &pool.Graphs[i].TimerManager);

		// Create the descriptor pools
		{
			uint32_t const maxTransientDescriptorTypeCount = 128;
			uint32_t const maxTransientDescriptorSetCount = 128;
			VkDescriptorPoolSize descriptorPoolSizes[] = 
			{
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxTransientDescriptorTypeCount},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, maxTransientDescriptorTypeCount},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxTransientDescriptorTypeCount},
				{VK_DESCRIPTOR_TYPE_SAMPLER, maxTransientDescriptorTypeCount},
				{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, maxTransientDescriptorTypeCount},
			};

			VkDescriptorPoolCreateInfo descriptorPoolCreate =
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.flags = 0,
				.maxSets = maxTransientDescriptorSetCount,
				.poolSizeCount = ib_arrayCount(descriptorPoolSizes),
				.pPoolSizes = descriptorPoolSizes
			};

			ib_vkCheck(vkCreateDescriptorPool(core->LogicalDevice, &descriptorPoolCreate, ib_NoVkAllocator, &pool.Graphs[i].TransientDescriptorPool));
		}

		for (uint32_t q = 0; q < ib_Queue_Count; q++)
		{
			VkCommandPoolCreateInfo commandPoolCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
				.queueFamilyIndex = core->Queues[q].Index,
			};

			ib_vkCheck(vkCreateCommandPool(core->LogicalDevice, &commandPoolCreateInfo, ib_NoVkAllocator, &pool.Graphs[i].TransientCommandPools[q]));
			
			// Allocate all our command buffers
			ib_allocCommandBuffers(core, (ib_AllocCommandBuffersDesc)
								   {
									   .OutCommandBuffers = ib_staticArrayRange(pool.Graphs[i].TransientCommandBuffers[q]),
									   .Queue = q,
									   .Pool = pool.Graphs[i].TransientCommandPools[q]
								   });
			pool.Graphs[i].ActiveCommandBuffers[q] = 0;
		}

		ib_vkCheck(vkCreateSemaphore(core->LogicalDevice, &(VkSemaphoreCreateInfo)
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		}, ib_NoVkAllocator, &pool.Graphs[i].FrameSemaphore));

		ib_vkCheck(vkCreateFence(core->LogicalDevice, &(VkFenceCreateInfo)
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		}, ib_NoVkAllocator, &pool.Graphs[i].FrameFence));
	}
	return pool;
}

void ibr_freeRenderGraphPool(ib_Core* core, ibr_RenderGraphPool* pool)
{
	ib_potentiallyUnused(core);

	for(uint32_t i = 0; i < ib_FramebufferCount; i++)
	{
		ibr_RenderGraph* graph = &pool->Graphs[i];

		for(uint32_t rt = 0; rt < graph->TransientTextures.Count; rt++)
		{
			ib_freeTexture(graph->Core, &graph->TransientTextures.Data[rt]);
		}
		ib_fsqClear(&graph->TransientTextures);

		for(uint32_t b = 0; b < graph->TransientBuffers.Count; b++)
		{
			ib_freeBuffer(graph->Core, &graph->TransientBuffers.Data[b]);
		}
		ib_fsqClear(&graph->TransientBuffers);

		for(uint32_t b = 0; b < graph->TransientImageViews.Count; b++)
		{
			vkDestroyImageView(graph->Core->LogicalDevice, graph->TransientImageViews.Data[b], ib_NoVkAllocator);
		}
		ib_fsqClear(&graph->TransientImageViews);

		vkDestroyDescriptorPool(core->LogicalDevice, graph->TransientDescriptorPool, ib_NoVkAllocator);

		for (uint32_t q = 0; q < ib_Queue_Count; q++)
		{
			vkDestroyCommandPool(core->LogicalDevice, graph->TransientCommandPools[q], ib_NoVkAllocator);
		}

		vkDestroyFence(core->LogicalDevice, graph->FrameFence, ib_NoVkAllocator);
		vkDestroySemaphore(core->LogicalDevice, graph->FrameSemaphore, ib_NoVkAllocator);

		ib_killTimerManager(core, &graph->TimerManager);
	}
}

ibr_RenderGraph* ibr_beginFrame(ibr_RenderGraphPool* pool, ibr_BeginFrameDesc desc)
{
	ibr_RenderGraph* graph = &pool->Graphs[desc.FrameIndex];

	// Fence is signaled - our resources are free, we're good to go!
	ib_vkCheck(vkWaitForFences(graph->Core->LogicalDevice, 1, &graph->FrameFence, VK_TRUE, UINT64_MAX));

	for(uint32_t i = 0; i < graph->TransientTextures.Count; i++)
	{
		ib_freeTexture(graph->Core, &graph->TransientTextures.Data[i]);
	}
	ib_fsqClear(&graph->TransientTextures);

	for(uint32_t b = 0; b < graph->TransientBuffers.Count; b++)
	{
		ib_freeBuffer(graph->Core, &graph->TransientBuffers.Data[b]);
	}
	ib_fsqClear(&graph->TransientBuffers);

	for(uint32_t b = 0; b < graph->TransientImageViews.Count; b++)
	{
		vkDestroyImageView(graph->Core->LogicalDevice, graph->TransientImageViews.Data[b], ib_NoVkAllocator);
	}
	ib_fsqClear(&graph->TransientImageViews);

	vkResetDescriptorPool(graph->Core->LogicalDevice, graph->TransientDescriptorPool, 0);
	for (uint32_t q = 0; q < ib_Queue_Count; q++)
	{
		vkResetCommandPool(graph->Core->LogicalDevice, graph->TransientCommandPools[q], 0);
		graph->ActiveCommandBuffers[q] = 0;
	}

	if (desc.Surface != NULL)
	{
		ib_PrepareSurfaceResult prepareResult = ib_prepareSurface(graph->Core, (ib_PrepareSurfaceDesc) {
			desc.Surface, desc.FrameIndex
		});

		// Couldn't prepare our surface.
		if (prepareResult.SurfaceState != ib_SurfaceState_Ok)
		{
			return NULL;
		}

		graph->SwapchainTextureIndex = prepareResult.SwapchainTextureIndex;
		graph->SwapchainTexture = &desc.Surface->SwapchainTextures[graph->SwapchainTextureIndex];
		graph->ScreenExtent = (VkExtent2D){ graph->SwapchainTexture->Extent.width, graph->SwapchainTexture->Extent.height };
	}
	else
	{
		graph->ScreenExtent = (VkExtent2D){ 1, 1 };
		graph->SwapchainTexture = NULL;
	}

	// Only reset our fence if we know we're going to signal it.
	// If we made it this far, we're good to go.
	ib_vkCheck(vkResetFences(graph->Core->LogicalDevice, 1, &graph->FrameFence));

	// Convert our timers to timings from the previous frame
	{
		ib_fsqClear(&graph->PreviousFrameTimings);
		for (uint32_t i = 0; i < graph->CompletedScopes.Count; i++)
		{
			bool isBlocking = false;
			ibr_ProfilingScope* scope = &graph->CompletedScopes.Data[i];
			ibr_ScopeTiming* timing = ib_fsqPush(&graph->PreviousFrameTimings);
			*timing = (ibr_ScopeTiming)
			{
				.Timing = ib_queryTimer(graph->Core, &graph->TimerManager, &scope->Timer, isBlocking),
				.Name = scope->Name,
			};
			ib_assert(timing->Timing != ib_TimerQueryNotReady); // We should be ready, our frame's fence was signaled.
		}
		ib_fsqClear(&graph->CompletedScopes);
	}
	ib_resetTimersCPU(graph->Core, &graph->TimerManager);

	return graph;
}

void ibr_endFrame(ibr_RenderGraphPool* pool, ibr_RenderGraph* graph)
{
	ib_potentiallyUnused(pool);
	ib_potentiallyUnused(graph);
	ib_assert(graph->ActiveProfilingScopes.Count == 0);
}

ibr_Resource ibr_allocPassResource(ibr_RenderGraph* graph, ibr_ResourceDesc resourceDesc)
{
	ibr_Resource outResource = (ibr_Resource){0};
	outResource.Type = resourceDesc.Type;
	outResource.LastReleaseStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	if(resourceDesc.Type == ibr_ResourceType_Texture)
	{
		outResource.TextureLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if((resourceDesc.Flags & ibr_ResourceFlag_Transient) != 0)
		{
			ib_TextureDesc textureDesc = resourceDesc.TextureDesc;
			if(memcmp(&textureDesc.Extent, &ibr_ScreenExtent, sizeof(VkExtent2D)) == 0)
			{
				textureDesc.Extent = (VkExtent3D){ graph->ScreenExtent.width, graph->ScreenExtent.height };
			}
			ib_Texture* texture = ib_fsqPush(&graph->TransientTextures);
			*texture = ib_allocTexture(graph->Core, textureDesc);

			// We just wrote to our texture, its layout will be transfer dest.
			if (textureDesc.InitialWrite.Data != NULL)
			{
				outResource.TextureLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
			}

			outResource.Texture = texture;
		}
		else
		{
			outResource.Texture = resourceDesc.Texture;
		}
	}
	else if(resourceDesc.Type == ibr_ResourceType_Buffer)
	{
		if((resourceDesc.Flags & ibr_ResourceFlag_Transient) != 0)
		{
			ib_BufferDesc bufferDesc = resourceDesc.BufferDesc;

			ib_Buffer* buffer = ib_fsqPush(&graph->TransientBuffers);
			*buffer = ib_allocBuffer(graph->Core, bufferDesc);

			outResource.Buffer = buffer;
		}
		else
		{
			outResource.Buffer = resourceDesc.Buffer;
		}
	}

	return outResource;
}

void ibr_allocPassResources(ibr_RenderGraph* graph, ibr_AllocPassResourcesDesc desc)
{
	for(uint32_t i = 0; i < desc.ResourceBindings.Count; i++)
	{
		ibr_ResourceDesc resourceDesc = desc.ResourceBindings.Data[i].Desc;
		ibr_Resource* outResource = desc.ResourceBindings.Data[i].OutResource;
		*outResource = ibr_allocPassResource(graph, resourceDesc);
	}
}

VkImageView ibr_allocTransientImageView(ibr_RenderGraph* graph, ibr_AllocTransientImageViewDesc desc)
{
	VkImageView* view = ib_fsqPush(&graph->TransientImageViews);

	VkImageViewCreateInfo imageCreateViewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = desc.Texture->Image,
		.viewType = desc.Texture->Extent.depth > 0 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
		.format = desc.Texture->Format,
		.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
		.subresourceRange = 
		{
			.aspectMask = desc.Texture->Aspect,
			.levelCount = 1,
			.layerCount = 1,
			.baseArrayLayer = desc.LayerIndex,
			.baseMipLevel = desc.BaseMip,
		},
	};

	ib_vkCheck(vkCreateImageView(graph->Core->LogicalDevice, &imageCreateViewInfo, ib_NoVkAllocator, view));

	return *view;
}

ib_ShaderInput ibr_allocTransientShaderInput(ibr_RenderGraph* graph, ib_AllocShaderInputDesc desc)
{
	ib_assert(desc.Pool == VK_NULL_HANDLE);
	desc.Pool = graph->TransientDescriptorPool;

	return ib_allocShaderInput(graph->Core, desc);
}

ib_ShaderInput ibr_resourcesToShaderInput(ibr_RenderGraph* graph, ibr_ResourceToShaderInputDesc desc)
{
	ib_assert(desc.Resources.Count <= desc.ShaderInputs.Count); // Can't have more resources than inputs

	// TODO: Can use a CPU stack allocator for this in the future.
#define maxShaderInputs 32
	ib_ShaderInputWrite writes[maxShaderInputs];
	ib_assert(desc.ShaderInputs.Count <= maxShaderInputs);
#undef maxShaderInputs

	uint32_t writeCount = 0;
	for (uint32_t i = 0; i < desc.Resources.Count; i++)
	{
		ibr_Resource* resource = desc.Resources.Data[i];
		if (resource != NULL) // resource array is allowed to be sparse. Just pass in the resources you care about at the right indices.
		{
			ib_ShaderInputWrite inputWrite = { .Desc = &desc.ShaderInputs.Data[i] };
			ib_assert(inputWrite.Desc->Index == i); // We're expecting our shader input index to match their in-array location.
			if (resource->Type == ibr_ResourceType_Texture)
			{
				inputWrite.TextureInput = (ib_ShaderInputWriteTexture) { resource->Texture, resource->TextureLayout };
			}
			else if (resource->Type == ibr_ResourceType_Buffer)
			{
				inputWrite.BufferInput = (ib_ShaderInputWriteBuffer) { resource->Buffer };
			}

			writes[writeCount++] = inputWrite;
		}
	}

	return ibr_allocTransientShaderInput(graph, (ib_AllocShaderInputDesc)
										 {
											 .Layout = desc.Layout,
											 .Inputs = { writes, writeCount }
										 });
}

VkCommandBuffer ibr_allocTransientCommandBuffer(ibr_RenderGraph* graph, ib_Queue queue)
{
	ib_assert(graph->ActiveCommandBuffers[queue] < ibr_MaxTransientCommandBuffer);
	return graph->TransientCommandBuffers[queue][graph->ActiveCommandBuffers[queue]++];
}

void ibr_beginGraphicsPass(ibr_RenderGraph* graph, VkCommandBuffer cmd, ibr_BeginGraphicsPassDesc desc)
{
	pushProfilingScope(graph, cmd, desc.PassName);

	char const* passDebugLabel = desc.PassName == NULL
		? "Unnamed Graphic Pass"
		: desc.PassName;

	VkDebugUtilsLabelEXT passDebugLabelInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = passDebugLabel,
	};

	vkCmdBeginDebugUtilsLabelEXT(cmd, &passDebugLabelInfo);

	// Barriers
	{
		uint32_t totalResourceCount = desc.RenderTargets.Count + desc.OtherResourceStates.Count;
		if(desc.DepthTarget.Resource != NULL)
		{
			totalResourceCount++;
		}

		// Transition for write
		#define maxBarrierCount 32
		ib_assert(totalResourceCount < maxBarrierCount);
		VkImageMemoryBarrier2 imageMemoryBarriers[maxBarrierCount];
		VkBufferMemoryBarrier2 memoryBarriers[maxBarrierCount];
		#undef maxBarrierCount

		uint32_t imageBarrierCount;
		uint32_t memoryBarrierCount;
		createPipelineBarriers(graph->Core, (CreatePipelineBarriersDesc)
			{
				desc.OtherResourceStates,
				ib_staticArrayRange(imageMemoryBarriers),
				&imageBarrierCount,
				ib_staticArrayRange(memoryBarriers),
				&memoryBarrierCount
			});

		for(uint32_t i = 0; i < desc.RenderTargets.Count; i++)
		{
			ibr_RenderTargetState state = desc.RenderTargets.Data[i];

			ib_assert(state.Resource->Type == ibr_ResourceType_Texture);
			imageMemoryBarriers[imageBarrierCount + i] = ib_createTextureBarrier(graph->Core, (ib_TextureBarrierDesc)
			{
				.Texture = state.Resource->Texture,
				.OldLayout = state.Resource->TextureLayout,
				.NewLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.SourceStageMask = state.Resource->LastReleaseStageMask,
				.DestStageMask = state.AcquireStageMask != VK_PIPELINE_STAGE_NONE ? state.AcquireStageMask : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.SourceAccessMask = state.Resource->LastReleaseAccessMask,
				.DestAccessMask = state.AcquireAccessMask != VK_ACCESS_NONE ? state.AcquireAccessMask : VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
			});

			// ASSUMPTION: Assume that what where we acquire is where we release for now.
			state.Resource->LastReleaseAccessMask = state.ReleaseAccessMask != VK_ACCESS_NONE ? state.ReleaseAccessMask : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			state.Resource->LastReleaseStageMask = state.ReleaseStageMask != VK_PIPELINE_STAGE_NONE ? state.ReleaseStageMask : VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			state.Resource->TextureLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		imageBarrierCount += desc.RenderTargets.Count;

		// Depth Target
		if(desc.DepthTarget.Resource != NULL)
		{
			ibr_RenderTargetState state = desc.DepthTarget;

			ib_assert(state.Resource->Type == ibr_ResourceType_Texture);
			imageMemoryBarriers[imageBarrierCount] = ib_createTextureBarrier(graph->Core, (ib_TextureBarrierDesc)
			{
				.Texture = state.Resource->Texture,
				.OldLayout = state.Resource->TextureLayout,
				.NewLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				.SourceStageMask = state.Resource->LastReleaseStageMask,
				.DestStageMask = state.AcquireStageMask != VK_PIPELINE_STAGE_NONE ? state.AcquireStageMask : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.SourceAccessMask = state.Resource->LastReleaseAccessMask,
				.DestAccessMask = state.AcquireAccessMask != VK_ACCESS_NONE ? state.AcquireAccessMask : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
			});

			// ASSUMPTION: Assume that what where we acquire is where we release for now.
			state.Resource->LastReleaseAccessMask = state.ReleaseAccessMask != VK_ACCESS_NONE ? state.ReleaseAccessMask : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			state.Resource->LastReleaseStageMask = state.ReleaseStageMask != VK_PIPELINE_STAGE_NONE ? state.ReleaseStageMask : VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			state.Resource->TextureLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			imageBarrierCount++;
		}

		vkCmdPipelineBarrier2(cmd, &(VkDependencyInfo)
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = imageBarrierCount,
			.pImageMemoryBarriers = imageMemoryBarriers,
			.bufferMemoryBarrierCount = memoryBarrierCount,
			.pBufferMemoryBarriers = memoryBarriers
		});
	}

	VkExtent2D extents = { 0 };
	if(desc.RenderTargets.Count > 0)
	{
		extents = (VkExtent2D){ desc.RenderTargets.Data[0].Resource->Texture->Extent.width, desc.RenderTargets.Data[0].Resource->Texture->Extent.height };
	}
	else
	{
		extents = (VkExtent2D){ desc.DepthTarget.Resource->Texture->Extent.width, desc.DepthTarget.Resource->Texture->Extent.height };
	}

	// Attachments
	{
		#define maxColorAttachments 16
		VkRenderingAttachmentInfo colorAttachments[maxColorAttachments];
		#undef maxColorAttachments

		for(uint32_t i = 0; i < desc.RenderTargets.Count; i++)
		{
			ibr_RenderTargetState state = desc.RenderTargets.Data[i];
			colorAttachments[i] = (VkRenderingAttachmentInfo)
			{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.imageView = state.Resource->Texture->View,
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.loadOp = state.LoadOp,
				.storeOp = state.StoreOp,
				.clearValue = state.ClearValue
			};
		}

		VkRenderingAttachmentInfo depthAttachment = { 0 };
		if(desc.DepthTarget.Resource != NULL)
		{
			depthAttachment = (VkRenderingAttachmentInfo)
			{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.imageView = desc.DepthTarget.Resource->Texture->View,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				.loadOp = desc.DepthTarget.LoadOp,
				.storeOp = desc.DepthTarget.StoreOp,
				.clearValue = desc.DepthTarget.ClearValue
			};
		}

		VkRenderingInfo renderInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = {.extent = extents },
			.layerCount = 1,
			.colorAttachmentCount = desc.RenderTargets.Count,
			.pColorAttachments = colorAttachments,
			.pDepthAttachment = (desc.DepthTarget.Resource != NULL) ? &depthAttachment : NULL
		};

		vkCmdBeginRendering(cmd, &renderInfo);
	}

	if(desc.MinDepth == 0.0f && desc.MaxDepth == 0.0f)
	{
		desc.MaxDepth = 1.0f;
	}

	vkCmdSetViewport(cmd, 0, 1, &(VkViewport)
	{
		.width = (float)extents.width, .height = (float)extents.height,
		.minDepth = desc.MinDepth, .maxDepth = desc.MaxDepth,
	});

	vkCmdSetScissor(cmd, 0, 1, &(VkRect2D){.extent = extents});
}

void ibr_endGraphicsPass(ibr_RenderGraph* graph, VkCommandBuffer cmd)
{
	ib_potentiallyUnused(graph);
	vkCmdEndRendering(cmd);
	popProfilingScope(graph, cmd);
	vkCmdEndDebugUtilsLabelEXT(cmd);
}

void ibr_barriers(ibr_RenderGraph* graph, VkCommandBuffer cmd, ibr_BarriersDesc desc)
{
	// TODO: Can use a CPU stack allocator for this.
	#define maxBarrierCount 32
	VkImageMemoryBarrier2 imageMemoryBarriers[maxBarrierCount];
	VkBufferMemoryBarrier2 memoryBarriers[maxBarrierCount];
	#undef maxImageBarrierCount

	uint32_t imageBarrierCount;
	uint32_t memoryBarrierCount;
	createPipelineBarriers(graph->Core,
		(CreatePipelineBarriersDesc)
		{
			desc.ResourceStates,
			ib_staticArrayRange(imageMemoryBarriers),
			&imageBarrierCount,
			ib_staticArrayRange(memoryBarriers),
			&memoryBarrierCount
		});

	vkCmdPipelineBarrier2(cmd, &(VkDependencyInfo)
	{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = imageBarrierCount,
		.pImageMemoryBarriers = imageMemoryBarriers,
		.bufferMemoryBarrierCount = memoryBarrierCount,
		.pBufferMemoryBarriers = memoryBarriers
	});
}

void ibr_beginComputePass(ibr_RenderGraph* graph, VkCommandBuffer cmd, ibr_BeginComputePassDesc desc)
{
	pushProfilingScope(graph, cmd, desc.PassName);

	char const* passDebugLabel = desc.PassName == NULL
		? "Unnamed Compute Pass"
		: desc.PassName;

	VkDebugUtilsLabelEXT passDebugLabelInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pLabelName = passDebugLabel,
	};

	vkCmdBeginDebugUtilsLabelEXT(cmd, &passDebugLabelInfo);

	// TODO: Can use a CPU stack allocator for this.
	#define maxBarrierCount 32
	VkImageMemoryBarrier2 imageMemoryBarriers[maxBarrierCount];
	VkBufferMemoryBarrier2 memoryBarriers[maxBarrierCount];
	#undef maxBarrierCount

	uint32_t imageBarrierCount;
	uint32_t memoryBarrierCount;
	createPipelineBarriers(graph->Core,
		(CreatePipelineBarriersDesc)
		{
			desc.ResourceStates,
			ib_staticArrayRange(imageMemoryBarriers),
			&imageBarrierCount,
			ib_staticArrayRange(memoryBarriers),
			&memoryBarrierCount
		});

	vkCmdPipelineBarrier2(cmd, &(VkDependencyInfo)
	{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = imageBarrierCount,
		.pImageMemoryBarriers = imageMemoryBarriers,
		.bufferMemoryBarrierCount = memoryBarrierCount,
		.pBufferMemoryBarriers = memoryBarriers
	});
}

void ibr_endComputePass(ibr_RenderGraph* graph, VkCommandBuffer cmd)
{
	popProfilingScope(graph, cmd);
	vkCmdEndDebugUtilsLabelEXT(cmd);
}

ibr_ResourceState ibr_textureState(ibr_Resource* resource, ibr_TextureState state, VkPipelineStageFlags stage)
{
	ib_assert(resource->Type == ibr_ResourceType_Texture);

	bool isDepth = (state & ibr_TextureStateFlag_Depth) != 0;
	switch (state & ibr_TextureState_Mask)
	{
		case ibr_TextureState_Write:
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.Layout = VK_IMAGE_LAYOUT_GENERAL,
				.ReleaseAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
				.AcquireAndReleaseStageMask = stage
			};
		case ibr_TextureState_Read:
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.Layout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.AcquireAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
		case ibr_TextureState_ReadWrite:
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.Layout = VK_IMAGE_LAYOUT_GENERAL,
				.AcquireAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.ReleaseAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
		case ibr_TextureState_TransferSrc:
			ib_assert(stage == VK_PIPELINE_STAGE_TRANSFER_BIT);
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.Layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.AcquireAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
		case ibr_TextureState_TransferDst:
			ib_assert(stage == VK_PIPELINE_STAGE_TRANSFER_BIT);
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.Layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.ReleaseAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
	}
	
	ib_assert(false); // Unexpected texture state!
	return (ibr_ResourceState){ 0 };
}

ibr_ResourceState ibr_textureToPresentState(ibr_Resource* resource)
{
	return (ibr_ResourceState)
	{
		.Resource = resource,
		.Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.AcquireStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		.ReleaseStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
	};
}

ibr_ResourceState ibr_bufferState(ibr_Resource* resource, ibr_BufferState state, VkPipelineStageFlags stage)
{
	ib_assert(resource->Type == ibr_ResourceType_Buffer);

	switch (state)
	{
		case ibr_BufferState_Write:
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.ReleaseAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
				.AcquireAndReleaseStageMask = stage
			};
		case ibr_BufferState_Read:
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.AcquireAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
		case ibr_BufferState_ReadWrite:
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.AcquireAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.ReleaseAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
		case ibr_BufferState_TransferSrc:
			ib_assert(stage == VK_PIPELINE_STAGE_TRANSFER_BIT);
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.AcquireAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
		case ibr_BufferState_TransferDst:
			ib_assert(stage == VK_PIPELINE_STAGE_TRANSFER_BIT);
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.ReleaseAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
	}
	
	ib_assert(false); // Unexpected buffer state!
	return (ibr_ResourceState){ 0 };
}
