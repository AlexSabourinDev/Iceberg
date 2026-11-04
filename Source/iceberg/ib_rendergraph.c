// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#include <iceberg/ib_rendergraph.h>
#include <string.h>
#include <stdlib.h>

#define list_push(list, in) \
	in->Next = *(list); \
	*(list) = in;


#define list_pop(out, list) \
	out = *(list); \
	*list = out != NULL ? out->Next : NULL; \
	out->Next = NULL;

#define list_clear(list) \
	*list = NULL;

#define list_pushAlloc(out, type, list) \
	{ \
		type* _transient = (type *)ibr_allocTransientMemory(graph, sizeof(type), alignof(type)); \
		memset(_transient, 0u, sizeof(type)); \
		list_push(list, _transient); \
		out = _transient; \
	}

static void pushProfilingScope(ibr_RenderGraph* graph, VkCommandBuffer cmd, char const* passName)
{
	ibr_TransientProfileScope* transientScope;
	list_pushAlloc(transientScope, ibr_TransientProfileScope, &graph->ActiveProfilingScopes);
	transientScope->Scope = (ibr_ProfilingScope)
	{
		.Timer = ib_beginTimer(&graph->TimerManager, cmd),
		.Name = passName,
	};

	graph->CurrentProfilingDepth++;
	ib_assert(graph->CurrentProfilingDepth == 1); // We don't currently support nested scopes.
}

static void popProfilingScope(ibr_RenderGraph* graph, VkCommandBuffer cmd)
{
	ibr_TransientProfileScope* transientScope;
	list_pop(transientScope, &graph->ActiveProfilingScopes);
	ib_assert(transientScope != NULL);
	ib_endTimer(&graph->TimerManager, cmd, &transientScope->Scope.Timer);

	list_push(&graph->CompletedScopes, transientScope);
	graph->CurrentProfilingDepth--;
}

static void getAcquireAndReleaseMask(ibr_ResourceState state, VkPipelineStageFlags2* acquire, VkPipelineStageFlags2* release)
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

typedef struct
{
	ibr_ResourceStateRange States;
	VkImageMemoryBarrier2** OutImageBarriers;
	uint32_t* OutImageBarrierCount;
	VkBufferMemoryBarrier2** OutMemoryBarriers;
	uint32_t* OutMemoryBarrierCount;
} CreatePipelineBarriersDesc;

static void createPipelineBarriers(ibr_RenderGraph* graph, CreatePipelineBarriersDesc desc)
{
	uint32_t imageBarrierCount = 0;
	uint32_t memoryBarrierCount = 0;

	ibr_ResourceState* stateBegin = ib_srangeBegin(desc.States);
	ibr_ResourceState* stateEnd = ib_srangeEnd(desc.States);

	{
		ibr_ResourceState *iter = stateBegin;
		for (; iter != stateEnd; iter++)
		{
			// List terminates on first null resource.
			if (iter->Resource == NULL)
			{
				break;
			}

			if (iter->Resource->Type == ibr_ResourceType_Texture)
			{
				imageBarrierCount++;
			}
			else if (iter->Resource->Type == ibr_ResourceType_Buffer)
			{
				memoryBarrierCount++;
			}
		}

		for (; iter != stateEnd; iter++)
		{
			// Make sure no extra resources after the first null.
			ib_assert(iter->Resource == NULL);
		}
	}

	VkImageMemoryBarrier2* imageBarriers = *desc.OutImageBarriers;
	if (imageBarriers == NULL)
	{
		imageBarriers = (VkImageMemoryBarrier2*)ibr_allocTransientMemory(graph, sizeof(VkImageMemoryBarrier2) * imageBarrierCount, alignof(VkImageMemoryBarrier2));
	}
	else
	{
		// API must have set OutImageBarrierCount if passing a preallocated array.
		ib_assert(imageBarrierCount <= *desc.OutImageBarrierCount);
	}

	VkBufferMemoryBarrier2* memoryBarriers = *desc.OutMemoryBarriers;
	if (memoryBarriers == NULL)
	{
		memoryBarriers = (VkBufferMemoryBarrier2*)ibr_allocTransientMemory(graph, sizeof(VkBufferMemoryBarrier2) * memoryBarrierCount, alignof(VkBufferMemoryBarrier2));
	}
	else
	{
		// API must have set OutImageBarrierCount if passing a preallocated array.
		ib_assert(memoryBarrierCount <= *desc.OutMemoryBarrierCount);
	}

	uint32_t imageBarrierWrite = 0;
	uint32_t memoryBarrierWrite = 0;
	for (ibr_ResourceState *iter = stateBegin; iter != stateEnd; iter++)
	{
		if (iter->Resource == NULL)
		{
			break;
		}

		ibr_ResourceState state = *iter;

		VkPipelineStageFlags2 acquireStageMask;
		VkPipelineStageFlags2 releaseStageMask;
		getAcquireAndReleaseMask(state, &acquireStageMask, &releaseStageMask);

		if (state.Resource->Type == ibr_ResourceType_Texture)
		{
			ib_assert(imageBarrierWrite < imageBarrierCount);
			if (imageBarrierWrite < imageBarrierCount)
			{
				imageBarriers[imageBarrierWrite] = ib_createTextureBarrier(graph->Core, (ib_TextureBarrierDesc)
																						{
																							.Texture = state.Resource->Texture,
																							.OldLayout = state.Resource->TextureLayout,
																							.NewLayout = state.Layout,
																							.SourceStageMask = state.Resource->LastReleaseStageMask,
																							.DestStageMask = acquireStageMask,
																							.SourceAccessMask = state.Resource->LastReleaseAccessMask,
																							.DestAccessMask = state.AcquireAccessMask
																						});

				// ASSUMPTION: Assume that what where we acquire is where we release for now.
				state.Resource->LastReleaseAccessMask = state.ReleaseAccessMask;
				state.Resource->LastReleaseStageMask = releaseStageMask;
				state.Resource->TextureLayout = state.Layout;
				imageBarrierWrite++;
			}
		}
		else if (state.Resource->Type == ibr_ResourceType_Buffer)
		{
			ib_assert(memoryBarrierWrite < memoryBarrierCount);
			if (memoryBarrierWrite < memoryBarrierCount)
			{
				memoryBarriers[memoryBarrierWrite] = (VkBufferMemoryBarrier2)
				{
					.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
					.buffer = state.Resource->Buffer->VulkanBuffer,
					.size = VK_WHOLE_SIZE,
					.srcStageMask = state.Resource->LastReleaseStageMask,
					.dstStageMask = acquireStageMask,
					.srcAccessMask = state.Resource->LastReleaseAccessMask,
					.dstAccessMask = state.AcquireAccessMask,
				};

				// ASSUMPTION: Assume that what where we acquire is where we release for now.
				state.Resource->LastReleaseAccessMask = state.ReleaseAccessMask;
				state.Resource->LastReleaseStageMask = releaseStageMask;
				memoryBarrierWrite++;
			}
		}
		else
		{
			ib_assert(false);
		}
	}
	*desc.OutImageBarriers = imageBarriers;
	*desc.OutImageBarrierCount = imageBarrierCount;
	*desc.OutMemoryBarriers = memoryBarriers;
	*desc.OutMemoryBarrierCount = memoryBarrierCount;
}

typedef struct StackGpuMemoryPage
{
    iba_PageHeader Header;
    VkBuffer Buffer;
    iba_GpuAllocation PageAlloc;
} StackGpuMemoryPage;

static iba_PageHeader* allocGpuMemoryPage(void* userData, size_t pageSize)
{
    // It would be fun to use the transient CPU allocator here.
    // But we don't free the pages every frame. We just reset the head pointer.
    StackGpuMemoryPage* page = calloc(1, sizeof(StackGpuMemoryPage));

    iba_GpuAllocator* generalAllocator = (iba_GpuAllocator*)userData;
    VkDevice logicalDevice = generalAllocator->LogicalDevice;
    VkBufferCreateInfo bufferCreate =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = pageSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    ib_vkCheck(vkCreateBuffer(logicalDevice, &bufferCreate, ib_NoVkAllocator, &page->Buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, page->Buffer, &memoryRequirements);

    page->PageAlloc = iba_gpuAlloc(generalAllocator, (iba_GpuAllocationRequest)
                                   {
                                       .Size = memoryRequirements.size,
                                       .Alignment = memoryRequirements.alignment,
                                       .TypeBits = memoryRequirements.memoryTypeBits,
                                       .RequiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   });

    ib_vkCheck(vkBindBufferMemory(logicalDevice, page->Buffer, page->PageAlloc.Memory, page->PageAlloc.Offset));

    return &page->Header;
}

static void freeGpuMemoryPage(void* userData, iba_PageHeader* pageHeader)
{
    iba_GpuAllocator* generalAllocator = (iba_GpuAllocator*)userData;
    StackGpuMemoryPage* page = (StackGpuMemoryPage*)pageHeader;

    vkDestroyBuffer(generalAllocator->LogicalDevice, page->Buffer, ib_NoVkAllocator);
    iba_gpuFree(generalAllocator, &page->PageAlloc);
    free(page);
}

void ibr_initRenderGraphs(ib_Core* core, ibr_RenderGraph* graphs, uint32_t graphCount)
{
    for (uint32_t i = 0; i < graphCount; i++)
    {
        graphs[i].Core = core;

        static size_t const fullCpuPageSize = 1024 * 1024;
        iba_initCpuStackAllocator((iba_CpuStackAllocatorDesc)
                                  {
                                      .PageSize = fullCpuPageSize
                                  }, &graphs[i].FrameCPUStack);

        ib_initTimerManager((ib_TimerManagerDesc)
                            {
                                .Core = core,
                                .MaxTimerCount = 1024,
                            }, &graphs[i].TimerManager);

        // Create the descriptor pools
        {
            uint32_t const maxTransientDescriptorTypeCount = 128;
            uint32_t const maxTransientDescriptorSetCount = 128;
            VkDescriptorPoolSize descriptorPoolSizes[] =
            {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxTransientDescriptorTypeCount },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, maxTransientDescriptorTypeCount },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxTransientDescriptorTypeCount },
                { VK_DESCRIPTOR_TYPE_SAMPLER, maxTransientDescriptorTypeCount },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, maxTransientDescriptorTypeCount },
            };

            VkDescriptorPoolCreateInfo descriptorPoolCreate =
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .flags = 0,
                .maxSets = maxTransientDescriptorSetCount,
                .poolSizeCount = ib_arrayCount(descriptorPoolSizes),
                .pPoolSizes = descriptorPoolSizes
            };

            ib_vkCheck(vkCreateDescriptorPool(core->LogicalDevice, &descriptorPoolCreate, ib_NoVkAllocator, &graphs[i].TransientDescriptorPool));
        }

        for (uint32_t q = 0; q < ib_Queue_Count; q++)
        {
            VkCommandPoolCreateInfo commandPoolCreateInfo =
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = core->Queues[q].Index,
            };

            ib_vkCheck(vkCreateCommandPool(core->LogicalDevice, &commandPoolCreateInfo, ib_NoVkAllocator, &graphs[i].TransientCommandPools[q]));
        }

        ib_vkCheck(vkCreateFence(core->LogicalDevice, &(VkFenceCreateInfo)
                                 {
                                     .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                     .flags = VK_FENCE_CREATE_SIGNALED_BIT
                                 }, ib_NoVkAllocator, &graphs[i].FrameFence));

        uint32_t const transientGpuPageSize = 1024 * 1024; // 10MB pages seems large :thinking:
        iba_initStackAllocator((iba_StackAllocatorDesc)
                               {
                                   .PageAllocator =
                                   {
                                       .AllocPage = &allocGpuMemoryPage,
                                       .FreePage = &freeGpuMemoryPage,
                                       .UserData = &core->Allocator
                                   },
                                   .PageSize = transientGpuPageSize
                               },
                               &graphs[i].FrameGPUStack);
    }
}

void ibr_killRenderGraphs(ib_Core* core, ibr_RenderGraph* graphs, uint32_t graphCount)
{
    for (uint32_t i = 0; i < graphCount; i++)
    {
        ibr_RenderGraph* graph = &graphs[i];

        for (ibr_TransientTexture* iter = graph->TransientTextures; iter != NULL; iter = iter->Next)
        {
            ib_freeTexture(graph->Core, &iter->Texture);
        }
        graph->TransientTextures = NULL;

        for (ibr_TransientBuffer* iter = graph->TransientBuffers; iter != NULL; iter = iter->Next)
        {
            ib_freeBuffer(graph->Core, &iter->Buffer);
        }
        graph->TransientBuffers = NULL;

        for (ibr_TransientImageView* iter = graph->TransientImageViews; iter != NULL; iter = iter->Next)
        {
            vkDestroyImageView(graph->Core->LogicalDevice, iter->View, ib_NoVkAllocator);
        }
        graph->TransientImageViews = NULL;

        vkDestroyDescriptorPool(core->LogicalDevice, graph->TransientDescriptorPool, ib_NoVkAllocator);

        for (uint32_t q = 0; q < ib_Queue_Count; q++)
        {
            vkDestroyCommandPool(core->LogicalDevice, graph->TransientCommandPools[q], ib_NoVkAllocator);
        }

        vkDestroyFence(core->LogicalDevice, graph->FrameFence, ib_NoVkAllocator);

        ib_killTimerManager(core, &graph->TimerManager);
        iba_killStackAllocator(&graph->FrameCPUStack);
        iba_killStackAllocator(&graph->FrameGPUStack);
    }
}

bool ibr_beginFrame(ibr_RenderGraph* graph, ibr_BeginFrameDesc desc)
{
    // Fence is signaled - our resources are free, we're good to go!
    ib_vkCheck(vkWaitForFences(graph->Core->LogicalDevice, 1, &graph->FrameFence, VK_TRUE, UINT64_MAX));

    for (ibr_TransientTexture* head = graph->TransientTextures; head != NULL; head = head->Next)
    {
        ib_freeTexture(graph->Core, &head->Texture);
    }
    graph->TransientTextures = NULL;

    for (ibr_TransientBuffer* head = graph->TransientBuffers; head != NULL; head = head->Next)
    {
        ib_freeBuffer(graph->Core, &head->Buffer);
    }
    graph->TransientBuffers = NULL;

    for (ibr_TransientImageView* iter = graph->TransientImageViews; iter != NULL; iter = iter->Next)
    {
        vkDestroyImageView(graph->Core->LogicalDevice, iter->View, ib_NoVkAllocator);
    }
    graph->TransientImageViews = NULL;

    vkResetDescriptorPool(graph->Core->LogicalDevice, graph->TransientDescriptorPool, 0);
    for (uint32_t q = 0; q < ib_Queue_Count; q++)
    {
        vkResetCommandPool(graph->Core->LogicalDevice, graph->TransientCommandPools[q], 0);
        // TODO: Revisit later. We should simply track the previous list of active command buffers and grow that list as we go.
        // For now, just free our previous command buffers every frame.
        for (ibr_TransientCommandBuffer* iter = graph->TransientCommandBuffers[q]; iter != NULL; iter = iter->Next)
        {
            vkFreeCommandBuffers(graph->Core->LogicalDevice, graph->TransientCommandPools[q], 1, &iter->CommandBuffer);
        }
        list_clear(&graph->TransientCommandBuffers[q]);
    }

    if (desc.Surface != NULL)
    {
        ib_PrepareSurfaceResult prepareResult = ib_prepareSurface(graph->Core, (ib_PrepareSurfaceDesc) {
            desc.Surface, desc.FrameIndex
        });

        // Couldn't prepare our surface.
        if (prepareResult.SurfaceState != ib_SurfaceState_Ok)
        {
            return false;
        }

        graph->SwapchainTextureIndex = prepareResult.SwapchainTextureIndex;
        graph->SwapchainTexture = &desc.Surface->SwapchainTextures[graph->SwapchainTextureIndex];
        graph->SwapchainAcquireSemaphore = desc.Surface->Framebuffers[desc.FrameIndex].AcquireSemaphore;

        // The present semaphore is tied to the swapchain texture index.
        // See: https://docs.vulkan.org/guide/latest/swapchain_semaphore_reuse.html
        graph->PresentSemaphore = desc.Surface->PresentSemaphores[graph->SwapchainTextureIndex];
        graph->ScreenExtent = (VkExtent2D) { graph->SwapchainTexture->Extent.width, graph->SwapchainTexture->Extent.height };
    }
    else
    {
        graph->ScreenExtent = (VkExtent2D) { 1, 1 };
        graph->SwapchainTexture = NULL;
        graph->SwapchainAcquireSemaphore = VK_NULL_HANDLE;
        graph->PresentSemaphore = VK_NULL_HANDLE;
    }

    // Only reset our fence if we know we're going to signal it.
    // If we made it this far, we're good to go.
    ib_vkCheck(vkResetFences(graph->Core->LogicalDevice, 1, &graph->FrameFence));

    // We don't get to leverage our transient allocator here
    // Since our allocations live between our allocator reset boundary we need to go from one stack to the next.
    // There's probably a better way to do this.
#define maxScopeTimings 1024
    ibr_ScopeTiming scopeTimings[maxScopeTimings] = {};
    uint32_t scopeTimingCount = 0;
#undef maxScopeTimings

    for (ibr_TransientProfileScope* iter = graph->CompletedScopes; iter != NULL; iter = iter->Next)
    {
        // Scope overflow
        if (scopeTimingCount == ib_arrayCount(scopeTimings))
        {
            ib_assert(false);
            break;
        }

        bool isBlocking = false;
        ibr_ProfilingScope* scope = &iter->Scope;
        ibr_ScopeTiming* transientTiming = &scopeTimings[scopeTimingCount++];
        *transientTiming = (ibr_ScopeTiming)
        {
            .Timing = ib_queryTimer(graph->Core, &graph->TimerManager, &scope->Timer, isBlocking),
            .Name = scope->Name,
        };
        ib_assert(transientTiming->Timing != ib_TimerQueryNotReady); // We should be ready, our frame's fence was signaled.
    }
    list_clear(&graph->CompletedScopes);
    list_clear(&graph->PreviousFrameTimings);

    ib_resetTimersCPU(graph->Core, &graph->TimerManager);

    iba_stackReset(&graph->FrameCPUStack);
    // Our GPU memory is no longer in use by the previous frame. Reset for the current frame.
    iba_stackReset(&graph->FrameGPUStack);

    // Copy our data from the previous stack to the current stack.
    for(uint32_t i = 0; i < scopeTimingCount; i++)
    {
        ibr_TransientScopeTiming* transientTiming;
        list_pushAlloc(transientTiming, ibr_TransientScopeTiming, &graph->PreviousFrameTimings);
        transientTiming->Timing = scopeTimings[i];
    }

    return true;
}

void ibr_endFrame(ibr_RenderGraph* graph)
{
    ib_potentiallyUnused(graph);
    ib_assert(graph->ActiveProfilingScopes == NULL);
}

void ibr_beginUpload(ibr_RenderGraph* graph, VkCommandBuffer commands)
{
    ib_unused(graph); // Don't actually need to know which memory. We're using a global memory barrier.
    // Make sure our memory write to host memory is visible with a memory barrier.
    VkMemoryBarrier2 memoryBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_HOST_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
        .srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT,
    };

    vkCmdPipelineBarrier2(commands, &(VkDependencyInfo)
                        {
                            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                            .memoryBarrierCount = 1,
                            .pMemoryBarriers = &memoryBarrier
                        });
}

void ibr_endUpload(ibr_RenderGraph* graph)
{
    // Doesn't do anything, here for symmetry.
    ib_unused(graph);
}

void* ibr_allocTransientMemory(ibr_RenderGraph* graph, size_t size, size_t alignment)
{
    iba_StackAllocation allocation = iba_stackAlloc(&graph->FrameCPUStack, (iba_StackAllocationRequest) { size, alignment });
    return iba_cpuStackAllocToMemory(allocation);
}

ibr_TransientGpuMemory ibr_allocTransientGpuMemory(ibr_RenderGraph* graph, size_t size, size_t alignment)
{
    iba_StackAllocation allocation = iba_stackAlloc(&graph->FrameGPUStack, (iba_StackAllocationRequest) { size, alignment });
    StackGpuMemoryPage* gpuPage = (StackGpuMemoryPage*)allocation.Page;
    return (ibr_TransientGpuMemory)
    {
        .CpuMemory = (uint8_t*)gpuPage->PageAlloc.CPUMemory + allocation.Offset,
        .Buffer = gpuPage->Buffer,
        .BufferOffset = allocation.Offset
    };
}

ibr_Resource ibr_allocPassResource(ibr_RenderGraph* graph, VkCommandBuffer commands, ibr_ResourceDesc resourceDesc)
{
    ibr_Resource outResource = (ibr_Resource) { 0 };
    ibr_allocPassResources(graph, commands, (ibr_AllocPassResourcesDesc)
                           {
                               .ResourceBindings =
                               {
                                   &outResource,
                                   resourceDesc
                               }
                           });

    return outResource;
}

void ibr_allocPassResources(ibr_RenderGraph* graph, VkCommandBuffer commands, ibr_AllocPassResourcesDesc desc)
{
    uint32_t transientWriteCount = 0u;

    ibr_AllocResourceBinding* begin = ib_srangeBegin(desc.ResourceBindings);
    ibr_AllocResourceBinding* end = ib_srangeEnd(desc.ResourceBindings);
    for (ibr_AllocResourceBinding* iter = begin; iter != end; iter++)
    {
        if (iter->OutResource == NULL)
        {
            break;
        }

        ibr_ResourceDesc resourceDesc = iter->Desc;
        ibr_Resource* outResource = iter->OutResource;
        *outResource = (ibr_Resource) { 0 };
        outResource->Type = resourceDesc.Type;

        if (resourceDesc.Type == ibr_ResourceType_Texture)
        {
            outResource->TextureLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if ((resourceDesc.Flags & ibr_ResourceFlag_Transient) != 0)
            {
                ib_TextureDesc textureDesc = resourceDesc.TextureDesc;
                if (memcmp(&textureDesc.Extent, &ibr_ScreenExtent, sizeof(VkExtent2D)) == 0)
                {
                    textureDesc.Extent = (VkExtent3D) { graph->ScreenExtent.width, graph->ScreenExtent.height };
                }

                ibr_TransientTexture* transientTexture;
                list_pushAlloc(transientTexture, ibr_TransientTexture, &graph->TransientTextures);
                ib_Texture* texture = &transientTexture->Texture;
                *texture = ib_allocTexture(graph->Core, textureDesc);
                outResource->Texture = texture;
            }
            else
            {
                outResource->Texture = resourceDesc.Texture;
                outResource->TextureLayout = resourceDesc.TextureLayout;
            }
        }
        else if (resourceDesc.Type == ibr_ResourceType_Buffer)
        {
            if ((resourceDesc.Flags & ibr_ResourceFlag_Transient) != 0)
            {
                ib_BufferDesc bufferDesc = resourceDesc.BufferDesc;

                ibr_TransientBuffer* transientBuffer;
                list_pushAlloc(transientBuffer, ibr_TransientBuffer, &graph->TransientBuffers);
                ib_Buffer* buffer = &transientBuffer->Buffer;
                *buffer = ib_allocBuffer(graph->Core, bufferDesc);
                outResource->Buffer = buffer;
            }
            else
            {
                outResource->Buffer = resourceDesc.Buffer;
            }
        }
    }
}

ibr_ResourceDesc ibr_textureResourceDesc(ib_Texture* texture, VkImageLayout layout)
{
    return (ibr_ResourceDesc)
    {
        .Type = ibr_ResourceType_Texture,
        .Texture = texture,
        .TextureLayout = layout
    };
}

ibr_ResourceDesc ibr_transientTextureResourceDesc(VkExtent3D extent, VkFormat format, ibr_TransientTextureFlags flags, char const* debugName)
{
    VkImageUsageFlags usageFlags = 0u;
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    if ((flags & ibr_TransientTextureFlag_StorageBit) != 0u)
    {
        usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if ((flags & ibr_TransientTextureFlag_ColorAttachmentBit) != 0u)
    {
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if ((flags & ibr_TransientTextureFlag_TransferSrcBit) != 0u)
    {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if ((flags & ibr_TransientTextureFlag_TransferDstBit) != 0u)
    {
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if ((flags & ibr_TransientTextureFlag_DepthBit) != 0u)
    {
        usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    return (ibr_ResourceDesc)
    {
        .Type = ibr_ResourceType_Texture,
        .Flags = ibr_ResourceFlag_Transient,
        .TextureDesc = (ib_TextureDesc)
        {
            .Usage = usageFlags,
            .Format = format,
            .Extent = extent,
            .Aspect = aspectFlags,
            .DebugName = debugName,
        }
    };
}

ibr_ResourceDesc ibr_transientBufferResourceDesc(size_t size, ibr_TransientBufferFlags flags, char const* debugName)
{
    VkMemoryPropertyFlags memoryFlags = 0u;
    switch (flags & ibr_TransientBufferFlag_MemoryMask)
    {
    case ibr_TransientBufferFlag_Device:
        memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case ibr_TransientBufferFlag_Host:
        memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        break;
    default:
        ib_assert(false, "Missing case!");
        break;
    }

    VkBufferUsageFlags bufferUsage = 0u;
    if ((flags & ibr_TransientBufferFlag_UniformBufferBit) != 0u)
    {
        bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if ((flags & ibr_TransientBufferFlag_StorageBufferBit) != 0u)
    {
        bufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    return (ibr_ResourceDesc)
    {
        .Type = ibr_ResourceType_Buffer,
        .Flags = ibr_ResourceFlag_Transient,
        .BufferDesc =
        {
            .Usage = bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .Size = size,
            .RequiredMemoryFlags = memoryFlags,
            .DebugName = debugName,
        }
    };
}

void ibr_writeResources(ibr_RenderGraph* graph, VkCommandBuffer commands, ibr_WriteResourcesDesc desc)
{
    ibr_WriteResourceDesc* begin = ib_srangeBegin(desc.Writes);
    ibr_WriteResourceDesc* end = ib_srangeEnd(desc.Writes);
    uint32_t maxBarriers = ib_srangeCapacity(desc.Writes);
    ibr_ResourceState* resourceTransitions = (ibr_ResourceState*)ibr_allocTransientMemory(graph, sizeof(ibr_ResourceState) * maxBarriers, alignof(ibr_ResourceState));
    uint32_t resourceTransitionCount = 0;
    for (ibr_WriteResourceDesc* iter = begin; iter != end && iter->Resource != NULL; iter++)
    {
        if (iter->Resource->Type == ibr_ResourceType_Texture)
        {
            resourceTransitions[resourceTransitionCount++] = ibr_textureState(iter->Resource, ibr_TextureState_TransferDst, VK_PIPELINE_STAGE_2_COPY_BIT);
        }
        else if (iter->Resource->Type == ibr_ResourceType_Buffer)
        {
            resourceTransitions[resourceTransitionCount++] = ibr_bufferState(iter->Resource, ibr_BufferState_TransferDst, VK_PIPELINE_STAGE_2_COPY_BIT);
        }
    }

    ibr_barriers(graph, commands, (ibr_BarriersDesc)
                 {
                     .ResourceStates =
                     {
                         .Data = resourceTransitions,
                         .Count = resourceTransitionCount
                     }
                 });


    for (ibr_WriteResourceDesc* iter = begin; iter != end && iter->Resource != NULL; iter++)
    {
        ibr_Resource* resource = iter->Resource;
        if (resource->Type == ibr_ResourceType_Texture)
        {
            ib_Texture* texture = resource->Texture;
            ibr_TransientGpuMemory gpuMemory = ibr_allocTransientGpuMemory(graph, iter->Size, iter->Alignment);
            memcpy(gpuMemory.CpuMemory, iter->Data, iter->Size);

            // Image copy
            {
                VkBufferImageCopy copyRegion =
                {
                    .bufferOffset = gpuMemory.BufferOffset,
                    .bufferRowLength = 0,
                    .bufferImageHeight = 0,
                    .imageSubresource =
                    {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel = 0,
                        .baseArrayLayer = 0,
                        .layerCount = ib_max(texture->LayerCount, 1u),
                    },
                    .imageOffset = { 0 },
                    .imageExtent =
                    {
                        .width = texture->Extent.width,
                        .height = texture->Extent.height,
                        .depth = 1,
                    },
                };

                vkCmdCopyBufferToImage(commands, gpuMemory.Buffer, texture->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
            }
        }
        else if (resource->Type == ibr_ResourceType_Buffer)
        {
            ib_Buffer* buffer = resource->Buffer;

            ibr_TransientGpuMemory gpuMemory = ibr_allocTransientGpuMemory(graph, iter->Size, iter->Alignment);
            memcpy(gpuMemory.CpuMemory, iter->Data, iter->Size);

            // Buffer copy
            {
                VkBufferCopy copyRegion =
                {
                    .srcOffset = gpuMemory.BufferOffset,
                    .dstOffset = 0u,
                    .size = iter->Size
                };

                vkCmdCopyBuffer(commands, gpuMemory.Buffer, buffer->VulkanBuffer, 1, &copyRegion);
            }

            resource->LastReleaseStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
            resource->LastReleaseAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        }
    }
}

VkImageView ibr_allocTransientImageView(ibr_RenderGraph* graph, ibr_AllocTransientImageViewDesc desc)
{
	ibr_TransientImageView* transientImageView;
	list_pushAlloc(transientImageView, ibr_TransientImageView, &graph->TransientImageViews);
	VkImageView* view = &transientImageView->View;

	VkImageViewCreateInfo imageCreateViewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = desc.Texture->Image,
		.viewType = desc.Texture->Extent.depth > 0 ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D,
		.format = desc.Texture->Format,
		.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
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
	ib_assert(desc.Resources.Count == desc.ShaderInputs.Count); // Can't have more resources than inputs

	ib_ShaderInputWrite* writes = (ib_ShaderInputWrite*)ibr_allocTransientMemory(graph, desc.Resources.Count * sizeof(ib_ShaderInputWrite), alignof(ib_ShaderInputWrite));

	uint32_t writeCount = 0;
	for (uint32_t i = 0; i < desc.Resources.Count; i++)
	{
		ibr_Resource* resource = desc.Resources.Data[i];
		if (resource != NULL) // resource array is allowed to be sparse. Just pass in the resources you care about at the right indices.
		{
			ib_ShaderInputWrite inputWrite = { .Desc = &desc.ShaderInputs.Data[i], i };
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
	ibr_TransientCommandBuffer* transientCommandBuffer;
	list_pushAlloc(transientCommandBuffer, ibr_TransientCommandBuffer, &graph->TransientCommandBuffers[queue]);
	ib_allocCommandBuffers(graph->Core, (ib_AllocCommandBuffersDesc)
						{
							.OutCommandBuffers = ib_singlePtrRange(&transientCommandBuffer->CommandBuffer),
							.Queue = queue,
							.Pool = graph->TransientCommandPools[queue]
						});
	return transientCommandBuffer->CommandBuffer;
}

void ibr_submitCommandBuffers(ibr_RenderGraph* graph, ibr_SubmitCommandBufferDesc desc)
{
    uint32_t maxCommandCount = ib_srangeCapacity(desc.CommandBuffers);
    VkCommandBufferSubmitInfo* commands = (VkCommandBufferSubmitInfo*)ibr_allocTransientMemory(graph, sizeof(VkCommandBufferSubmitInfo) * maxCommandCount, alignof(VkCommandBufferSubmitInfo));
    uint32_t commandCount = 0;
    for (VkCommandBuffer* iter = ib_srangeBegin(desc.CommandBuffers),
        *end = ib_srangeEnd(desc.CommandBuffers); iter != end; iter++)
    {
        if (*iter == VK_NULL_HANDLE)
        {
            break;
        }

        commands[commandCount++] = (VkCommandBufferSubmitInfo)
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = *iter,
        };
    }

    uint32_t maxWaitSemaphores = ib_srangeCapacity(desc.WaitSemaphores);
    VkSemaphoreSubmitInfo* waitSemaphores = (VkSemaphoreSubmitInfo*)ibr_allocTransientMemory(graph, sizeof(VkSemaphoreSubmitInfo) * maxWaitSemaphores, alignof(VkSemaphoreSubmitInfo));
    uint32_t waitSemaphoreCount = 0;
    for (ibr_SemaphoreSubmit* iter = ib_srangeBegin(desc.WaitSemaphores),
        *end = ib_srangeEnd(desc.WaitSemaphores); iter != end; iter++)
    {
        if (iter->Semaphore == VK_NULL_HANDLE)
        {
            break;
        }

        waitSemaphores[waitSemaphoreCount++] = (VkSemaphoreSubmitInfo)
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = iter->Semaphore,
            .stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            .value = iter->Value
        };
    }

    uint32_t maxSignalSemaphores = ib_srangeCapacity(desc.SignalSemaphores);
    VkSemaphoreSubmitInfo* signalSemaphores = (VkSemaphoreSubmitInfo*)ibr_allocTransientMemory(graph, sizeof(VkSemaphoreSubmitInfo) * maxSignalSemaphores, alignof(VkSemaphoreSubmitInfo));
    uint32_t signalSemaphoreCount = 0;
    for (ibr_SemaphoreSubmit* iter = ib_srangeBegin(desc.SignalSemaphores),
        *end = ib_srangeEnd(desc.SignalSemaphores); iter != end; iter++)
    {
        if (iter->Semaphore == VK_NULL_HANDLE)
        {
            break;
        }

        signalSemaphores[signalSemaphoreCount++] = (VkSemaphoreSubmitInfo)
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = iter->Semaphore,
            .stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            .value = iter->Value
        };
    }

    VkSubmitInfo2 submitInfo = (VkSubmitInfo2)
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pCommandBufferInfos = commands,
        .commandBufferInfoCount = commandCount,
        .pWaitSemaphoreInfos = waitSemaphores,
        .waitSemaphoreInfoCount = waitSemaphoreCount,
        .pSignalSemaphoreInfos = signalSemaphores,
        .signalSemaphoreInfoCount = signalSemaphoreCount
    };
    ib_vkCheck(vkQueueSubmit2(graph->Core->Queues[ib_Queue_Graphics].Queue, 1, &submitInfo, desc.SubmitFence));
}

void ibr_present(ibr_PresentDesc desc)
{
	ib_SurfaceState state = ib_presentSurface(desc.Graph->Core, (ib_PresentSurfaceDesc)
											{
												.Surface = desc.Surface,
												.SwapchainTextureIndex = desc.Graph->SwapchainTextureIndex,
												.WaitSemaphore = desc.Graph->PresentSemaphore
											});
	if (state == ib_SurfaceState_ShouldRebuild)
	{
		ib_rebuildSurface(desc.Graph->Core, desc.Surface);
	}
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
	uint32_t renderTargetCount = 0;
	{
		uint32_t totalResourceCount = ib_srangeCapacity(desc.RenderTargets) + ib_srangeCapacity(desc.OtherResourceStates);
		if (desc.DepthTarget.Resource != NULL)
		{
			totalResourceCount++;
		}

		// Transition for write
		VkImageMemoryBarrier2* imageMemoryBarriers = (VkImageMemoryBarrier2*)ibr_allocTransientMemory(graph, sizeof(VkImageMemoryBarrier2) * totalResourceCount, alignof(VkImageMemoryBarrier2));
		VkBufferMemoryBarrier2* memoryBarriers = (VkBufferMemoryBarrier2*)ibr_allocTransientMemory(graph, sizeof(VkBufferMemoryBarrier2) * totalResourceCount, alignof(VkBufferMemoryBarrier2));

		uint32_t imageBarrierCount = totalResourceCount;
		uint32_t memoryBarrierCount = totalResourceCount;
		createPipelineBarriers(graph, (CreatePipelineBarriersDesc)
							{
								desc.OtherResourceStates,
								&imageMemoryBarriers,
								&imageBarrierCount,
								&memoryBarriers,
								&memoryBarrierCount
							});

		for (ibr_RenderTargetState* iter = ib_srangeBegin(desc.RenderTargets),
			*end = ib_srangeEnd(desc.RenderTargets); iter != end; iter++)
		{
			if (iter->Resource == NULL)
			{
				break;
			}

			renderTargetCount++;

			ibr_RenderTargetState state = *iter;

			ib_assert(state.Resource->Type == ibr_ResourceType_Texture);
			imageMemoryBarriers[imageBarrierCount++] = ib_createTextureBarrier(graph->Core, (ib_TextureBarrierDesc)
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

		// Depth Target
		if (desc.DepthTarget.Resource != NULL)
		{
			ibr_RenderTargetState state = desc.DepthTarget;

			ib_assert(state.Resource->Type == ibr_ResourceType_Texture);
			imageMemoryBarriers[imageBarrierCount++] = ib_createTextureBarrier(graph->Core, (ib_TextureBarrierDesc)
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

	ibr_RenderTargetState* renderTargetBegin = ib_srangeBegin(desc.RenderTargets);
	VkExtent2D extents = { 0 };
	if (renderTargetBegin->Resource != NULL)
	{
		extents = (VkExtent2D) { renderTargetBegin->Resource->Texture->Extent.width, renderTargetBegin->Resource->Texture->Extent.height };
	}
	else
	{
		extents = (VkExtent2D) { desc.DepthTarget.Resource->Texture->Extent.width, desc.DepthTarget.Resource->Texture->Extent.height };
	}

	// Attachments
	{
		uint32_t colorAttachmentWrite = 0;
		VkRenderingAttachmentInfo* colorAttachments = (VkRenderingAttachmentInfo*)ibr_allocTransientMemory(graph, sizeof(VkRenderingAttachmentInfo) * renderTargetCount, alignof(VkRenderingAttachmentInfo));
		for (ibr_RenderTargetState* iter = renderTargetBegin,
			*end = renderTargetBegin + renderTargetCount; iter != end; iter++)
		{
			ibr_RenderTargetState state = *iter;
			colorAttachments[colorAttachmentWrite++] = (VkRenderingAttachmentInfo)
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
		if (desc.DepthTarget.Resource != NULL)
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
			.renderArea = { .extent = extents },
			.layerCount = 1,
			.colorAttachmentCount = renderTargetCount,
			.pColorAttachments = colorAttachments,
			.pDepthAttachment = (desc.DepthTarget.Resource != NULL) ? &depthAttachment : NULL
		};

		vkCmdBeginRendering(cmd, &renderInfo);
	}

	if (desc.MinDepth == 0.0f && desc.MaxDepth == 0.0f)
	{
		desc.MaxDepth = 1.0f;
	}

	vkCmdSetViewport(cmd, 0, 1, &(VkViewport)
					{
						.width = (float)extents.width, .height = (float)extents.height,
						.minDepth = desc.MinDepth, .maxDepth = desc.MaxDepth,
					});

	vkCmdSetScissor(cmd, 0, 1, &(VkRect2D) { .extent = extents });
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
	VkImageMemoryBarrier2* imageMemoryBarriers = NULL;
	VkBufferMemoryBarrier2* memoryBarriers = NULL;

	uint32_t imageBarrierCount = 0;
	uint32_t memoryBarrierCount = 0;
	createPipelineBarriers(graph,
						(CreatePipelineBarriersDesc)
						{
							desc.ResourceStates,
							&imageMemoryBarriers,
							&imageBarrierCount,
							&memoryBarriers,
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

	VkImageMemoryBarrier2* imageMemoryBarriers = NULL;
	VkBufferMemoryBarrier2* memoryBarriers = NULL;

	uint32_t imageBarrierCount = 0;
	uint32_t memoryBarrierCount = 0;
	createPipelineBarriers(graph,
						(CreatePipelineBarriersDesc)
						{
							desc.ResourceStates,
							&imageMemoryBarriers,
							&imageBarrierCount,
							&memoryBarriers,
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

ibr_ResourceState ibr_textureState(ibr_Resource* resource, ibr_TextureState state, VkPipelineStageFlags2 stage)
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
        case ibr_TextureState_RenderTarget:
            if (isDepth)
            {
                return (ibr_ResourceState)
                {
                    .Resource = resource,
                    .Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                    .AcquireAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                    .ReleaseAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    .AcquireAndReleaseStageMask = stage,
                };
            }
            else
            {
                return (ibr_ResourceState)
                {
                    .Resource = resource,
                    .Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .AcquireAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                    .ReleaseAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    .AcquireAndReleaseStageMask = stage,
                };
            }
		case ibr_TextureState_TransferSrc:
			ib_assert(stage == VK_PIPELINE_STAGE_TRANSFER_BIT || stage == VK_PIPELINE_STAGE_2_COPY_BIT);
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.Layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.AcquireAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
		case ibr_TextureState_TransferDst:
			ib_assert(stage == VK_PIPELINE_STAGE_TRANSFER_BIT || stage == VK_PIPELINE_STAGE_2_COPY_BIT);
			return (ibr_ResourceState)
			{
				.Resource = resource,
				.Layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.ReleaseAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.AcquireAndReleaseStageMask = stage,
			};
	}
	
	ib_assert(false); // Unexpected texture state!
	return (ibr_ResourceState) { 0 };
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

ibr_ResourceState ibr_bufferState(ibr_Resource* resource, ibr_BufferState state, VkPipelineStageFlags2 stage)
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
            ib_assert(stage == VK_PIPELINE_STAGE_TRANSFER_BIT || stage == VK_PIPELINE_STAGE_2_COPY_BIT);
            return (ibr_ResourceState)
            {
                .Resource = resource,
                .AcquireAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                .AcquireAndReleaseStageMask = stage,
            };
        case ibr_BufferState_TransferDst:
            ib_assert(stage == VK_PIPELINE_STAGE_TRANSFER_BIT || stage == VK_PIPELINE_STAGE_2_COPY_BIT);
            return (ibr_ResourceState)
            {
                .Resource = resource,
                .ReleaseAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .AcquireAndReleaseStageMask = stage,
            };
    }

    ib_assert(false); // Unexpected buffer state!
    return (ibr_ResourceState) { 0 };
}

void ibr_initDefaultResources(ibr_RenderGraph* graph, VkCommandBuffer commandBuffer, ibr_DefaultResources* resources)
{
    ib_TextureDesc textureDescs[] =
    {
        [ibr_DefaultTexture_White] = (ib_TextureDesc)
        {
            .Usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .Format = VK_FORMAT_R8G8B8A8_UNORM,
            .Extent = { 1, 1 },
            .Aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .DebugName = "DefaultTexture_White"
        },
        [ibr_DefaultTexture_Checkerboard] = (ib_TextureDesc)
        {
            .Usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .Format = VK_FORMAT_R8G8B8A8_UNORM,
            .Extent = { 2, 2 },
            .Aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .DebugName = "DefaultTexture_Checkerboard"
        }
    };

    static_assert(ib_arrayCount(textureDescs) == ibr_DefaultTexture_Count);

    for (uint32_t i = 0; i < ibr_DefaultTexture_Count; i++)
    {
        resources->Textures[i] = ib_allocTexture(graph->Core, textureDescs[i]);
    }

    ibr_Resource writeResources[ibr_DefaultTexture_Count];
    ibr_AllocResourceBinding resourceBindings[ibr_DefaultTexture_Count];
    for (uint32_t i = 0; i < ibr_DefaultTexture_Count; i++)
    {
        resourceBindings[i] = (ibr_AllocResourceBinding)
        {
            .OutResource = &writeResources[i],
            .Desc = ibr_textureResourceDesc(&resources->Textures[i], VK_IMAGE_LAYOUT_UNDEFINED)
        };
    }

    ibr_allocPassResources(graph, commandBuffer, (ibr_AllocPassResourcesDesc)
                    {
                        ib_staticArrayRange(resourceBindings)
                    });

    ibr_ResourceState inStates[ibr_DefaultTexture_Count];
    for (uint32_t i = 0; i < ibr_DefaultTexture_Count; i++)
    {
        inStates[i] = ibr_textureState(&writeResources[i], ibr_TextureState_TransferDst, VK_PIPELINE_STAGE_2_COPY_BIT);
    }

    ibr_beginTransferPass(graph, commandBuffer, (ibr_BeginTransferPassDesc)
                         {
                             .ResourceStates = ib_staticArrayRange(inStates),
                             .PassName = "Default Resource Upload"
                         });

    ibr_WriteResourceDesc resourceWrites[] =
    {
        [ibr_DefaultTexture_White] =
        {
            &writeResources[ibr_DefaultTexture_White],
            .Data = (uint8_t[]) { 255, 255, 255, 255 },
            .Size = 4
        },
        [ibr_DefaultTexture_Checkerboard] =
        {
            &writeResources[ibr_DefaultTexture_Checkerboard],
            .Data = (uint8_t[]) 
            {
                255, 255, 255, 255, 0, 0, 0, 0,
                0, 0, 0, 0, 255, 255, 255, 255,
            },
            .Size = 16
        }
    };

    static_assert(ib_arrayCount(resourceWrites) == ibr_DefaultTexture_Count);

    ibr_writeResources(graph, commandBuffer, (ibr_WriteResourcesDesc)
                       {
                           ib_staticArrayRange(resourceWrites)
                       });


    // We don't know who will use this texture. Block access on all stages.
    ibr_ResourceState outStates[ibr_DefaultTexture_Count];
    for (uint32_t i = 0; i < ibr_DefaultTexture_Count; i++)
    {
        outStates[i] = ibr_textureState(&writeResources[i], ibr_TextureState_Read, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }
    ibr_barriers(graph, commandBuffer, (ibr_BarriersDesc)
                 {
                     ib_staticArrayRange(outStates)
                 });

    ibr_endTransferPass(graph, commandBuffer);
}

void ibr_killDefaultResources(ib_Core* core, ibr_DefaultResources* resources)
{
    for (uint32_t i = 0; i < ibr_DefaultTexture_Count; i++)
    {
        ib_freeTexture(core, &resources->Textures[i]);
    }
}
