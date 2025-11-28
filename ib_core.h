// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#ifndef IB_CORE_H
#define IB_CORE_H

#define VK_PROTOTYPES
#define VK_NO_STDDEF_H
#include <vulkan/vulkan.h>

#include <stdint.h>

// Include options:
// #define IB_DEBUG

#include "ib_util.h"
#include "ib_allocator.h"

static VkAllocationCallbacks *const ib_NoVkAllocator = NULL;

#ifndef ib_FramebufferCount
#define ib_FramebufferCount 2
#endif // ib_FramebufferCount

typedef struct ib_Core ib_Core;

// Timeline semaphore
typedef struct
{
    VkSemaphore Semaphore;
    uint64_t LastSignalValue;
} ib_timelineSemaphore;

ib_timelineSemaphore ib_allocTimelineSemaphore(ib_Core* core, uint64_t initialValue);
void ib_freeTimelineSemaphore(ib_Core* core, ib_timelineSemaphore* semaphore);
void ib_waitTimelineSemaphore(ib_Core* core, ib_timelineSemaphore* semaphore);

// Staging
#define MaxTransientStagingCommandBuffers 256
typedef struct
{
    VkDevice LogicalDevice;
    iba_StackAllocator StackAllocator;

	VkCommandPool TransferCommandPool;
	VkCommandBuffer TransientCommandBuffers[MaxTransientStagingCommandBuffers];
	uint32_t ActiveCommandBuffers;
    VkSemaphore TimelineSemaphore;
    uint64_t LastSemaphoreSignal;
} ib_Staging;

typedef struct
{
    VkBuffer Buffer;
    void* Memory;
    size_t Offset;
    uint64_t SemaphoreSignalValue;
} ib_StagingBuffer;

typedef struct
{
    size_t Size;
    size_t Alignment;
} ib_StagingRequest;

typedef struct
{
    VkDevice LogicalDevice;
    uint32_t TransferQueueIndex;
    iba_GpuAllocator* Allocator;
} ib_StagingDesc;

void ib_initStaging(ib_StagingDesc desc, ib_Staging* outStaging);
void ib_killStaging(ib_Staging* staging);
ib_StagingBuffer ib_requestStagingBuffer(ib_Staging* staging, ib_StagingRequest request);

// Core API
enum
{
    ib_Queue_Present = 0,
    ib_Queue_Graphics,
    ib_Queue_Compute,
    ib_Queue_Transfer,
    ib_Queue_Count,
    ib_Queue_Unknown
};
typedef uint32_t ib_Queue;

enum
{
    ib_Sampler_LinearRepeat = 0,
    ib_Sampler_LinearClamp,
    ib_Sampler_CompareLess,
	ib_Sampler_NearestClamp,
    ib_Sampler_Count
};

enum
{
    ib_DefaultTexture_White = 0,
    ib_DefaultTexture_Count
};
	
typedef struct
{
    // Win32
    void const* Win32MainWindowHandle;
    void const* Win32MainInstanceHandle;
} ib_CoreDesc;

void ib_initCore(ib_CoreDesc desc, ib_Core* outCore);
void ib_killCore(ib_Core* core);
void ib_flushStaging(ib_Core* core, ib_Staging* staging);

// Command buffer
typedef struct
{
    ib_range(VkCommandBuffer) OutCommandBuffers;
    ib_Queue Queue;
    VkCommandPool Pool;
} ib_AllocCommandBuffersDesc;

typedef struct
{
    ib_Queue Queue;
    VkCommandPool Pool;
} ib_AllocCommandBufferDesc;

VkCommandBuffer ib_allocCommandBuffer(ib_Core* core, ib_AllocCommandBufferDesc desc);
void ib_allocCommandBuffers(ib_Core* core, ib_AllocCommandBuffersDesc desc);
void ib_freeCommandBuffer(ib_Core* core, ib_Queue queue, VkCommandBuffer commands);
void ib_freeCommandBuffers(ib_Core* core, ib_Queue queue, uint32_t commandCount, VkCommandBuffer* commands);

// Utility command buffers
VkCommandBuffer ib_allocAndBeginCommandBuffer(ib_Core* core, ib_Queue queue);
void ib_endAndSubmitCommandBuffer(ib_Core* core, VkCommandBuffer commandBuffer, ib_Queue queue);

// Texture
typedef struct
{
    iba_GpuAllocation Allocation;
    VkImage Image;
    VkImageView View;
    VkImageAspectFlags Aspect;

    VkExtent3D Extent;
    VkFormat Format;
	uint32_t MipCount;
	uint32_t LayerCount;
} ib_Texture;

typedef struct
{
    VkImageUsageFlags Usage;
    VkFormat Format;
    VkExtent3D Extent;
    VkImageAspectFlags Aspect;
	uint32_t MipCount;
	uint32_t LayerCount;
    char const* DebugName;
    struct 
    {
        void const* Data;
        size_t Size;
        size_t Alignment;
    } InitialWrite;
} ib_TextureDesc;

typedef struct
{
    ib_Texture* Texture;
    void const* Data;
    size_t Size;
    size_t Alignment;
} ib_WriteToTextureDesc;

ib_Texture ib_allocTexture(ib_Core* core, ib_TextureDesc desc);
void ib_freeTexture(ib_Core* core, ib_Texture* texture);
void ib_writeToTexture(ib_Core* core, ib_WriteToTextureDesc desc);

uint32_t ib_formatToSize(VkFormat format);
inline size_t ib_textureSize(ib_Texture const* texture, uint32_t mip)
{
	// TODO: Store allocation size with texture instead.
	return (size_t)(texture->Extent.width >> mip)
		* (size_t)(texture->Extent.height >> mip)
		* ib_formatToSize(texture->Format)
		* (texture->LayerCount > 0 ? texture->LayerCount : 1);
}

typedef struct
{
    ib_Texture const* Texture;
    VkAccessFlags SourceAccessMask;
    VkAccessFlags DestAccessMask;
    VkPipelineStageFlags2 SourceStageMask;
    VkPipelineStageFlags2 DestStageMask;
    VkImageLayout OldLayout;
    VkImageLayout NewLayout;
    ib_Queue SourceQueue;
    ib_Queue DestQueue;
} ib_TextureBarrierDesc;

VkImageMemoryBarrier2 ib_createTextureBarrier(ib_Core* core, ib_TextureBarrierDesc desc);

// Buffer
typedef struct
{
    VkBuffer VulkanBuffer;
    VkDeviceAddress DeviceAddress;
    iba_GpuAllocation Allocation;
	size_t Size;
} ib_Buffer;

typedef struct
{
    VkBufferUsageFlags Usage;
    size_t Size;
    VkMemoryPropertyFlags RequiredMemoryFlags;
    VkMemoryPropertyFlags PreferredMemoryFlags;
    char const* DebugName;
    struct 
    {
        void const* Data;
        size_t Size;
        size_t Alignment;
        size_t WriteOffset;
    } InitialWrite;
} ib_BufferDesc;

typedef struct
{
    ib_Buffer* Buffer;
    void const* Data;
    size_t Size;
    size_t Alignment;
    size_t WriteOffset;
} ib_WriteToBufferDesc;

ib_Buffer ib_allocBuffer(ib_Core* core, ib_BufferDesc desc);
void ib_freeBuffer(ib_Core* core, ib_Buffer* buffer);
void ib_writeToBuffer(ib_Core* core, ib_WriteToBufferDesc desc);

// Surface
typedef struct
{
    VkSurfaceKHR VulkanSurface;
    VkSurfaceFormatKHR Format;
    VkExtent3D Extent;
    VkPresentModeKHR PresentMode;
    VkSwapchainKHR Swapchain;

    struct
    {
        VkSemaphore AcquireSemaphore;
    } Framebuffers[ib_FramebufferCount];
    ib_Texture SwapchainTextures[ib_FramebufferCount];
} ib_Surface;

typedef struct
{
    void const* Win32WindowHandle;
    void const* Win32InstanceHandle;
    bool UseVSync;
    bool SRGB;
} ib_SurfaceDesc;

ib_Surface ib_allocWin32Surface(ib_Core* core, ib_SurfaceDesc desc);
void ib_freeSurface(ib_Core* core, ib_Surface* surface);

enum
{
    ib_SurfaceState_Ok = 0,
    ib_SurfaceState_Error,
    ib_SurfaceState_ShouldRebuild,
};
typedef uint32_t ib_SurfaceState;

typedef struct
{
    ib_Surface* Surface;
    uint32_t Framebuffer;
} ib_PrepareSurfaceDesc;

typedef struct
{
    uint32_t SwapchainTextureIndex;
    ib_SurfaceState SurfaceState;
} ib_PrepareSurfaceResult;

ib_PrepareSurfaceResult ib_prepareSurface(ib_Core* core, ib_PrepareSurfaceDesc prepareDesc);

typedef struct
{
    ib_Surface* Surface;
    uint32_t SwapchainTextureIndex;
    VkSemaphore WaitSemaphore;
} ib_PresentSurfaceDesc;

ib_SurfaceState ib_presentSurface(ib_Core* core, ib_PresentSurfaceDesc presentDesc);
void ib_rebuildSurface(ib_Core* core, ib_Surface* surface);

// Graphics pipeline
typedef struct 
{
    uint32_t Index;
    VkShaderStageFlags Shaders;
    VkDescriptorType Type;
    uint32_t ArraySize;
    VkDescriptorBindingFlagsEXT BindingFlags;
    bool UseImmutableSamplers;
} ib_ShaderInputDesc;

typedef ib_range(ib_ShaderInputDesc const) ib_ShaderInputRange;

typedef struct
{
    ib_ShaderInputRange Inputs;
} ib_ShaderInputLayoutDesc;

typedef struct
{
    VkDescriptorSetLayout DescriptorSetLayout;
} ib_ShaderInputLayout;

ib_ShaderInputLayout ib_allocShaderInputLayout(ib_Core* core, ib_ShaderInputLayoutDesc blockLayoutDesc);
void ib_freeShaderInputLayout(ib_Core* core, ib_ShaderInputLayout* layout);

typedef struct
{
	ib_Texture const* Texture;
	VkImageLayout Layout;
	VkImageView View;
} ib_ShaderInputWriteTexture;

typedef struct
{
	ib_Buffer const* Buffer;
	size_t Offset;
	size_t Size;
} ib_ShaderInputWriteBuffer;


typedef struct
{
    ib_ShaderInputDesc const* Desc;

    // Write to the appropriate member
	uint32_t ArrayIndex;
    ib_ShaderInputWriteBuffer BufferInput;
    ib_ShaderInputWriteTexture TextureInput;
    VkAccelerationStructureKHR AccelerationStructureInput;
    VkSampler SamplerInput;
} ib_ShaderInputWrite;

typedef struct
{
    VkDescriptorSet DescriptorSet;
} ib_ShaderInput;

typedef struct 
{
    ib_ShaderInputLayout const* Layout;
    ib_range(ib_ShaderInputWrite const) Inputs;

    VkDescriptorPool Pool;
} ib_AllocShaderInputDesc;

ib_ShaderInput ib_allocShaderInput(ib_Core* core, ib_AllocShaderInputDesc allocDesc);
void ib_freeShaderInput(ib_Core* core, ib_ShaderInput* input);

typedef struct
{
    ib_ShaderInput* ShaderInput;
    ib_range(ib_ShaderInputWrite const) Inputs;
} ib_WriteToShaderInputDesc;

void ib_writeToShaderInput(ib_Core* core, ib_WriteToShaderInputDesc desc);

typedef struct
{
    char const* EntryPoint;
    void const* Code;
    size_t CodeSize;
    VkShaderStageFlagBits Stage; 
    uint32_t RequiredWaveSize;
} ib_ShaderDesc;

typedef struct
{
    VkPipelineColorBlendAttachmentState BlendDesc;
    VkFormat Format;
} ib_RenderTargetDesc;

typedef struct
{
    VkPipelineDepthStencilStateCreateInfo DepthState;
    VkFormat Format;
} ib_DepthRenderTargetDesc;

typedef struct
{
	ib_ShaderInputRange Inline;
	ib_ShaderInputLayout const* External;
} ib_PipelineShaderInputDesc;

#define ib_MaxShaderInputLayoutPerPipeline 4

typedef struct
{
    ib_range(ib_ShaderDesc const) ShaderDescs;
    ib_range(VkPushConstantRange const) PushConstants;
    ib_PipelineShaderInputDesc ShaderInputs[ib_MaxShaderInputLayoutPerPipeline];

    VkPipelineVertexInputStateCreateInfo VertexDesc;
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyDesc;
    VkPipelineRasterizationStateCreateInfo RasterizationDesc;

    ib_range(ib_RenderTargetDesc const) RenderTargetDescs;
    ib_DepthRenderTargetDesc DepthDesc;
} ib_GraphicsPipelineDesc;

typedef struct
{
    VkPipelineLayout Layout;
    VkPipeline VulkanPipeline;
    ib_ShaderInputLayout InlineShaderInputLayouts[ib_MaxShaderInputLayoutPerPipeline]; // Owned shader input layouts.
	uint32_t InlineShaderInputLayoutCount;
} ib_GraphicsPipeline;

ib_GraphicsPipeline ib_allocGraphicsPipeline(ib_Core* core, ib_GraphicsPipelineDesc desc);
void ib_freeGraphicsPipeline(ib_Core* core, ib_GraphicsPipeline* pipeline);

// Allow for simpler reloading that's resilient to missing shader code.
inline void ib_reloadGraphicsPipeline(ib_Core* core, ib_GraphicsPipeline* pipeline, ib_GraphicsPipelineDesc desc)
{
	bool allShadersValid = true;
	for (uint32_t i = 0; i < desc.ShaderDescs.Count; i++)
	{
		if (desc.ShaderDescs.Data[i].Code == NULL)
		{
			allShadersValid = false;
			break;
		}
	}

	if (allShadersValid)
	{
		ib_freeGraphicsPipeline(core, pipeline);
		*pipeline = ib_allocGraphicsPipeline(core, desc);
	}
}

typedef struct
{
    ib_ShaderDesc ShaderDesc;
    ib_range(VkPushConstantRange const) PushConstants;
	ib_PipelineShaderInputDesc ShaderInputs[ib_MaxShaderInputLayoutPerPipeline];
} ib_ComputePipelineDesc;

typedef struct
{
    VkPipelineLayout Layout;
    VkPipeline VulkanPipeline;
	ib_ShaderInputLayout InlineShaderInputLayouts[ib_MaxShaderInputLayoutPerPipeline]; // Owned shader input layouts.
	uint32_t InlineShaderInputLayoutCount;
} ib_ComputePipeline;

ib_ComputePipeline ib_allocComputePipeline(ib_Core* core, ib_ComputePipelineDesc desc);
void ib_freeComputePipeline(ib_Core* core, ib_ComputePipeline* pipeline);

// Allow for simpler reloading that's resilient to missing shader code.
inline void ib_reloadComputePipeline(ib_Core* core, ib_ComputePipeline* pipeline, ib_ComputePipelineDesc desc)
{
	if (desc.ShaderDesc.Code != NULL)
	{
		ib_freeComputePipeline(core, pipeline);
		*pipeline = ib_allocComputePipeline(core, desc);
	}
}

// Utility
void ib_printComputePipelineStatistics(ib_Core* core, ib_ComputePipeline const* pipeline);

// Timer
typedef struct
{
    VkQueryPool TimestampPool;
    uint32_t NextTimestamp;
    uint32_t MaxTimestampCount;
} ib_TimerManager;

typedef struct
{
    uint32_t TimestampIndex;
} ib_Timer;

typedef struct
{
    ib_Core* Core;
    uint32_t MaxTimerCount;
} ib_TimerManagerDesc;

void ib_initTimerManager(ib_TimerManagerDesc desc, ib_TimerManager* outManager);
void ib_killTimerManager(ib_Core* core, ib_TimerManager* manager);
void ib_resetTimers(ib_TimerManager* manager, VkCommandBuffer commandBuffer);
void ib_resetTimersCPU(ib_Core* core, ib_TimerManager* manager);
ib_Timer ib_beginTimer(ib_TimerManager* manager, VkCommandBuffer commandBuffer);
void ib_endTimer(ib_TimerManager* manager, VkCommandBuffer commandBuffer, ib_Timer* timer);

// Returns -1.0 if query not ready.
static double const ib_TimerQueryNotReady = -1.0f;
double ib_queryTimer(ib_Core* core, ib_TimerManager* manager, ib_Timer const* timer, bool blocking);
inline double ib_queryTimerBlocking(ib_Core* core, ib_TimerManager* manager, ib_Timer const* timer)
{
	return ib_queryTimer(core, manager, timer, true);
}

typedef struct ib_Core
{
    VkInstance Instance;
    VkPhysicalDevice PhysicalDevice;
    VkPhysicalDeviceLimits DeviceLimits;
    VkDevice LogicalDevice;

    struct
    {
		VkDescriptorPool Pool;
    } Descriptors;
    VkPipelineCache PipelineCache;

    iba_GpuAllocator Allocator;
    ib_Staging Staging;

    struct
    {
        uint32_t Index;
        VkQueue Queue;
        VkCommandPool CommandPool;
    } Queues[ib_Queue_Count];

    VkSampler Samplers[ib_Sampler_Count];
    ib_Texture DefaultTextures[ib_DefaultTexture_Count];

    bool RaytracingEnabled;
} ib_Core;

// Utility constants to reduce friction when creating graphics pipelines.
static VkPipelineRasterizationStateCreateInfo const ib_RasterizationCullBackFaceCCW =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .lineWidth = 1.0f
};

static VkPipelineRasterizationStateCreateInfo const ib_RasterizationNoCull =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    .lineWidth = 1.0f
};

static VkPipelineVertexInputStateCreateInfo const ib_VertexInputNull =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
};

static VkPipelineInputAssemblyStateCreateInfo const ib_InputAssemblyTriangleList =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
};

static VkPipelineInputAssemblyStateCreateInfo const ib_InputAssemblyLineList =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST
};

static VkPipelineDepthStencilStateCreateInfo const ib_DepthStateLessTestWriteNoStencil =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_TRUE,
    .depthWriteEnable = VK_TRUE,
    .depthCompareOp = VK_COMPARE_OP_LESS,
    .stencilTestEnable = VK_FALSE,
    .minDepthBounds = 0.0f,
    .maxDepthBounds = 1.0f,
};

static VkPipelineDepthStencilStateCreateInfo const ib_DepthStateLessTestNoWriteNoStencil =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_TRUE,
    .depthWriteEnable = VK_FALSE,
    .depthCompareOp = VK_COMPARE_OP_LESS,
    .stencilTestEnable = VK_FALSE,
    .minDepthBounds = 0.0f,
    .maxDepthBounds = 1.0f,
};

static VkPipelineDepthStencilStateCreateInfo const ib_DepthStateNoTestNoWrite =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
};

static VkPipelineColorBlendAttachmentState const ib_BlendDescAlphaBlend =
{
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
};

static VkPipelineColorBlendAttachmentState const ib_BlendDescNoBlend =
{
    .blendEnable = VK_FALSE,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
};

static VkPipelineColorBlendAttachmentState const ib_BlendDescDualSourceBlend =
{
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
};

static VkPipelineColorBlendAttachmentState const ib_BlendDescAdditiveBlend =
{
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
};

// Raytracing

typedef struct
{
	ib_Core* Core;
	uint32_t AccelerationStructureScratchBufferAlignment;
	uint32_t SBTGroupHandleSize;
	uint32_t SBTGroupHandleAlignment;
	uint32_t SBTGroupBaseAlignment;
	uint32_t SBTGroupHandleSizeAligned;
} ib_Raytracing;

void ib_initRaytracing(ib_Core* core, ib_Raytracing* raytracing);
void ib_killRaytracing(ib_Raytracing* raytracing);

void ib_initRaytracingScratch(ib_Raytracing* raytracing, iba_StackAllocator* allocator);
void ib_killRaytracingScratch(iba_StackAllocator* allocator);

typedef struct
{
    VkPipelineLayout Layout;
    VkPipeline VulkanPipeline;
    ib_ShaderInputLayout ShaderInputLayout;

    ib_Buffer ShaderBindingTable;
    VkStridedDeviceAddressRegionKHR RaygenShaderRegion;
    VkStridedDeviceAddressRegionKHR MissShaderRegion;
    VkStridedDeviceAddressRegionKHR HitShaderRegion;
    VkStridedDeviceAddressRegionKHR CallableShaderRegion;

} ib_RaytracePipeline;

typedef struct {
    ib_ShaderDesc ClosestHit;
    ib_ShaderDesc AnyHit;
    ib_ShaderDesc Intersection;
    uint32_t InstanceIdx;
} ib_HitGroupShadersDescription;

typedef struct {
    ib_ShaderDesc Miss;
    uint32_t InstanceIdx;
} ib_MissGroupShaderDescription;

typedef struct
{
    ib_ShaderDesc RaygenShaderDesc;
    ib_range(ib_HitGroupShadersDescription) HitGroupsDesc;
    ib_range(ib_MissGroupShaderDescription) MissGroupDesc;
    ib_range(VkPushConstantRange const) PushConstants;
    ib_ShaderInputRange ShaderInputs;
    ib_range(ib_ShaderInputLayout const*) ExternalShaderInputLayouts;
} ib_RaytracePipelineDesc;

ib_RaytracePipeline ib_allocRaytracePipeline(ib_Raytracing* raytracing, ib_RaytracePipelineDesc desc);
void ib_freeRaytracePipeline(ib_Raytracing* raytracing, ib_RaytracePipeline* pipeline);

typedef struct
{
	ib_Buffer Buffer;
    VkAccelerationStructureKHR AccelerationStructure;
    VkDeviceAddress Address;
} ib_AccelerationStructureData;

// BLAS and TLAS share memory layout but conceptually different.
typedef struct
{
	ib_AccelerationStructureData Data;
} ib_BLAS;

typedef struct
{
    ib_AccelerationStructureData Data;
} ib_TLAS;

typedef struct
{
    VkGeometryFlagsKHR GeometryFlags;
    VkBuildAccelerationStructureFlagBitsKHR AccelerationStructureFlags;

    struct
    {
        ib_Buffer* Vertices;
        uint32_t VertexCount;
        VkFormat VertexFormat;
        VkDeviceSize VertexStride;

        ib_Buffer* Indices;
        VkIndexType IndexType;

        uint32_t TrianglesCount;
    } Triangles;

} ib_BLASDesc;


typedef struct
{
    VkBuildAccelerationStructureFlagBitsKHR AccelerationStructureFlags;
    ib_Buffer* InstancesBuffer;
    uint32_t InstancesCount;
} ib_TLASDesc;


ib_BLAS ib_allocBLAS(ib_Raytracing* raytracing, ib_BLASDesc desc, iba_StackAllocator* scratchMemoryStack, ib_timelineSemaphore* buildingSemaphore);
ib_TLAS ib_allocTLAS(ib_Raytracing* raytracing, ib_TLASDesc desc, iba_StackAllocator* scratchMemoryStack, ib_timelineSemaphore* buildingSemaphore);
void ib_freeBLAS(ib_Raytracing* raytracing, ib_BLAS* accelerationStructure);
void ib_freeTLAS(ib_Raytracing* raytracing, ib_TLAS* accelerationStructure);

#endif // IB_CORE_H