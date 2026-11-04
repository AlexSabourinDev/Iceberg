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
    void const* Data;
    size_t Size;
    size_t Alignment;
} ibr_WriteData;

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

typedef struct ibr_TransientTexture
{
    ib_Texture Texture;
    struct ibr_TransientTexture* Next;
} ibr_TransientTexture;

typedef struct ibr_TransientBuffer
{
    ib_Buffer Buffer;
    struct ibr_TransientBuffer* Next;
} ibr_TransientBuffer;

typedef struct ibr_TransientImageView
{
    VkImageView View;
    struct ibr_TransientImageView* Next;
} ibr_TransientImageView;

typedef struct ibr_TransientCommandBuffer
{
    VkCommandBuffer CommandBuffer;
    struct ibr_TransientCommandBuffer* Next;
} ibr_TransientCommandBuffer;

typedef struct ibr_TransientProfileScope
{
    ibr_ProfilingScope Scope;
    struct ibr_TransientProfileScope* Next;
} ibr_TransientProfileScope;

typedef struct ibr_TransientScopeTiming
{
    ibr_ScopeTiming Timing;
    struct ibr_TransientScopeTiming* Next;
} ibr_TransientScopeTiming;

typedef struct ibr_RenderGraph
{
    ib_Core* Core;
    iba_StackAllocator FrameGPUStack;
    iba_StackAllocator FrameCPUStack;

    ibr_TransientTexture* TransientTextures;
    ibr_TransientBuffer* TransientBuffers;
    ibr_TransientImageView* TransientImageViews;
    VkDescriptorPool TransientDescriptorPool;
    VkCommandPool TransientCommandPools[ib_Queue_Count];
    ibr_TransientCommandBuffer* TransientCommandBuffers[ib_Queue_Count];
    ibr_TransientCommandBuffer* ActiveCommandBuffer[ib_Queue_Count];
    VkFence FrameFence;
    VkSemaphore PresentSemaphore;
    VkSemaphore SwapchainAcquireSemaphore;
    uint32_t SwapchainTextureIndex;
    ib_Texture* SwapchainTexture;
    VkExtent2D ScreenExtent;

    ib_TimerManager TimerManager;
    ibr_TransientProfileScope* ActiveProfilingScopes;
    ibr_TransientProfileScope* CompletedScopes;
    uint32_t CurrentProfilingDepth;

    ibr_TransientScopeTiming* PreviousFrameTimings;
} ibr_RenderGraph;

void ibr_initRenderGraphs(ib_Core* core, ibr_RenderGraph* graphs, uint32_t graphCount);
void ibr_killRenderGraphs(ib_Core* core, ibr_RenderGraph* graphs, uint32_t graphCount);

typedef struct
{
    uint32_t FrameIndex;
    ib_Surface* Surface;
} ibr_BeginFrameDesc;

bool ibr_beginFrame(ibr_RenderGraph* graph, ibr_BeginFrameDesc desc);
// I don't love this API - needing to remember to call beginFrame and THEN begin upload once you have a command buffer isn't ideal.
void ibr_beginUpload(ibr_RenderGraph* graph, VkCommandBuffer commandBuffer);
void ibr_endUpload(ibr_RenderGraph* graph);
void ibr_endFrame(ibr_RenderGraph* graph);

void* ibr_allocTransientMemory(ibr_RenderGraph* graph, size_t size, size_t alignment);

typedef struct
{
    void* CpuMemory;
    VkBuffer Buffer;
    size_t BufferOffset;
} ibr_TransientGpuMemory;

ibr_TransientGpuMemory ibr_allocTransientGpuMemory(ibr_RenderGraph* graph, size_t size, size_t alignment);

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

    VkPipelineStageFlags2 LastReleaseStageMask;
    VkAccessFlags2 LastReleaseAccessMask;
} ibr_Resource;

typedef struct
{
    ibr_ResourceType Type;
    ibr_ResourceFlags Flags;
    union
    {
        struct
        {
            ib_Texture* Texture;
            VkImageLayout TextureLayout;
        };
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
    ib_srange(ibr_AllocResourceBinding, 8) ResourceBindings;
} ibr_AllocPassResourcesDesc;

typedef struct
{
    ib_Texture* Texture;
    uint32_t BaseMip;
    uint32_t LayerIndex;
} ibr_AllocTransientImageViewDesc;

ibr_Resource ibr_allocPassResource(ibr_RenderGraph* graph, VkCommandBuffer commands, ibr_ResourceDesc resourceDesc);
void ibr_allocPassResources(ibr_RenderGraph* graph, VkCommandBuffer commands, ibr_AllocPassResourcesDesc desc);
VkImageView ibr_allocTransientImageView(ibr_RenderGraph* graph, ibr_AllocTransientImageViewDesc desc);
ib_ShaderInput ibr_allocTransientShaderInput(ibr_RenderGraph* graph, ib_AllocShaderInputDesc desc);
VkCommandBuffer ibr_allocTransientCommandBuffer(ibr_RenderGraph* graph, ib_Queue queue);

ibr_ResourceDesc ibr_textureResourceDesc(ib_Texture* texture, VkImageLayout layout);

typedef enum
{
    ibr_TransientTextureFlag_StorageBit = 0x01,
    ibr_TransientTextureFlag_ColorAttachmentBit = 0x02,
    ibr_TransientTextureFlag_TransferSrcBit = 0x04,
    ibr_TransientTextureFlag_TransferDstBit = 0x08,
    ibr_TransientTextureFlag_DepthBit = 0x10
} ibr_TransientTextureFlags;

ibr_ResourceDesc ibr_transientTextureResourceDesc(VkExtent3D extent, VkFormat format, ibr_TransientTextureFlags flags, char const* debugName);

typedef enum
{
    ibr_TransientBufferFlag_Device = 0x0001,
    ibr_TransientBufferFlag_Host = 0x0002,
    ibr_TransientBufferFlag_MemoryMask = 0x00FF,
    ibr_TransientBufferFlag_UniformBufferBit = 0x0100,
    ibr_TransientBufferFlag_StorageBufferBit = 0x0200,
} ibr_TransientBufferFlags;

ibr_ResourceDesc ibr_transientBufferResourceDesc(size_t size, ibr_TransientBufferFlags flags, char const* debugName);

typedef struct
{
    ibr_Resource* Resource;
    void const* Data;
    size_t Size;
    size_t Alignment;
} ibr_WriteResourceDesc;

typedef struct
{
    ib_srange(ibr_WriteResourceDesc, 8) Writes;
} ibr_WriteResourcesDesc;
void ibr_writeResources(ibr_RenderGraph* graph, VkCommandBuffer commands, ibr_WriteResourcesDesc desc);

static void ibr_writeResource(ibr_RenderGraph* graph, VkCommandBuffer commands, ibr_Resource* resource, ibr_WriteData writeData)
{
    ibr_writeResources(graph, commands, (ibr_WriteResourcesDesc)
                       {
                           (ibr_WriteResourceDesc)
                           {
                               resource,
                               writeData.Data,
                               writeData.Size,
                               writeData.Alignment
                           }
                       });
}

typedef struct
{
    VkSemaphore Semaphore;
    uint64_t Value;
} ibr_SemaphoreSubmit;

typedef struct
{
    ib_Queue Queue;
    ib_srange(VkCommandBuffer, 4) CommandBuffers;
    ib_srange(ibr_SemaphoreSubmit, 4) WaitSemaphores;
    ib_srange(ibr_SemaphoreSubmit, 4) SignalSemaphores;
    VkFence SubmitFence;
} ibr_SubmitCommandBufferDesc;
void ibr_submitCommandBuffers(ibr_RenderGraph* graph, ibr_SubmitCommandBufferDesc desc);

typedef struct
{
    ib_Surface* Surface;
    ibr_RenderGraph* Graph;
} ibr_PresentDesc;
void ibr_present(ibr_PresentDesc desc);

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

    VkPipelineStageFlags2 AcquireAndReleaseStageMask; // Convenience value can be used over Acquire/Release values.
    VkPipelineStageFlags2 AcquireStageMask; // When do we need our resource to be available
    VkPipelineStageFlags2 ReleaseStageMask; // When are future passes free to use this resource
} ibr_ResourceState;

typedef ib_srange(ibr_ResourceState, 8) ibr_ResourceStateRange;
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
    ib_srange(ibr_RenderTargetState, 4) RenderTargets; // Rendertargets will be appropriately transitioned
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
inline void ibr_beginTransferPass(ibr_RenderGraph* graph, VkCommandBuffer cmd, ibr_BeginTransferPassDesc desc)
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
    ibr_TextureState_RenderTarget,
    ibr_TextureState_Mask = 0x0F, // Up to 16 RW states
    ibr_TextureStateFlag_Depth = 0x10,
};

typedef uint32_t ibr_TextureState;

ibr_ResourceState ibr_textureState(ibr_Resource* resource, ibr_TextureState state, VkPipelineStageFlags2 stage);
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
ibr_ResourceState ibr_bufferState(ibr_Resource* resource, ibr_BufferState state, VkPipelineStageFlags2 stage);

enum
{
    ibr_DefaultTexture_White = 0,
    ibr_DefaultTexture_Checkerboard = 1,
    ibr_DefaultTexture_Count
};

// I don't love this API, but its a start. We'll revisit later.
typedef struct
{
    ib_Texture Textures[ibr_DefaultTexture_Count];
} ibr_DefaultResources;

void ibr_initDefaultResources(ibr_RenderGraph* graph, VkCommandBuffer commandBuffer, ibr_DefaultResources* resources);
void ibr_killDefaultResources(ib_Core* core, ibr_DefaultResources* resources);

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // IB_RENDERGRAPH_H