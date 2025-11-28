// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#ifndef IB_RENDERGRAPH_H
#define IB_RENDERGRAPH_H

#include "ib_core.h"

// TODO: Add a transient buffer allocator

#ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable:4201)   /* nonstandard extension used: nameless struct/union */
#endif // _MSC_VER

typedef struct
{
	ib_Timer Timer;
	char const* Name;
} ibr_ProfilingScope;

typedef struct
{
	double Timing;
	char const* Name;
} ibr_ScopeTiming;

#define ibr_MaxTransientTextureCount 128
#define ibr_MaxTransientBufferCount 128
#define ibr_MaxTransientImageView 128
#define ibr_MaxProfilingScopeCount 1024
#define ibr_MaxTransientCommandBuffer 128
typedef struct ibr_RenderGraph
{
	ib_Core* Core;
	ib_fsq(ib_Texture, ibr_MaxTransientTextureCount) TransientTextures;
	ib_fsq(ib_Buffer, ibr_MaxTransientBufferCount) TransientBuffers;
	ib_fsq(VkImageView, ibr_MaxTransientImageView) TransientImageViews;
	VkDescriptorPool TransientDescriptorPool;
	VkCommandPool TransientCommandPools[ib_Queue_Count];
	VkCommandBuffer TransientCommandBuffers[ib_Queue_Count][ibr_MaxTransientCommandBuffer];
	uint32_t ActiveCommandBuffers[ib_Queue_Count];
	VkFence FrameFence;
	VkSemaphore FrameSemaphore;
	uint32_t SwapchainTextureIndex;
	ib_Texture* SwapchainTexture;
	VkExtent2D ScreenExtent;

	ib_TimerManager TimerManager;
	ib_fsq(ibr_ProfilingScope, ibr_MaxProfilingScopeCount) ActiveProfilingScopes;
	ib_fsq(ibr_ProfilingScope, ibr_MaxProfilingScopeCount) CompletedScopes;
	uint32_t CurrentProfilingDepth;

	ib_fsq(ibr_ScopeTiming, ibr_MaxProfilingScopeCount) PreviousFrameTimings;
} ibr_RenderGraph;

typedef struct
{
	ibr_RenderGraph Graphs[ib_FramebufferCount];
} ibr_RenderGraphPool;

ibr_RenderGraphPool ibr_allocRenderGraphPool(ib_Core* core);
void ibr_freeRenderGraphPool(ib_Core* core, ibr_RenderGraphPool* pool);

typedef struct
{
	uint32_t FrameIndex;
	ib_Surface* Surface;
} ibr_BeginFrameDesc;

ibr_RenderGraph* ibr_beginFrame(ibr_RenderGraphPool* pool, ibr_BeginFrameDesc desc);
void ibr_endFrame(ibr_RenderGraphPool* pool, ibr_RenderGraph* graph);

enum
{
	ibr_ResourceFlag_Transient = 0x01
};
typedef uint32_t ibr_ResourceFlags;

static VkExtent3D const ibr_ScreenExtent = { UINT32_MAX, UINT32_MAX };

enum
{
	ibr_ResourceType_Undefined = 0,
	ibr_ResourceType_Texture,
	ibr_ResourceType_Buffer
};
typedef uint32_t ibr_ResourceType;

typedef struct ibr_Resource
{
	ibr_ResourceType Type;
	union
	{
		struct
		{
			ib_Texture* Texture;
			VkImageLayout TextureLayout;
		};
		
		ib_Buffer* Buffer;
	};

	VkPipelineStageFlags LastReleaseStageMask;
	VkAccessFlags LastReleaseAccessMask;
} ibr_Resource;

typedef struct
{
	ibr_ResourceType Type;
	ibr_ResourceFlags Flags;
	union
	{
		ib_Texture* Texture;
		ib_TextureDesc TextureDesc;
		ib_Buffer* Buffer;
		ib_BufferDesc BufferDesc;
	};
} ibr_ResourceDesc;

typedef struct
{
	ibr_Resource* OutResource;
	ibr_ResourceDesc Desc;
} ibr_AllocResourceBinding;

typedef struct
{
	ib_range(ibr_AllocResourceBinding) ResourceBindings;
} ibr_AllocPassResourcesDesc;

typedef struct
{
	ib_Texture* Texture;
	uint32_t BaseMip;
	uint32_t LayerIndex;
} ibr_AllocTransientImageViewDesc;

ibr_Resource ibr_allocPassResource(ibr_RenderGraph* graph, ibr_ResourceDesc resourceDesc);
void ibr_allocPassResources(ibr_RenderGraph* graph, ibr_AllocPassResourcesDesc desc);
VkImageView ibr_allocTransientImageView(ibr_RenderGraph* graph, ibr_AllocTransientImageViewDesc desc);
ib_ShaderInput ibr_allocTransientShaderInput(ibr_RenderGraph* graph, ib_AllocShaderInputDesc desc);
VkCommandBuffer ibr_allocTransientCommandBuffer(ibr_RenderGraph* graph, ib_Queue queue);

typedef struct
{                        
	ib_ShaderInputLayout const* Layout;
	ib_ShaderInputRange ShaderInputs;
	ib_range(ibr_Resource*) Resources;
} ibr_ResourceToShaderInputDesc;
ib_ShaderInput ibr_resourcesToShaderInput(ibr_RenderGraph* graph, ibr_ResourceToShaderInputDesc desc);

typedef struct
{
	ibr_Resource* Resource;
	VkImageLayout Layout;
	VkAccessFlags AcquireAccessMask;
	VkAccessFlags ReleaseAccessMask;

	VkPipelineStageFlags AcquireAndReleaseStageMask; // Convenience value can be used over Acquire/Release values.
	VkPipelineStageFlags AcquireStageMask; // When do we need our resource to be available
	VkPipelineStageFlags ReleaseStageMask; // When are future passes free to use this resource
} ibr_ResourceState;

typedef ib_range(ibr_ResourceState) ibr_ResourceStateRange;
typedef struct
{
	ibr_ResourceStateRange ResourceStates;
} ibr_BarriersDesc;

void ibr_barriers(ibr_RenderGraph* graph, VkCommandBuffer cmd, ibr_BarriersDesc desc);

typedef struct
{
	ibr_Resource* Resource;
	VkAttachmentLoadOp LoadOp;
	VkAttachmentStoreOp StoreOp;
	VkClearValue ClearValue;

	// Default for color rendertargets is VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
	// Default for depth rendertargets is VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	VkAccessFlags AcquireAccessMask;
	VkAccessFlags ReleaseAccessMask;

	// Default for color rendertargets is VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	// Default for depth rendertargets is VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT and VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
	VkPipelineStageFlags AcquireStageMask; // When do we need our resource to be available
	VkPipelineStageFlags ReleaseStageMask; // When are future passes free to use this resource
} ibr_RenderTargetState;

typedef struct
{
	ib_range(ibr_RenderTargetState) RenderTargets; // Rendertargets will be appropriately transitioned
	ibr_RenderTargetState DepthTarget;

	ibr_ResourceStateRange OtherResourceStates; // Other resources that aren't included in our rendertargets.

	float MinDepth;
	float MaxDepth;

	char const* PassName; // Can be NULL
} ibr_BeginGraphicsPassDesc;

void ibr_beginGraphicsPass(ibr_RenderGraph* graph, VkCommandBuffer cmd, ibr_BeginGraphicsPassDesc desc);
void ibr_endGraphicsPass(ibr_RenderGraph* graph, VkCommandBuffer cmd);

typedef struct
{
	ibr_ResourceStateRange ResourceStates;
	char const* PassName; // Can be NULL
} ibr_BeginComputePassDesc;

void ibr_beginComputePass(ibr_RenderGraph* graph, VkCommandBuffer cmd, ibr_BeginComputePassDesc desc);
void ibr_endComputePass(ibr_RenderGraph* graph, VkCommandBuffer cmd);

typedef struct
{
	ibr_ResourceStateRange ResourceStates;
	char const* PassName; // Can be NULL
} ibr_BeginTransferPassDesc;

// Cheating - Compute and transfer do the same.
inline void ibr_beginTranserPass(ibr_RenderGraph* graph, VkCommandBuffer cmd, ibr_BeginTransferPassDesc desc)
{
	ibr_beginComputePass(graph, cmd, (ibr_BeginComputePassDesc)
						 {
							 desc.ResourceStates,
							 desc.PassName
						 });
}

inline void ibr_endTransferPass(ibr_RenderGraph* graph, VkCommandBuffer cmd)
{
	ibr_endComputePass(graph, cmd);
}

// Utility resource states

enum
{
	ibr_TextureState_Write = 0,
	ibr_TextureState_Read,
	ibr_TextureState_ReadWrite,
	ibr_TextureState_TransferSrc,
	ibr_TextureState_TransferDst,
	ibr_TextureState_Mask = 0x0F, // Up to 16 RW states
	ibr_TextureStateFlag_Depth = 0x10,
};

typedef uint32_t ibr_TextureState;

ibr_ResourceState ibr_textureState(ibr_Resource* resource, ibr_TextureState state, VkPipelineStageFlags stage);
ibr_ResourceState ibr_textureToPresentState(ibr_Resource* resource);

enum
{
	ibr_BufferState_Write = 0,
	ibr_BufferState_Read,
	ibr_BufferState_ReadWrite,
	ibr_BufferState_TransferSrc,
	ibr_BufferState_TransferDst
};

typedef uint32_t ibr_BufferState;
ibr_ResourceState ibr_bufferState(ibr_Resource* resource, ibr_BufferState state, VkPipelineStageFlags stage);

#ifdef _MSC_VER
        #pragma warning(pop)
#endif // _MSC_VER

#endif // IB_RENDERGRAPH_H