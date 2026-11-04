// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#include <iceberg/ib_core.h>

// #define VK_USE_PLATFORM_WIN32_KHR
// Don't include Windows.h in our vulkan header.
// Just include the vulkan win32 header here and forward declare the types we want to avoid including windows.h at all
// #include <Windows.h>
typedef void* HANDLE;
typedef void* HWND;
typedef void* HINSTANCE;
typedef wchar_t WCHAR;
typedef WCHAR const* LPCWSTR;
typedef struct _SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES;
typedef void* HMONITOR;
typedef unsigned long DWORD;
#include <vulkan/vulkan_win32.h>

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

// Vulkan functions

PFN_vkCreateDebugUtilsMessengerEXT ib_vkCreateDebugUtilsMessengerEXT = NULL;
PFN_vkDestroyDebugUtilsMessengerEXT ib_vkDestroyDebugUtilsMessengerEXT = NULL;
PFN_vkGetPipelineExecutableStatisticsKHR ib_vkGetPipelineExecutableStatisticsKHR;
PFN_vkGetPipelineExecutableInternalRepresentationsKHR ib_vkGetPipelineExecutableInternalRepresentationsKHR;
PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR ib_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR;
PFN_vkGetBufferDeviceAddressKHR ib_vkGetBufferDeviceAddressKHR;
PFN_vkSetDebugUtilsObjectNameEXT ib_vkSetDebugUtilsObjectNameEXT;
PFN_vkCmdBeginDebugUtilsLabelEXT ib_vkCmdBeginDebugUtilsLabelEXT;
PFN_vkCmdEndDebugUtilsLabelEXT ib_vkCmdEndDebugUtilsLabelEXT;
PFN_vkCmdBindDescriptorSets2KHR ib_vkCmdBindDescriptorSets2KHR;

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger)
{
    if (ib_vkCreateDebugUtilsMessengerEXT != NULL)
    {
        return ib_vkCreateDebugUtilsMessengerEXT(instance, createInfo, allocator, debugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks const* allocator)
{
    if (ib_vkDestroyDebugUtilsMessengerEXT != NULL)
    {
        ib_vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, allocator);
    }
}

VkResult vkGetPipelineExecutableStatisticsKHR(VkDevice device,
                                              VkPipelineExecutableInfoKHR const* executableInfo,
                                              uint32_t* statisticCount,
                                              VkPipelineExecutableStatisticKHR* statistics)
{
    if (ib_vkGetPipelineExecutableStatisticsKHR != NULL)
    {
        return ib_vkGetPipelineExecutableStatisticsKHR(device, executableInfo, statisticCount, statistics);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VkResult vkGetPipelineExecutableInternalRepresentationsKHR(
    VkDevice device,
    const VkPipelineExecutableInfoKHR* pExecutableInfo,
    uint32_t* pInternalRepresentationCount,
    VkPipelineExecutableInternalRepresentationKHR* pInternalRepresentations)
{
    if (ib_vkGetPipelineExecutableInternalRepresentationsKHR != NULL)
    {
        return ib_vkGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VkResult VKAPI_CALL vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    uint32_t*                                   pCounterCount,
    VkPerformanceCounterKHR*                    pCounters,
    VkPerformanceCounterDescriptionKHR*         pCounterDescriptions)
{
    if (ib_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR != NULL)
    {
        return ib_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
            physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VkDeviceAddress VKAPI_CALL vkGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo* pInfo)
{
    if (ib_vkGetBufferDeviceAddressKHR == NULL)
    {
        ib_vkCheck(VK_ERROR_EXTENSION_NOT_PRESENT);
        return 0;
    }

    return ib_vkGetBufferDeviceAddressKHR(device, pInfo);
}

VkResult vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
{
    if (ib_vkSetDebugUtilsObjectNameEXT != NULL)
    {
        return ib_vkSetDebugUtilsObjectNameEXT(device, pNameInfo);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void vkCmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo)
{
    if (ib_vkCmdBeginDebugUtilsLabelEXT != NULL)
    {
        ib_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

void vkCmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer)
{
    if (ib_vkCmdEndDebugUtilsLabelEXT != NULL)
    {
        ib_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets2KHR(
    VkCommandBuffer commandBuffer,
    const VkBindDescriptorSetsInfoKHR*          pBindDescriptorSetsInfo)
{
    if (ib_vkCmdBindDescriptorSets2KHR != NULL)
    {
        ib_vkCmdBindDescriptorSets2KHR(commandBuffer, pBindDescriptorSetsInfo);
    }
}

#define ib_getVulkanFunc(instance, funcName) (PFN_ ## funcName) vkGetInstanceProcAddr(instance, #funcName)

void ib_loadVulkanFunctions(VkInstance instance)
{
    ib_vkCreateDebugUtilsMessengerEXT = ib_getVulkanFunc(instance, vkCreateDebugUtilsMessengerEXT);
    ib_vkDestroyDebugUtilsMessengerEXT = ib_getVulkanFunc(instance, vkDestroyDebugUtilsMessengerEXT);
    ib_vkGetPipelineExecutableStatisticsKHR = ib_getVulkanFunc(instance, vkGetPipelineExecutableStatisticsKHR);
    ib_vkGetPipelineExecutableInternalRepresentationsKHR = ib_getVulkanFunc(instance, vkGetPipelineExecutableInternalRepresentationsKHR);
    ib_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = ib_getVulkanFunc(instance, vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR);
    ib_vkGetBufferDeviceAddressKHR = ib_getVulkanFunc(instance, vkGetBufferDeviceAddressKHR);
    ib_vkSetDebugUtilsObjectNameEXT = ib_getVulkanFunc(instance, vkSetDebugUtilsObjectNameEXT);
    ib_vkCmdBeginDebugUtilsLabelEXT = ib_getVulkanFunc(instance, vkCmdBeginDebugUtilsLabelEXT);
    ib_vkCmdEndDebugUtilsLabelEXT = ib_getVulkanFunc(instance, vkCmdEndDebugUtilsLabelEXT);
    ib_vkCmdBindDescriptorSets2KHR = ib_getVulkanFunc(instance, vkCmdBindDescriptorSets2KHR);
}

ib_TimelineSemaphore ib_allocTimelineSemaphore(ib_Core* core, uint64_t initialValue)
{
    ib_TimelineSemaphore semaphore = {
        .LastSignalValue = initialValue
    };

    VkSemaphoreTypeCreateInfo timelineSemaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = initialValue
    };

    ib_vkCheck(vkCreateSemaphore(core->LogicalDevice, &(VkSemaphoreCreateInfo)
                                 {
                                     .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                                     .pNext = &timelineSemaphoreCreateInfo
                                 }, ib_NoVkAllocator, &semaphore.Semaphore));

    return semaphore;
}

void ib_freeTimelineSemaphore(ib_Core* core, ib_TimelineSemaphore* semaphore)
{
    vkDestroySemaphore(core->LogicalDevice, semaphore->Semaphore, ib_NoVkAllocator);
}

void ib_waitTimelineSemaphore(ib_Core* core, ib_TimelineSemaphore* semaphore)
{
    vkWaitSemaphores(core->LogicalDevice, &(VkSemaphoreWaitInfo)
                     {
                         .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
                         .semaphoreCount = 1,
                         .pSemaphores = &semaphore->Semaphore,
                         .pValues = &semaphore->LastSignalValue
                     }, UINT64_MAX);
}

typedef struct StackGpuMemoryPage
{
    iba_PageHeader Header;
    VkBuffer Buffer;
    iba_GpuAllocation PageAlloc;
} StackGpuMemoryPage;

#ifdef IB_DEBUG
VkBool32 ib_vkValidationCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    ib_potentiallyUnused(messageSeverity);
    ib_potentiallyUnused(messageType);
    ib_potentiallyUnused(pUserData);

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        ib_assert(false, pCallbackData->pMessage);
    }
    else
    {
        char const* prefix = "PRINTF";
        if (strstr(pCallbackData->pMessage, prefix) != NULL)
        {
            printf("%s\n", pCallbackData->pMessage);
        }

        char const* assertPrefix = "IB_ASSERT";
        if (strstr(pCallbackData->pMessage, assertPrefix) != NULL)
        {
            ib_assert(false, pCallbackData->pMessage);
        }
    }
    return VK_FALSE;
}
#endif // IB_DEBUG

const char *ib_InstanceExtensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#ifdef IB_DEBUG
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // IB_DEBUG
};
const char *ib_DeviceExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
#ifdef IB_DEBUG
	VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
#endif // IB_DEBUG
};

const char* ib_RaytracingDeviceExtensions[] =
{
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_RAY_QUERY_EXTENSION_NAME,
};
const char *ib_ValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };

uint32_t const ib_MaxUniformBufferCount = 1000;
uint32_t const ib_MaxStorageBufferCount = 1000;
uint32_t const ib_MaxStorageImageCount = 100;
uint32_t const ib_MaxImageSamplerCount = 1000;
uint32_t const ib_MaxDescriptorSetCount = 1000;
uint32_t const ib_MaxSampledImageCount = 1000;
uint32_t const ib_MaxAccelerationStructureCount = 100;

#ifdef IB_DEBUG
static VkDebugUtilsMessengerEXT ib_VkDebugUtilsMessenger;
#endif // IB_DEBUGs

// Forward from surface API
static VkSurfaceKHR ib_createWin32VkSurface(VkInstance vkInstance, void const* windowHandle, void const* instanceHandle);
void ib_initCore(ib_CoreDesc desc, ib_Core* outCore)
{
    *outCore = (ib_Core) { 0 };
    // Instance
    {
        VkApplicationInfo appInfo =
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Iceberg",
            .applicationVersion = 1,
            .pEngineName = "Iceberg",
            .engineVersion = 1,
            .apiVersion = VK_MAKE_VERSION(1, 3, VK_HEADER_VERSION)
        };

        bool printfValidationEnabled = false;
#ifdef IB_DEBUG
        printfValidationEnabled = true;
#endif // IB_DEBUG
        VkValidationFeaturesEXT validationFeatures =
        {
            .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
            .enabledValidationFeatureCount = printfValidationEnabled ? 1 : 0,
            .pEnabledValidationFeatures = (VkValidationFeatureEnableEXT[]) { VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT }
        };
 
        VkInstanceCreateInfo createInfo =
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = &validationFeatures,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = ib_arrayCount(ib_InstanceExtensions),
            .ppEnabledExtensionNames = ib_InstanceExtensions,
        };

#ifdef IB_DEBUG
        createInfo.enabledLayerCount = ib_arrayCount(ib_ValidationLayers);
        createInfo.ppEnabledLayerNames = ib_ValidationLayers;
#endif // IB_DEBUG

        ib_vkCheck(vkCreateInstance(&createInfo, ib_NoVkAllocator, &outCore->Instance));

        ib_loadVulkanFunctions(outCore->Instance);
    }

#ifdef IB_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &ib_vkValidationCallback,
    };
    vkCreateDebugUtilsMessengerEXT(outCore->Instance, &messengerCreateInfo, ib_NoVkAllocator, &ib_VkDebugUtilsMessenger);
#endif // IB_DEBUG

    // Device
    {
#define maxPhysicalDeviceCount 2
#define maxPhysicalDeviceProperties 64

        uint32_t physicalDeviceCount;
        VkPhysicalDevice physicalDevices[maxPhysicalDeviceCount];

        uint32_t queuePropertyCounts[maxPhysicalDeviceCount];
        VkQueueFamilyProperties queueProperties[maxPhysicalDeviceCount][maxPhysicalDeviceProperties];

        // Enumerate the physical device properties
        {
            ib_vkCheck(vkEnumeratePhysicalDevices(outCore->Instance, &physicalDeviceCount, NULL));
            physicalDeviceCount = physicalDeviceCount <= maxPhysicalDeviceCount ? physicalDeviceCount : maxPhysicalDeviceCount;

            ib_vkCheck(vkEnumeratePhysicalDevices(outCore->Instance, &physicalDeviceCount, physicalDevices));
            for (uint32_t deviceIndex = 0; deviceIndex < physicalDeviceCount; deviceIndex++)
            {
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[deviceIndex], &queuePropertyCounts[deviceIndex], NULL);
                ib_assert(queuePropertyCounts[deviceIndex] > 0 && queuePropertyCounts[deviceIndex] <= maxPhysicalDeviceProperties, "Unexpected queue properties!");

                queuePropertyCounts[deviceIndex] = queuePropertyCounts[deviceIndex] <= maxPhysicalDeviceProperties ? queuePropertyCounts[deviceIndex] : maxPhysicalDeviceProperties;
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[deviceIndex], &queuePropertyCounts[deviceIndex], queueProperties[deviceIndex]);
            }
        }

#undef maxPhysicalDeviceCount
#undef maxPhysicalDeviceProperties

        // Select the device
        {
            uint32_t physicalDeviceIndex = UINT32_MAX;
            for (uint32_t deviceIndex = 0; deviceIndex < physicalDeviceCount; deviceIndex++)
            {
                uint32_t graphicsQueue = UINT32_MAX;
                uint32_t computeQueue = UINT32_MAX;
                uint32_t presentQueue = UINT32_MAX;
                uint32_t transferQueue = UINT32_MAX;

                // Find our graphics queue
                for (uint32_t propIndex = 0; propIndex < queuePropertyCounts[deviceIndex]; propIndex++)
                {
                    if (queueProperties[deviceIndex][propIndex].queueCount == 0)
                    {
                        continue;
                    }

                    if (queueProperties[deviceIndex][propIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    {
                        graphicsQueue = propIndex;
                    }

                    if (queueProperties[deviceIndex][propIndex].queueFlags & VK_QUEUE_COMPUTE_BIT)
                    {
                        computeQueue = propIndex;
                    }

                    if (queueProperties[deviceIndex][propIndex].queueFlags & VK_QUEUE_TRANSFER_BIT)
                    {
                        transferQueue = propIndex;
                    }
                }

                if (desc.Win32MainWindowHandle != NULL)
                {
                    // Create a temporary surface based on our main window to select our device.
                    VkSurfaceKHR const temporarySurface = ib_createWin32VkSurface(outCore->Instance, desc.Win32MainWindowHandle, desc.Win32MainInstanceHandle);

                    // Find our present queue
                    for (uint32_t propIndex = 0; propIndex < queuePropertyCounts[deviceIndex]; propIndex++)
                    {
                        if (queueProperties[deviceIndex][propIndex].queueCount == 0)
                        {
                            continue;
                        }

                        VkBool32 supportsPresent = VK_FALSE;
                        ib_vkCheck(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[deviceIndex], propIndex, temporarySurface, &supportsPresent));

                        if (supportsPresent)
                        {
                            presentQueue = propIndex;
                            break;
                        }
                    }

                    vkDestroySurfaceKHR(outCore->Instance, temporarySurface, ib_NoVkAllocator);
                }
                else
                {
                    // TEMP: If we're not going to present.
                    // Fix our present queue to our graphics queue.
                    // We're not expecting to present from our queue in these circumstances.
                    presentQueue = graphicsQueue;
                }

                if (presentQueue != UINT32_MAX && graphicsQueue != UINT32_MAX && computeQueue != UINT32_MAX && transferQueue != UINT32_MAX)
                {
                    outCore->Queues[ib_Queue_Graphics].Index = graphicsQueue;
                    outCore->Queues[ib_Queue_Compute].Index = computeQueue;
                    outCore->Queues[ib_Queue_Present].Index = presentQueue;
                    outCore->Queues[ib_Queue_Transfer].Index = transferQueue;
                    physicalDeviceIndex = deviceIndex;
                    break;
                }
            }

            ib_assert(physicalDeviceIndex != UINT32_MAX, "Failed to select a physical device!");
            outCore->PhysicalDevice = physicalDevices[physicalDeviceIndex];
        }
    }

    outCore->RaytracingEnabled = false;
    {
        uint32_t propertyCount;
        ib_vkCheck(vkEnumerateDeviceExtensionProperties(outCore->PhysicalDevice, NULL, &propertyCount, NULL));

#define maxPhysicalExtensionCount 1024
        propertyCount = ib_min(maxPhysicalExtensionCount, propertyCount);
        VkExtensionProperties extensions[maxPhysicalExtensionCount];
        ib_vkCheck(vkEnumerateDeviceExtensionProperties(outCore->PhysicalDevice, NULL, &propertyCount, extensions));

        // Use VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME as a proxy for raytracing
        for (uint32_t i = 0; i < propertyCount; i++)
        {
            if (strcmp(extensions[i].extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0)
            {
                outCore->RaytracingEnabled = true;
                break;
            }
        }
#undef maxPhysicalExtensionCount
    }

    // Create the logical device
    {
        VkDeviceQueueCreateInfo queueCreateInfo[ib_Queue_Count] = { 0 };
        uint32_t queueCreateInfoCount = 0;

        uint32_t queueCreatedList = 0;
        for (uint32_t i = 0; i < ib_Queue_Count; i++)
        {
            ib_assert(outCore->Queues[i].Index < 32);
            if (queueCreatedList & (1 << outCore->Queues[i].Index))
            {
                continue;
            }
            queueCreatedList |= (1 << outCore->Queues[i].Index);

            float const queuePriority = 1.0f;
            VkDeviceQueueCreateInfo createQueueInfo =
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueCount = 1,
                .queueFamilyIndex = outCore->Queues[i].Index,
                .pQueuePriorities = &queuePriority
            };

            queueCreateInfo[queueCreateInfoCount++] = createQueueInfo;
        }

        VkPhysicalDeviceFeatures physicalDeviceFeatures =
        {
            .shaderStorageImageWriteWithoutFormat = VK_TRUE,
            .multiDrawIndirect = VK_TRUE,
            .drawIndirectFirstInstance = VK_TRUE,
            .shaderInt64 = VK_TRUE,
            .shaderInt16 = VK_TRUE,
            .dualSrcBlend = VK_TRUE
        };

        // Intel iGPU doesn't support int64 atomics.
        // Use 32 bit visibility buffer instead. (Max of 65536 triangles!)
        //VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT shaderAtomicFeatures =
        //{
        //	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT,
        //	.shaderImageInt64Atomics = VK_TRUE,
        //};

        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
            // .pNext = &shaderAtomicFeatures,
            .accelerationStructure = VK_TRUE
        };

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingPipelineFeatures =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
            .pNext = &accelerationStructureFeatures,
            .rayTracingPipeline = VK_TRUE
        };

        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
            .pNext = &raytracingPipelineFeatures,
            .rayQuery = VK_TRUE
        };

        VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipelinePropertyFeatures =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR,
            .pNext = outCore->RaytracingEnabled ? &rayQueryFeatures : NULL,
#if IB_DEBUG
            .pipelineExecutableInfo = VK_TRUE,
#else
            .pipelineExecutableInfo = VK_FALSE,
#endif // IB_DEBUG
        };

        VkPhysicalDeviceVulkan13Features vulkan13Features =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &pipelinePropertyFeatures,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
            .maintenance4 = VK_TRUE,
            .subgroupSizeControl = VK_TRUE,
            .computeFullSubgroups = VK_TRUE,
            .shaderDemoteToHelperInvocation = VK_TRUE
        };

        VkPhysicalDeviceVulkan12Features vulkan12Features =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &vulkan13Features,
            .shaderFloat16 = VK_TRUE,
            .shaderSharedInt64Atomics = VK_FALSE, // Turning off for now. Messes with Renderdoc :(
            .runtimeDescriptorArray = VK_TRUE,
            .timelineSemaphore = VK_TRUE,
            .shaderSubgroupExtendedTypes = VK_TRUE,
            .bufferDeviceAddress = VK_TRUE,
            .hostQueryReset = VK_TRUE,
            .scalarBlockLayout = VK_TRUE,
            .descriptorBindingPartiallyBound = VK_TRUE
        };

        VkPhysicalDeviceVulkan11Features vulkan11Features =
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &vulkan12Features,
            .uniformAndStorageBuffer16BitAccess = VK_TRUE,
            .storageBuffer16BitAccess = VK_TRUE,
            .shaderDrawParameters = VK_TRUE
        };


        uint32_t extensionCount = ib_arrayCount(ib_DeviceExtensions);
        char const* deviceExtensions[ib_arrayCount(ib_DeviceExtensions) + ib_arrayCount(ib_RaytracingDeviceExtensions)];
        memcpy((void*)deviceExtensions, ib_DeviceExtensions, sizeof(ib_DeviceExtensions));
        if (outCore->RaytracingEnabled)
        {
            memcpy((void*)(deviceExtensions + extensionCount), ib_RaytracingDeviceExtensions, sizeof(ib_RaytracingDeviceExtensions));
            extensionCount += ib_arrayCount(ib_RaytracingDeviceExtensions);
        }

        VkDeviceCreateInfo deviceCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &vulkan11Features,
            .enabledExtensionCount = extensionCount,
            .queueCreateInfoCount = queueCreateInfoCount,
            .pQueueCreateInfos = queueCreateInfo,
            .pEnabledFeatures = &physicalDeviceFeatures,
            .ppEnabledExtensionNames = deviceExtensions,
#ifdef IB_DEBUG
            .enabledLayerCount = ib_arrayCount(ib_ValidationLayers),
            .ppEnabledLayerNames = ib_ValidationLayers
#endif // IB_DEBUG
        };

        ib_vkCheck(vkCreateDevice(outCore->PhysicalDevice, &deviceCreateInfo, ib_NoVkAllocator, &outCore->LogicalDevice));
        for (uint32_t i = 0; i < ib_Queue_Count; i++)
        {
            vkGetDeviceQueue(outCore->LogicalDevice, outCore->Queues[i].Index, 0, &outCore->Queues[i].Queue);
        }

        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(outCore->PhysicalDevice, &properties);
            outCore->DeviceLimits = properties.limits;
        }
    }

    iba_initGpuAllocator(
        (iba_GpuAllocatorDesc) {
        outCore->PhysicalDevice,
        outCore->LogicalDevice,
        1024 * 1024 * 100 // 100MB max allocation
    },
    &outCore->Allocator);

    // Create the descriptor pools
    {
        VkDescriptorPoolSize descriptorPoolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ib_MaxUniformBufferCount },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, ib_MaxStorageBufferCount },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, ib_MaxStorageImageCount },
            { VK_DESCRIPTOR_TYPE_SAMPLER, ib_MaxImageSamplerCount },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, ib_MaxSampledImageCount },
            { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, ib_MaxAccelerationStructureCount }
        };

        uint32_t descriptorPoolCount = ib_arrayCount(descriptorPoolSizes);
        if (!outCore->RaytracingEnabled)
        {
            descriptorPoolCount--; // Remove acceleration structure pool
        }
        VkDescriptorPoolCreateInfo descriptorPoolCreate =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = ib_MaxDescriptorSetCount,
            .poolSizeCount = descriptorPoolCount,
            .pPoolSizes = descriptorPoolSizes
        };

        ib_vkCheck(vkCreateDescriptorPool(outCore->LogicalDevice, &descriptorPoolCreate, ib_NoVkAllocator, &outCore->Descriptors.Pool));
    }

    // Create the pipeline cache
    {
        VkPipelineCacheCreateInfo pipelineCacheCreate = { .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
        ib_vkCheck(vkCreatePipelineCache(outCore->LogicalDevice, &pipelineCacheCreate, ib_NoVkAllocator, &outCore->PipelineCache));
    }

    // Create the command pools
    for (uint32_t i = 0; i < ib_Queue_Count; i++)
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = outCore->Queues[i].Index,
        };

        ib_vkCheck(vkCreateCommandPool(outCore->LogicalDevice, &commandPoolCreateInfo, ib_NoVkAllocator, &outCore->Queues[i].CommandPool));
    }

    // Samplers
    {
        VkSamplerCreateInfo createInfos[] =
        {
            [ib_Sampler_LinearRepeat] =
            {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .maxLod = VK_LOD_CLAMP_NONE
            },
            [ib_Sampler_LinearClamp] =
            {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .maxLod = VK_LOD_CLAMP_NONE
            },
            [ib_Sampler_CompareLess] =
            {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
                .compareEnable = VK_TRUE,
                .compareOp = VK_COMPARE_OP_LESS,
                .maxLod = VK_LOD_CLAMP_NONE
            },
            [ib_Sampler_NearestClamp] =
            {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_NEAREST,
                .minFilter = VK_FILTER_NEAREST,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .maxLod = VK_LOD_CLAMP_NONE
            },
            [ib_Sampler_NearestRepeat] =
            {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = VK_FILTER_NEAREST,
                .minFilter = VK_FILTER_NEAREST,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .maxLod = VK_LOD_CLAMP_NONE
            },
        };

        // I don't want to create a sampler in every new system, just create common ones here
        for (uint32_t i = 0; i < ib_Sampler_Count; i++)
        {
            ib_assert(createInfos[i].sType != 0); // Make sure we initialized all our sampler types
            ib_vkCheck(vkCreateSampler(outCore->LogicalDevice, &createInfos[i], ib_NoVkAllocator, &outCore->Samplers[i]));
        }
    }
}

void ib_killCore(ib_Core* core)
{
    for (uint32_t i = 0; i < ib_Sampler_Count; i++)
    {
        vkDestroySampler(core->LogicalDevice, core->Samplers[i], ib_NoVkAllocator);
    }

    for (uint32_t i = 0; i < ib_Queue_Count; i++)
    {
        vkDestroyCommandPool(core->LogicalDevice, core->Queues[i].CommandPool, ib_NoVkAllocator);
    }

    vkDestroyPipelineCache(core->LogicalDevice, core->PipelineCache, ib_NoVkAllocator);
    vkDestroyDescriptorPool(core->LogicalDevice, core->Descriptors.Pool, ib_NoVkAllocator);

    iba_killGpuAllocator(&core->Allocator);

    vkDestroyDevice(core->LogicalDevice, ib_NoVkAllocator);

#ifdef IB_DEBUG
    vkDestroyDebugUtilsMessengerEXT(core->Instance, ib_VkDebugUtilsMessenger, ib_NoVkAllocator);
#endif // IB_DEBUG

    vkDestroyInstance(core->Instance, ib_NoVkAllocator);
}

// CommandBuffer

void ib_allocCommandBuffers(ib_Core* core, ib_AllocCommandBuffersDesc desc)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = desc.OutCommandBuffers.Count,
        .commandPool = desc.Pool != VK_NULL_HANDLE ? desc.Pool : core->Queues[desc.Queue].CommandPool,
    };

    ib_vkCheck(vkAllocateCommandBuffers(core->LogicalDevice, &commandBufferAllocateInfo, desc.OutCommandBuffers.Data));
}

VkCommandBuffer ib_allocCommandBuffer(ib_Core* core, ib_AllocCommandBufferDesc desc)
{
    VkCommandBuffer commandBuffer;
    ib_allocCommandBuffers(core,
                           (ib_AllocCommandBuffersDesc)
                           {
                               .Queue = desc.Queue,
                               .Pool = desc.Pool,
                               .OutCommandBuffers = ib_singlePtrRange(&commandBuffer)
                           });
    return commandBuffer;
}

void ib_freeCommandBuffer(ib_Core* core, ib_Queue queue, VkCommandBuffer commands)
{
    vkFreeCommandBuffers(core->LogicalDevice, core->Queues[queue].CommandPool, 1, &commands);
}

void ib_freeCommandBuffers(ib_Core* core, ib_Queue queue, uint32_t count, VkCommandBuffer* commands)
{
    vkFreeCommandBuffers(core->LogicalDevice, core->Queues[queue].CommandPool, count, commands);
}

void ib_beginCommandBuffer(ib_Core* core, VkCommandBuffer commandBuffer)
{
    ib_unused(core);
    ib_vkCheck(vkBeginCommandBuffer(commandBuffer,
                                    &(VkCommandBufferBeginInfo)
                                    {
                                        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                                    }));
}

VkCommandBuffer ib_allocAndBeginCommandBuffer(ib_Core* core, ib_Queue queue)
{
    VkCommandBuffer commandBuffer = ib_allocCommandBuffer(core, (ib_AllocCommandBufferDesc) { .Queue = queue });
    ib_vkCheck(vkBeginCommandBuffer(commandBuffer,
                                    &(VkCommandBufferBeginInfo)
                                    {
                                        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                                    }));

    return commandBuffer;
}

void ib_endAndSubmitCommandBuffer(ib_Core* core, VkCommandBuffer commandBuffer, ib_Queue queue)
{
    ib_vkCheck(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };
    ib_vkCheck(vkQueueSubmit(core->Queues[queue].Queue, 1, &submitInfo, VK_NULL_HANDLE));
}

// Texture
ib_Texture ib_allocTexture(ib_Core* core, ib_TextureDesc desc)
{
    ib_Texture texture =
    {
        .Aspect = desc.Aspect,
        .Extent = (VkExtent3D) { desc.Extent.width, desc.Extent.height, ib_max(desc.Extent.depth, 1u) },
        .Format = desc.Format,
        .MipCount = desc.MipCount,
        .LayerCount = desc.LayerCount
    };

    bool is3D = desc.Extent.depth > 1;
    {
        uint32_t queueFamilyCount = 1;
        uint32_t queueFamilies[3] = { core->Queues[ib_Queue_Graphics].Index };
        if (core->Queues[ib_Queue_Compute].Index != core->Queues[ib_Queue_Graphics].Index)
        {
            queueFamilies[queueFamilyCount++] = core->Queues[ib_Queue_Compute].Index;
        }

        if (core->Queues[ib_Queue_Transfer].Index != core->Queues[ib_Queue_Graphics].Index
            && core->Queues[ib_Queue_Transfer].Index != core->Queues[ib_Queue_Compute].Index)
        {
            queueFamilies[queueFamilyCount++] = core->Queues[ib_Queue_Transfer].Index;
        }

        VkImageCreateInfo imageCreate =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = is3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
            .format = desc.Format,
            .extent = { desc.Extent.width, desc.Extent.height, is3D ? desc.Extent.depth : 1 },
            .mipLevels = desc.MipCount > 0 ? desc.MipCount : 1,
            .arrayLayers = desc.LayerCount > 0 ? desc.LayerCount : 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = desc.Usage,
            .initialLayout = desc.InitialLayout,
            .sharingMode = queueFamilyCount > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE, // TODO: Investigate making these exclusive and managing queue ownership.
            .queueFamilyIndexCount = queueFamilyCount,
            .pQueueFamilyIndices = queueFamilies,
        };

        VkImageFormatProperties properties;
        vkGetPhysicalDeviceImageFormatProperties(core->PhysicalDevice, desc.Format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, desc.Usage, 0, &properties);
        ib_potentiallyUnused(properties);

        ib_vkCheck(vkCreateImage(core->LogicalDevice, &imageCreate, ib_NoVkAllocator, &texture.Image));

        if (desc.DebugName != NULL)
        {
            VkDebugUtilsObjectNameInfoEXT textureDebugNameInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .objectType = VK_OBJECT_TYPE_IMAGE,
                .objectHandle = (uint64_t)texture.Image,
                .pObjectName = desc.DebugName
            };

            ib_vkCheck(vkSetDebugUtilsObjectNameEXT(core->LogicalDevice, &textureDebugNameInfo));
        }
    }

    // Allocation
    {
        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(core->LogicalDevice, texture.Image, &memoryRequirements);

        iba_GpuAllocationRequest request =
        {
            .Alignment = memoryRequirements.alignment,
            .Size = memoryRequirements.size,
            .RequiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .PreferredFlags = 0,
            .TypeBits = memoryRequirements.memoryTypeBits,
        };

        iba_GpuAllocation *allocation = &texture.Allocation;
        *allocation = iba_gpuAlloc(&core->Allocator, request);

        ib_vkCheck(vkBindImageMemory(core->LogicalDevice, texture.Image, allocation->Memory, allocation->Offset));
    }

    // Image view
    {
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        if (is3D)
        {
            ib_assert(desc.LayerCount <= 1);
            viewType = VK_IMAGE_VIEW_TYPE_3D;
        }
        else if (desc.LayerCount > 1)
        {
            ib_assert(!is3D);
            viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }

        VkImageViewCreateInfo imageCreateViewInfo =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = texture.Image,
            .viewType = viewType,
            .format = desc.Format,
            .components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
            .subresourceRange =
            {
                .aspectMask = desc.Aspect,
                .levelCount = 1,
                .layerCount = desc.LayerCount > 0 ? desc.LayerCount : 1,
                .baseMipLevel = 0,
            },
        };

        ib_vkCheck(vkCreateImageView(core->LogicalDevice, &imageCreateViewInfo, ib_NoVkAllocator, &texture.View));

        if (desc.DebugName != NULL)
        {
            VkDebugUtilsObjectNameInfoEXT imageViewDebugNameInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
                .objectHandle = (uint64_t)texture.View,
                .pObjectName = desc.DebugName
            };

            ib_vkCheck(vkSetDebugUtilsObjectNameEXT(core->LogicalDevice, &imageViewDebugNameInfo));
        }
    }
    return texture;
}

void ib_freeTexture(ib_Core* core, ib_Texture* texture)
{
    if (texture->Image != VK_NULL_HANDLE)
    {
        vkDestroyImage(core->LogicalDevice, texture->Image, ib_NoVkAllocator);
        vkDestroyImageView(core->LogicalDevice, texture->View, ib_NoVkAllocator);
        iba_gpuFree(&core->Allocator, &texture->Allocation);
    }
}

VkImageMemoryBarrier2 ib_createTextureBarrier(ib_Core* core, ib_TextureBarrierDesc desc)
{
    uint32_t srcQueue = VK_QUEUE_FAMILY_IGNORED;
    uint32_t destQueue = VK_QUEUE_FAMILY_IGNORED;

    if (desc.SourceQueue != desc.DestQueue)
    {
        srcQueue = desc.SourceQueue != ib_Queue_Unknown ? core->Queues[desc.SourceQueue].Index : VK_QUEUE_FAMILY_IGNORED;
        destQueue = desc.DestQueue != ib_Queue_Unknown ? core->Queues[desc.DestQueue].Index : VK_QUEUE_FAMILY_IGNORED;
    }

    return (VkImageMemoryBarrier2)
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .oldLayout = desc.OldLayout,
        .newLayout = desc.NewLayout,
        .srcQueueFamilyIndex = srcQueue,
        .dstQueueFamilyIndex = destQueue,
        .srcStageMask = desc.SourceStageMask,
        .dstStageMask = desc.DestStageMask,
        .image = desc.Texture->Image,
        .subresourceRange =
        {
            .aspectMask = desc.Texture->Aspect,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseMipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        },
        .srcAccessMask = desc.SourceAccessMask,
        .dstAccessMask = desc.DestAccessMask,
    };
}

uint32_t ib_formatToSize(VkFormat format)
{
    static uint32_t const sizes[] =
    {
        // 8bit Size 1
        [VK_FORMAT_R8_UNORM] = 1, [VK_FORMAT_R8_SNORM] = 1, [VK_FORMAT_R8_USCALED] = 1,
        [VK_FORMAT_R8_SSCALED] = 1, [VK_FORMAT_R8_UINT] = 1, [VK_FORMAT_R8_SINT] = 1, [VK_FORMAT_R8_SRGB] = 1,
        
        // 8bit Size 2
        [VK_FORMAT_R8G8_UNORM] = 2, [VK_FORMAT_R8G8_SNORM] = 2, [VK_FORMAT_R8G8_USCALED] = 2,
        [VK_FORMAT_R8G8_SSCALED] = 2, [VK_FORMAT_R8G8_UINT] = 2, [VK_FORMAT_R8G8_SINT] = 2, [VK_FORMAT_R8G8_SRGB] = 2,
        
        // 8bit Size 3
        [VK_FORMAT_R8G8B8_UNORM] = 3, [VK_FORMAT_R8G8B8_SNORM] = 3, [VK_FORMAT_R8G8B8_USCALED] = 3,
        [VK_FORMAT_R8G8B8_SSCALED] = 3, [VK_FORMAT_R8G8B8_UINT] = 3, [VK_FORMAT_R8G8B8_SINT] = 3,
        [VK_FORMAT_R8G8B8_SRGB] = 3, [VK_FORMAT_B8G8R8_UNORM] = 3, [VK_FORMAT_B8G8R8_SNORM] = 3,
        [VK_FORMAT_B8G8R8_USCALED] = 3, [VK_FORMAT_B8G8R8_SSCALED] = 3, [VK_FORMAT_B8G8R8_UINT] = 3,
        [VK_FORMAT_B8G8R8_SINT] = 3, [VK_FORMAT_B8G8R8_SRGB] = 3,

        // 8bit Size 4
        [VK_FORMAT_R8G8B8A8_UNORM] = 4, [VK_FORMAT_R8G8B8A8_SNORM] = 4, [VK_FORMAT_R8G8B8A8_USCALED] = 4,
        [VK_FORMAT_R8G8B8A8_SSCALED] = 4, [VK_FORMAT_R8G8B8A8_UINT] = 4, [VK_FORMAT_R8G8B8A8_SINT] = 4,
        [VK_FORMAT_R8G8B8A8_SRGB] = 4, [VK_FORMAT_B8G8R8A8_UNORM] = 4, [VK_FORMAT_B8G8R8A8_SNORM] = 4,
        [VK_FORMAT_B8G8R8A8_USCALED] = 4, [VK_FORMAT_B8G8R8A8_SSCALED] = 4, [VK_FORMAT_B8G8R8A8_UINT] = 4,
        [VK_FORMAT_B8G8R8A8_SINT] = 4, [VK_FORMAT_B8G8R8A8_SRGB] = 4, [VK_FORMAT_A8B8G8R8_UNORM_PACK32] = 4,
        [VK_FORMAT_A8B8G8R8_SNORM_PACK32] = 4, [VK_FORMAT_A8B8G8R8_USCALED_PACK32] = 4, [VK_FORMAT_A8B8G8R8_SSCALED_PACK32] = 4,
        [VK_FORMAT_A8B8G8R8_UINT_PACK32] = 4, [VK_FORMAT_A8B8G8R8_SINT_PACK32] = 4, [VK_FORMAT_A8B8G8R8_SRGB_PACK32] = 4,
        
        // A2R10G10B10
        [VK_FORMAT_A2R10G10B10_UNORM_PACK32] = 4, [VK_FORMAT_A2R10G10B10_SNORM_PACK32] = 4, [VK_FORMAT_A2R10G10B10_USCALED_PACK32] = 4,
        [VK_FORMAT_A2R10G10B10_SSCALED_PACK32] = 4, [VK_FORMAT_A2R10G10B10_UINT_PACK32] = 4, [VK_FORMAT_A2R10G10B10_SINT_PACK32] = 4,
        [VK_FORMAT_A2B10G10R10_UNORM_PACK32] = 4, [VK_FORMAT_A2B10G10R10_SNORM_PACK32] = 4, [VK_FORMAT_A2B10G10R10_USCALED_PACK32] = 4,
        [VK_FORMAT_A2B10G10R10_SSCALED_PACK32] = 4, [VK_FORMAT_A2B10G10R10_UINT_PACK32] = 4, [VK_FORMAT_A2B10G10R10_SINT_PACK32] = 4,
        
        // R11G11B10
        [VK_FORMAT_B10G11R11_UFLOAT_PACK32] = 4,

        // 16bit, Size 2
        [VK_FORMAT_R16_UNORM] = 2, [VK_FORMAT_R16_SNORM] = 2, [VK_FORMAT_R16_USCALED] = 2,
        [VK_FORMAT_R16_SSCALED] = 2, [VK_FORMAT_R16_UINT] = 2, [VK_FORMAT_R16_SINT] = 2, [VK_FORMAT_R16_SFLOAT] = 2,
        
        // 16bit, Size 4
        [VK_FORMAT_R16G16_UNORM] = 4, [VK_FORMAT_R16G16_SNORM] = 4, [VK_FORMAT_R16G16_USCALED] = 4,
        [VK_FORMAT_R16G16_SSCALED] = 4, [VK_FORMAT_R16G16_UINT] = 4, [VK_FORMAT_R16G16_SINT] = 4, [VK_FORMAT_R16G16_SFLOAT] = 4,
        
        // 16bit, Size 6
        [VK_FORMAT_R16G16B16_UNORM] = 6, [VK_FORMAT_R16G16B16_SNORM] = 6, [VK_FORMAT_R16G16B16_USCALED] = 6,
        [VK_FORMAT_R16G16B16_SSCALED] = 6, [VK_FORMAT_R16G16B16_UINT] = 6, [VK_FORMAT_R16G16B16_SINT] = 6, [VK_FORMAT_R16G16B16_SFLOAT] = 6,
        
        // 16bit, Size 8
        [VK_FORMAT_R16G16B16A16_UNORM] = 8, [VK_FORMAT_R16G16B16A16_SNORM] = 8, [VK_FORMAT_R16G16B16A16_USCALED] = 8,
        [VK_FORMAT_R16G16B16A16_SSCALED] = 8, [VK_FORMAT_R16G16B16A16_UINT] = 8, [VK_FORMAT_R16G16B16A16_SINT] = 8, [VK_FORMAT_R16G16B16A16_SFLOAT] = 8,
        
        // 32 bit
        [VK_FORMAT_R32_UINT] = 4, [VK_FORMAT_R32_SINT] = 4, [VK_FORMAT_R32_SFLOAT] = 4,
        [VK_FORMAT_R32G32_UINT] = 8, [VK_FORMAT_R32G32_SINT] = 8, [VK_FORMAT_R32G32_SFLOAT] = 8,
        [VK_FORMAT_R32G32B32_UINT] = 12, [VK_FORMAT_R32G32B32_SINT] = 12, [VK_FORMAT_R32G32B32_SFLOAT] = 12,
        [VK_FORMAT_R32G32B32A32_UINT] = 16, [VK_FORMAT_R32G32B32A32_SINT] = 16, [VK_FORMAT_R32G32B32A32_SFLOAT] = 16,

        // Depth
        [VK_FORMAT_D16_UNORM] = 2, [VK_FORMAT_D32_SFLOAT] = 4,
    };

    uint32_t size = format < ib_arrayCount(sizes) ? sizes[format] : 0;
    ib_assert(size > 0, "Format is not supported in size API. Add the necessary formats to this function.");

    return size;
}

// Buffer
ib_Buffer ib_allocBuffer(ib_Core* core, ib_BufferDesc desc)
{
    ib_Buffer buffer = { 0 };
    buffer.Size = desc.Size;

    VkBufferUsageFlags const finalUsage = desc.Usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkBufferCreateInfo bufferCreate =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = desc.Size,
        .usage = finalUsage, // buffers created through create buffer can always be transfered to
    };
    ib_vkCheck(vkCreateBuffer(core->LogicalDevice, &bufferCreate, ib_NoVkAllocator, &buffer.VulkanBuffer));

    if (desc.DebugName != NULL)
    {
        VkDebugUtilsObjectNameInfoEXT bufferDebugNameInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = (uint64_t)buffer.VulkanBuffer,
            .pObjectName = desc.DebugName
        };

        ib_vkCheck(vkSetDebugUtilsObjectNameEXT(core->LogicalDevice, &bufferDebugNameInfo));
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(core->LogicalDevice, buffer.VulkanBuffer, &memoryRequirements);

    iba_GpuAllocationRequest request =
    {
        .Alignment = memoryRequirements.alignment,
        .Size = memoryRequirements.size,
        .RequiredFlags = desc.RequiredMemoryFlags,
        .PreferredFlags = desc.PreferredMemoryFlags,
        .TypeBits = memoryRequirements.memoryTypeBits,
        .AllocationType = desc.AllocationType
    };

    buffer.Allocation = iba_gpuAlloc(&core->Allocator, request);
    ib_vkCheck(vkBindBufferMemory(core->LogicalDevice, buffer.VulkanBuffer, buffer.Allocation.Memory, buffer.Allocation.Offset));

    if (finalUsage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        VkBufferDeviceAddressInfo addressQueryInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer.VulkanBuffer,
        };

        buffer.DeviceAddress = vkGetBufferDeviceAddressKHR(core->LogicalDevice, &addressQueryInfo);
    }

    return buffer;
}

void ib_freeBuffer(ib_Core* core, ib_Buffer* buffer)
{
    vkDestroyBuffer(core->LogicalDevice, buffer->VulkanBuffer, ib_NoVkAllocator);
    iba_gpuFree(&core->Allocator, &buffer->Allocation);
}

// Surface
static VkSurfaceKHR ib_createWin32VkSurface(VkInstance vkInstance, void const* windowHandle, void const* instanceHandle)
{
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = (HINSTANCE)instanceHandle,
        .hwnd = (HWND)windowHandle,
    };

    VkSurfaceKHR surface;
    ib_vkCheck(vkCreateWin32SurfaceKHR(vkInstance, &surfaceCreateInfo, ib_NoVkAllocator, &surface));

    return surface;
}

static void ib_buildSwapchain(ib_Core* core, ib_Surface* surface)
{
    VkSwapchainKHR oldSwapchain = surface->Swapchain;

    // Destroy our previous imageview
    for (uint32_t fb = 0; fb < ib_FramebufferCount; fb++)
    {
        vkDestroyImageView(core->LogicalDevice, surface->SwapchainTextures[fb].View, ib_NoVkAllocator);
    }

    VkSwapchainCreateInfoKHR swapchainCreate =
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .minImageCount = ib_FramebufferCount,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .surface = surface->VulkanSurface,
        .imageFormat = surface->Format.format,
        .imageColorSpace = surface->Format.colorSpace,
        .imageExtent = (VkExtent2D) { surface->Extent.width, surface->Extent.height },
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = surface->PresentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = surface->Swapchain,
    };

    if (swapchainCreate.imageFormat != VK_FORMAT_R8G8B8A8_SRGB)
    {
        swapchainCreate.imageUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    uint32_t swapchainShareIndices[2];
    if (core->Queues[ib_Queue_Graphics].Index != core->Queues[ib_Queue_Present].Index)
    {
        swapchainShareIndices[0] = core->Queues[ib_Queue_Graphics].Index;
        swapchainShareIndices[1] = core->Queues[ib_Queue_Present].Index;

        swapchainCreate.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreate.queueFamilyIndexCount = 2;
        swapchainCreate.pQueueFamilyIndices = swapchainShareIndices;
    }
    else
    {
        swapchainCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    ib_vkCheck(vkCreateSwapchainKHR(core->LogicalDevice, &swapchainCreate, ib_NoVkAllocator, &surface->Swapchain));
    vkDestroySwapchainKHR(core->LogicalDevice, oldSwapchain, ib_NoVkAllocator);

    VkImage swapchainImages[ib_FramebufferCount];

    uint32_t swapchainImageCount = ib_FramebufferCount;
    ib_vkCheck(vkGetSwapchainImagesKHR(core->LogicalDevice, surface->Swapchain, &swapchainImageCount, swapchainImages));
    ib_assert(swapchainImageCount >= ib_FramebufferCount, "Too few swapchain images!");

    for (uint32_t fb = 0; fb < ib_FramebufferCount; fb++)
    {
        VkImageViewCreateInfo imageViewCreate =
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .components.r = VK_COMPONENT_SWIZZLE_R,
            .components.g = VK_COMPONENT_SWIZZLE_G,
            .components.b = VK_COMPONENT_SWIZZLE_B,
            .components.a = VK_COMPONENT_SWIZZLE_A,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.levelCount = 1,
            .subresourceRange.layerCount = 1,
            .image = swapchainImages[fb],
            .format = surface->Format.format,
        };

        surface->SwapchainTextures[fb].Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        surface->SwapchainTextures[fb].Image = swapchainImages[fb];
        surface->SwapchainTextures[fb].Extent = (VkExtent3D) { surface->Extent.width, surface->Extent.height };
        surface->SwapchainTextures[fb].Format = surface->Format.format;
        ib_vkCheck(vkCreateImageView(core->LogicalDevice, &imageViewCreate, ib_NoVkAllocator, &surface->SwapchainTextures[fb].View));
    }
}

ib_Surface ib_allocWin32Surface(ib_Core* core, ib_SurfaceDesc desc)
{
    ib_Surface surface = { 0 };

    surface.VulkanSurface = ib_createWin32VkSurface(core->Instance, desc.Win32WindowHandle, desc.Win32InstanceHandle);

    // Extents
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        ib_vkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core->PhysicalDevice, surface.VulkanSurface, &surfaceCapabilities));

        ib_assert(surfaceCapabilities.currentExtent.width != UINT32_MAX, "Surface has invalid width.");
        surface.Extent = (VkExtent3D) { surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height, 0 };
    }

    // Present mode
    surface.PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (!desc.UseVSync)
    {
#define maxPresentModes 32
        uint32_t presentModeCount;
        VkPresentModeKHR presentModes[maxPresentModes];

        ib_vkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(core->PhysicalDevice, surface.VulkanSurface, &presentModeCount, NULL));
        ib_assert(presentModeCount > 0, "Failed to find a present mode.");
        presentModeCount = presentModeCount < maxPresentModes ? presentModeCount : maxPresentModes;
        ib_vkCheck(vkGetPhysicalDeviceSurfacePresentModesKHR(core->PhysicalDevice, surface.VulkanSurface, &presentModeCount, presentModes));
#undef maxPresentModes

        for (uint32_t i = 0; i < presentModeCount; i++)
        {
            if (VK_PRESENT_MODE_MAILBOX_KHR == presentModes[i])
            {
                surface.PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }
    }

    // Surface format
    {
#define maxSurfaceFormatCount 32
        uint32_t surfaceFormatCount;
        VkSurfaceFormatKHR surfaceFormats[maxSurfaceFormatCount];

        ib_vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(core->PhysicalDevice, surface.VulkanSurface, &surfaceFormatCount, NULL));
        ib_assert(surfaceFormatCount > 0, "Failed to find any surface formats.");
        surfaceFormatCount = surfaceFormatCount < maxSurfaceFormatCount ? surfaceFormatCount : maxSurfaceFormatCount;
        ib_vkCheck(vkGetPhysicalDeviceSurfaceFormatsKHR(core->PhysicalDevice, surface.VulkanSurface, &surfaceFormatCount, surfaceFormats));
#undef maxSurfaceFormatCount

        VkFormat targetFormat = desc.SRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;

        if (1 == surfaceFormatCount && VK_FORMAT_UNDEFINED == surfaceFormats[0].format)
        {
            surface.Format.format = targetFormat;
            surface.Format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }
        else
        {
            surface.Format = surfaceFormats[0];
            for (uint32_t i = 0; i < surfaceFormatCount; i++)
            {
                if (targetFormat == surfaceFormats[i].format && VK_COLORSPACE_SRGB_NONLINEAR_KHR == surfaceFormats[i].colorSpace)
                {
                    surface.Format = surfaceFormats[i];
                    break;
                }
            }
        }
    }

    // Swapchain
    ib_buildSwapchain(core, &surface);

    // Acquire semaphore
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        for (uint32_t i = 0; i < ib_FramebufferCount; i++)
        {
            ib_vkCheck(vkCreateSemaphore(core->LogicalDevice, &semaphoreCreateInfo, ib_NoVkAllocator, &surface.Framebuffers[i].AcquireSemaphore));
            ib_vkCheck(vkCreateSemaphore(core->LogicalDevice, &semaphoreCreateInfo, ib_NoVkAllocator, &surface.PresentSemaphores[i]));
        }
    }

    return surface;
}

void ib_freeSurface(ib_Core* core, ib_Surface* surface)
{
    for (uint32_t fb = 0; fb < ib_FramebufferCount; fb++)
    {
        vkDestroySemaphore(core->LogicalDevice, surface->Framebuffers[fb].AcquireSemaphore, ib_NoVkAllocator);
        vkDestroySemaphore(core->LogicalDevice, surface->PresentSemaphores[fb], ib_NoVkAllocator);
        vkDestroyImageView(core->LogicalDevice, surface->SwapchainTextures[fb].View, ib_NoVkAllocator);
    }
    vkDestroySwapchainKHR(core->LogicalDevice, surface->Swapchain, ib_NoVkAllocator);
    vkDestroySurfaceKHR(core->Instance, surface->VulkanSurface, ib_NoVkAllocator);
}

ib_PrepareSurfaceResult ib_prepareSurface(ib_Core* core, ib_PrepareSurfaceDesc prepareDesc)
{
    ib_PrepareSurfaceResult prepareResult = { 0 };

    VkSemaphore acquireSemaphore = prepareDesc.Surface->Framebuffers[prepareDesc.Framebuffer].AcquireSemaphore;

    uint64_t acquireTimeout = 100000000; // 100ms
    VkResult acquireResult = vkAcquireNextImageKHR(core->LogicalDevice, prepareDesc.Surface->Swapchain, acquireTimeout, acquireSemaphore, VK_NULL_HANDLE, &prepareResult.SwapchainTextureIndex);
        
    if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
    {
        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            prepareResult.SurfaceState = ib_SurfaceState_ShouldRebuild;
        }
        else
        {
            prepareResult.SurfaceState = ib_SurfaceState_Error;
        }
    }
        
    return prepareResult;
}

ib_SurfaceState ib_presentSurface(ib_Core* core, ib_PresentSurfaceDesc presentDesc)
{
    VkPresentInfoKHR presentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = presentDesc.WaitSemaphore != VK_NULL_HANDLE ? 1 : 0,
        .pWaitSemaphores = &presentDesc.WaitSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &presentDesc.Surface->Swapchain,
        .pImageIndices = &presentDesc.SwapchainTextureIndex,
    };

    VkResult presentResult = vkQueuePresentKHR(core->Queues[ib_Queue_Present].Queue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
    {
        return ib_SurfaceState_ShouldRebuild;
    }
    else
    {
        return ib_SurfaceState_Ok;
    }
}

void ib_rebuildSurface(ib_Core* core, ib_Surface* surface)
{
    vkDeviceWaitIdle(core->LogicalDevice);
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    ib_vkCheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core->PhysicalDevice, surface->VulkanSurface, &surfaceCapabilities));
    ib_assert(surfaceCapabilities.currentExtent.width != UINT32_MAX, "Surface extents are undefined.");
    
    if (surfaceCapabilities.currentExtent.width > 0 && surfaceCapabilities.currentExtent.height > 0)
    {
        surface->Extent = (VkExtent3D) { surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height };
        ib_buildSwapchain(core, surface);
    }
}

// Graphics pipelines
ib_ShaderInputLayout ib_allocShaderInputLayout(ib_Core* core, ib_ShaderInputLayoutDesc blockLayoutDesc)
{
    ib_ShaderInputLayout blockLayout = { 0 };
    
    ib_assert(blockLayoutDesc.Inputs.Data == NULL || blockLayoutDesc.Inputs.Count > 0);

#define maxShaderInputPerBlock 32
    ib_assert(blockLayoutDesc.Inputs.Count < maxShaderInputPerBlock, "Too many shader inputs! Increase MaxShaderInputPerBlock.");

    VkDescriptorSetLayoutBinding bindings[maxShaderInputPerBlock] = { 0 };
    VkDescriptorBindingFlagsEXT bindFlags[maxShaderInputPerBlock] = { 0 };
#undef maxShaderInputPerBlock
    for (uint32_t i = 0; i < blockLayoutDesc.Inputs.Count; i++)
    {
        ib_ShaderInputDesc inputDesc = blockLayoutDesc.Inputs.Data[i];

        bindings[i].stageFlags = inputDesc.Shaders;
        bindings[i].binding = i;
        bindings[i].descriptorType = inputDesc.Type;

        if (inputDesc.UseImmutableSamplers)
        {
            ib_assert(bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER);
            bindings[i].descriptorCount = ib_Sampler_Count;
            bindings[i].pImmutableSamplers = core->Samplers;
        }
        else
        {
            bindings[i].descriptorCount = inputDesc.ArraySize == 0 ? 1 : inputDesc.ArraySize;
            bindFlags[i] = inputDesc.BindingFlags;
        }
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
        .bindingCount = blockLayoutDesc.Inputs.Count,
        .pBindingFlags = bindFlags
    };

    VkDescriptorSetLayoutCreateInfo createLayout =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &extendedInfo,
        .bindingCount = blockLayoutDesc.Inputs.Count,
        .pBindings = bindings,
    };
    ib_vkCheck(vkCreateDescriptorSetLayout(core->LogicalDevice, &createLayout, ib_NoVkAllocator, &blockLayout.DescriptorSetLayout));
    
    return blockLayout;
}

void ib_freeShaderInputLayout(ib_Core* core, ib_ShaderInputLayout* layout)
{
    vkDestroyDescriptorSetLayout(core->LogicalDevice, layout->DescriptorSetLayout, ib_NoVkAllocator);
}

ib_ShaderInput ib_allocShaderInput(ib_Core* core, ib_AllocShaderInputDesc allocDesc)
{
    ib_ShaderInput block = { 0 };

    ib_assert(allocDesc.Layout != NULL);

    VkDescriptorPool pool = allocDesc.Pool != VK_NULL_HANDLE ? allocDesc.Pool : core->Descriptors.Pool;
    VkDescriptorSetAllocateInfo descriptorSetAlloc =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &allocDesc.Layout->DescriptorSetLayout,
    };
    ib_vkCheck(vkAllocateDescriptorSets(core->LogicalDevice, &descriptorSetAlloc, &block.DescriptorSet));

    ib_assert(allocDesc.Inputs.Data == NULL || allocDesc.Inputs.Count > 0);
    if (allocDesc.Inputs.Count > 0)
    {
        ib_writeToShaderInput(core, (ib_WriteToShaderInputDesc)
                              {
                                  &block,
                                  { allocDesc.Inputs.Data, allocDesc.Inputs.Count }
                              });
    }

    return block;
}

void ib_freeShaderInput(ib_Core* core, ib_ShaderInput* input)
{
    vkFreeDescriptorSets(core->LogicalDevice, core->Descriptors.Pool, 1, &input->DescriptorSet);
}

enum
{
    ib_ShaderInputWriteType_None = 0,
    ib_ShaderInputWriteType_Buffer,
    ib_ShaderInputWriteType_Texture,
    ib_ShaderInputWriteType_Sampler,
    ib_ShaderInputWriteType_AccelerationStructure
};

static uint32_t getShaderInputType(ib_ShaderInputWrite const* input)
{
    uint32_t shaderInputType = ib_ShaderInputWriteType_None;
    if (input->BufferInput.Buffer != NULL)
    {
        shaderInputType = ib_ShaderInputWriteType_Buffer;
    }

    if (input->TextureInput.Texture != NULL)
    {
        ib_assert(shaderInputType == ib_ShaderInputWriteType_None);
        shaderInputType = ib_ShaderInputWriteType_Texture;
    }

    if (input->SamplerInput != VK_NULL_HANDLE)
    {
        ib_assert(shaderInputType == ib_ShaderInputWriteType_None);
        shaderInputType = ib_ShaderInputWriteType_Sampler;
    }

    if (input->AccelerationStructureInput != VK_NULL_HANDLE)
    {
        ib_assert(shaderInputType == ib_ShaderInputWriteType_None);
        shaderInputType = ib_ShaderInputWriteType_AccelerationStructure;
    }

    ib_assert(shaderInputType != ib_ShaderInputWriteType_None);
    return shaderInputType;
}

void ib_writeToShaderInput(ib_Core* core, ib_WriteToShaderInputDesc desc)
{
#define maxWritesPerCall 32
    ib_assert(desc.Inputs.Data == NULL || desc.Inputs.Count > 0);

    ib_assert(desc.Inputs.Count < maxWritesPerCall, "Too many writes. Update this function to be more resilient.");
    VkWriteDescriptorSet writes[maxWritesPerCall] = { 0 };

    uint32_t bufferWriteCount = 0;
    VkDescriptorBufferInfo bufferWrites[maxWritesPerCall] = { 0 };

    uint32_t imageWriteCount = 0;
    VkDescriptorImageInfo imageWrites[maxWritesPerCall];

    uint32_t accelerationStructureWriteCount = 0;
    VkWriteDescriptorSetAccelerationStructureKHR accelerationStructureWrites[maxWritesPerCall] = { 0 };
#undef maxWritesPerCall

    for (uint32_t i = 0; i < desc.Inputs.Count; i++)
    {
        writes[i] = (VkWriteDescriptorSet)
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = desc.ShaderInput->DescriptorSet,
            .descriptorType = desc.Inputs.Data[i].Desc->Type,
            .dstBinding = desc.Inputs.Data[i].BindingIndex,
            .descriptorCount = 1, // TODO: Support multiple array writes
            .dstArrayElement = desc.Inputs.Data[i].ArrayIndex
        };

        uint32_t type = getShaderInputType(&desc.Inputs.Data[i]);
        if (type == ib_ShaderInputWriteType_Buffer)
        {
            VkDeviceSize bufferOffset = desc.Inputs.Data[i].BufferInput.Offset;
            VkDeviceSize bufferRange = desc.Inputs.Data[i].BufferInput.Size != 0 ? desc.Inputs.Data[i].BufferInput.Size : VK_WHOLE_SIZE;

            bufferWrites[bufferWriteCount] = (VkDescriptorBufferInfo)
            {
                .buffer = desc.Inputs.Data[i].BufferInput.Buffer->VulkanBuffer,
                .offset = bufferOffset,
                .range = bufferRange
            };
            writes[i].pBufferInfo = &bufferWrites[bufferWriteCount];
            bufferWriteCount++;
        }
        else if (type == ib_ShaderInputWriteType_Texture)
        {
            imageWrites[imageWriteCount] = (VkDescriptorImageInfo)
            {
                // Allow users to feed a custom image view
                .imageView = desc.Inputs.Data[i].TextureInput.View != VK_NULL_HANDLE ? desc.Inputs.Data[i].TextureInput.View : desc.Inputs.Data[i].TextureInput.Texture->View,
                .imageLayout = desc.Inputs.Data[i].TextureInput.Layout
            };
            writes[i].pImageInfo = &imageWrites[imageWriteCount];
            imageWriteCount++;
        }
        else if (type == ib_ShaderInputWriteType_Sampler)
        {
            ib_assert(!desc.Inputs.Data[i].Desc->UseImmutableSamplers);
            imageWrites[imageWriteCount] = (VkDescriptorImageInfo)
            {
                .sampler = desc.Inputs.Data[i].SamplerInput,
            };
            writes[i].pImageInfo = &imageWrites[imageWriteCount];
            imageWriteCount++;
        }
        else if (type == ib_ShaderInputWriteType_AccelerationStructure)
        {
            accelerationStructureWrites[accelerationStructureWriteCount] = (VkWriteDescriptorSetAccelerationStructureKHR)
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
                .accelerationStructureCount = 1,
                .pAccelerationStructures = &desc.Inputs.Data[i].AccelerationStructureInput,
            };
            writes[i].pNext = &accelerationStructureWrites[accelerationStructureWriteCount];
            accelerationStructureWriteCount++;
        }
        else
        {
            ib_assert(false, "Unsupported shader write type.");
        }
    }

    vkUpdateDescriptorSets(core->LogicalDevice, desc.Inputs.Count, writes, 0, NULL);
}

ib_GraphicsPipeline ib_allocGraphicsPipeline(ib_Core* core, ib_GraphicsPipelineDesc desc)
{
    ib_GraphicsPipeline graphicsPipeline = { 0 };

    // Pipeline layout
    VkDescriptorSetLayout layouts[ib_MaxShaderInputLayoutPerPipeline] = { 0 };
    uint32_t layoutCount = 0;
    for (uint32_t i = 0; i < ib_MaxShaderInputLayoutPerPipeline; i++)
    {
        ib_PipelineShaderInputDesc const* input = &desc.ShaderInputs[i];
        if (input->Inline.Count > 0)
        {
            uint32_t inlineCount = graphicsPipeline.InlineShaderInputLayoutCount++;
            graphicsPipeline.InlineShaderInputLayouts[inlineCount] = ib_allocShaderInputLayout(core, (ib_ShaderInputLayoutDesc) { input->Inline });
            layouts[i] = graphicsPipeline.InlineShaderInputLayouts[inlineCount].DescriptorSetLayout;
        }
        else if (input->External != NULL)
        {
            layouts[i] = input->External->DescriptorSetLayout;
        }
        else
        {
            break; // No more shader inputs.
        }

        layoutCount++;
    }

#ifdef IB_DEBUG
    for (uint32_t i = layoutCount; i < ib_MaxShaderInputLayoutPerPipeline; i++)
    {
        ib_PipelineShaderInputDesc const* input = &desc.ShaderInputs[i];
        ib_assert(input->Inline.Count == 0 && input->External == NULL); // This suggests we had a gap between shader inputs. This is not allowed.
    }
#endif // IB_DEBUG

    ib_assert(desc.PushConstants.Data == NULL || desc.PushConstants.Count > 0);
    VkPipelineLayoutCreateInfo pipelineLayoutCreate =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = layoutCount,
        .pSetLayouts = layouts,
        .pushConstantRangeCount = desc.PushConstants.Count,
        .pPushConstantRanges = desc.PushConstants.Data
    };

    ib_vkCheck(vkCreatePipelineLayout(core->LogicalDevice, &pipelineLayoutCreate, ib_NoVkAllocator, &graphicsPipeline.Layout));

    VkGraphicsPipelineCreateInfo graphicsPipelineCreate =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT
#ifdef IB_DEBUG
            | VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR,
#endif // IB_DEBUG
    };

    // Shaders
#define maxShaderCount 32
    ib_assert(desc.ShaderDescs.Data == NULL || desc.ShaderDescs.Count > 0);

    ib_assert(desc.ShaderDescs.Count < maxShaderCount, "Too many shader blocks! Increase maxShaderCount or refactor.");

    VkShaderModule shaderModules[maxShaderCount] = { 0 };
    VkPipelineShaderStageCreateInfo shaderStages[maxShaderCount] = { 0 };
#undef maxShaderCount

    {
        for (uint32_t i = 0; i < desc.ShaderDescs.Count; i++)
        {
            VkShaderModuleCreateInfo createShader =
            {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pCode = (const uint32_t *)desc.ShaderDescs.Data[i].Code,
                .codeSize = desc.ShaderDescs.Data[i].CodeSize
            };
            ib_vkCheck(vkCreateShaderModule(core->LogicalDevice, &createShader, ib_NoVkAllocator, &shaderModules[i]));

            shaderStages[i] = (VkPipelineShaderStageCreateInfo)
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pName = desc.ShaderDescs.Data[i].EntryPoint,
                .module = shaderModules[i],
                .stage = desc.ShaderDescs.Data[i].Stage
            };
        }

        graphicsPipelineCreate.stageCount = desc.ShaderDescs.Count;
        graphicsPipelineCreate.pStages = shaderStages;
    }
            
    graphicsPipelineCreate.layout = graphicsPipeline.Layout;
    graphicsPipelineCreate.pVertexInputState = &desc.VertexDesc;
    graphicsPipelineCreate.pInputAssemblyState = &desc.InputAssemblyDesc;
    graphicsPipelineCreate.pRasterizationState = &desc.RasterizationDesc;

    // Rendertarget state
#define maxRenderTargetCount 32
    ib_assert(desc.RenderTargetDescs.Data == NULL || desc.RenderTargetDescs.Count > 0);
    ib_assert(desc.RenderTargetDescs.Count < maxRenderTargetCount, "Too many rendertargets! Increase maxRendertargetCount or refactor.");

    VkPipelineColorBlendAttachmentState colorBlendAttachments[maxRenderTargetCount] = { 0 };
    VkFormat colorFormats[maxRenderTargetCount] = { 0 };
#undef maxRenderTargetCount

    for (uint32_t i = 0; i < desc.RenderTargetDescs.Count; i++)
    {
        colorBlendAttachments[i] = desc.RenderTargetDescs.Data[i].BlendDesc;
        colorFormats[i] = desc.RenderTargetDescs.Data[i].Format;
    }
            
    VkPipelineColorBlendStateCreateInfo colorBlendCreate =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = desc.RenderTargetDescs.Count,
        .pAttachments = colorBlendAttachments,
    };
    graphicsPipelineCreate.pColorBlendState = &colorBlendCreate;

    VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = desc.RenderTargetDescs.Count,
        .pColorAttachmentFormats = colorFormats,
        .depthAttachmentFormat = desc.DepthDesc.Format
    };

    graphicsPipelineCreate.pNext = &pipelineRenderingCreateInfo;
    graphicsPipelineCreate.pDepthStencilState = &desc.DepthDesc.DepthState;

    VkPipelineMultisampleStateCreateInfo multisampleCreate =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };
    graphicsPipelineCreate.pMultisampleState = &multisampleCreate;

    // Scissor and viewport are set dynamically
    VkRect2D scissor = { 0 };
    VkViewport viewport = { 0 };
    VkPipelineViewportStateCreateInfo viewportStateCreate =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
    VkDynamicState const dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicStateCreate =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = ib_arrayCount(dynamicStates),
        .pDynamicStates = dynamicStates,
    };

    graphicsPipelineCreate.pDynamicState = &dynamicStateCreate;
    graphicsPipelineCreate.pViewportState = &viewportStateCreate;

    ib_vkCheck(vkCreateGraphicsPipelines(core->LogicalDevice, core->PipelineCache, 1, &graphicsPipelineCreate, ib_NoVkAllocator, &graphicsPipeline.VulkanPipeline));

    for (uint32_t i = 0; i < desc.ShaderDescs.Count; i++)
    {
        vkDestroyShaderModule(core->LogicalDevice, shaderModules[i], ib_NoVkAllocator);
    }

    return graphicsPipeline;
}

void ib_freeGraphicsPipeline(ib_Core* core, ib_GraphicsPipeline* pipeline)
{
    vkDestroyPipeline(core->LogicalDevice, pipeline->VulkanPipeline, ib_NoVkAllocator);
    vkDestroyPipelineLayout(core->LogicalDevice, pipeline->Layout, ib_NoVkAllocator);
    for (uint32_t i = 0; i < ib_MaxShaderInputLayoutPerPipeline; i++)
    {
        ib_freeShaderInputLayout(core, &pipeline->InlineShaderInputLayouts[i]);
    }
    *pipeline = (ib_GraphicsPipeline) { 0 };
}

ib_ComputePipeline ib_allocComputePipeline(ib_Core* core, ib_ComputePipelineDesc desc)
{
    ib_ComputePipeline computePipeline = { 0 };

    // Pipeline layout
    VkDescriptorSetLayout layouts[ib_MaxShaderInputLayoutPerPipeline] = { 0 };
    uint32_t layoutCount = 0;
    for (uint32_t i = 0; i < ib_MaxShaderInputLayoutPerPipeline; i++)
    {
        ib_PipelineShaderInputDesc const* input = &desc.ShaderInputs[i];
        if (input->Inline.Count > 0)
        {
            uint32_t inlineCount = computePipeline.InlineShaderInputLayoutCount++;
            computePipeline.InlineShaderInputLayouts[inlineCount] = ib_allocShaderInputLayout(core, (ib_ShaderInputLayoutDesc) { input->Inline });
            layouts[i] = computePipeline.InlineShaderInputLayouts[inlineCount].DescriptorSetLayout;
        }
        else if (input->External != NULL)
        {
            layouts[i] = input->External->DescriptorSetLayout;
        }
        else
        {
            break; // No more shader inputs.
        }

        layoutCount++;
    }

#ifdef IB_DEBUG
    for (uint32_t i = layoutCount; i < ib_MaxShaderInputLayoutPerPipeline; i++)
    {
        ib_PipelineShaderInputDesc const* input = &desc.ShaderInputs[i];
        ib_assert(input->Inline.Count == 0 && input->External == NULL); // This suggests we had a gap between shader inputs. This is not allowed.
    }
#endif // IB_DEBUG

    ib_assert(desc.PushConstants.Data == NULL || desc.PushConstants.Count > 0);
    VkPipelineLayoutCreateInfo pipelineLayoutCreate =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = layoutCount,
        .pSetLayouts = layouts,
        .pushConstantRangeCount = desc.PushConstants.Count,
        .pPushConstantRanges = desc.PushConstants.Data
    };

    ib_vkCheck(vkCreatePipelineLayout(core->LogicalDevice, &pipelineLayoutCreate, ib_NoVkAllocator, &computePipeline.Layout));

    VkComputePipelineCreateInfo computePipelineCreate =
    {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT
#ifdef IB_DEBUG
            | VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR | VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR,
#endif // IB_DEBUG
    };

    // Shaders
    VkShaderModule shaderModule = { 0 };
    VkPipelineShaderStageCreateInfo shaderStage = { 0 };

    {
        VkShaderModuleCreateInfo createShader =
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pCode = (const uint32_t *)desc.ShaderDesc.Code,
            .codeSize = desc.ShaderDesc.CodeSize
        };
        ib_vkCheck(vkCreateShaderModule(core->LogicalDevice, &createShader, ib_NoVkAllocator, &shaderModule));

        VkPipelineShaderStageRequiredSubgroupSizeCreateInfo requiredWaveSize =
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO,
            .requiredSubgroupSize = desc.ShaderDesc.RequiredWaveSize
        };

        shaderStage = (VkPipelineShaderStageCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = desc.ShaderDesc.RequiredWaveSize > 0 ? &requiredWaveSize : NULL,
            .flags = desc.ShaderDesc.RequiredWaveSize == 0 ? VK_PIPELINE_SHADER_STAGE_CREATE_ALLOW_VARYING_SUBGROUP_SIZE_BIT : 0,
            .pName = desc.ShaderDesc.EntryPoint,
            .module = shaderModule,
            .stage = desc.ShaderDesc.Stage
        };

        computePipelineCreate.stage = shaderStage;
    }
            
    computePipelineCreate.layout = computePipeline.Layout;
    ib_vkCheck(vkCreateComputePipelines(core->LogicalDevice, core->PipelineCache, 1, &computePipelineCreate, ib_NoVkAllocator, &computePipeline.VulkanPipeline));

    vkDestroyShaderModule(core->LogicalDevice, shaderModule, ib_NoVkAllocator);

    return computePipeline;
}

void ib_freeComputePipeline(ib_Core* core, ib_ComputePipeline* pipeline)
{
    vkDestroyPipeline(core->LogicalDevice, pipeline->VulkanPipeline, ib_NoVkAllocator);
    vkDestroyPipelineLayout(core->LogicalDevice, pipeline->Layout, ib_NoVkAllocator);
    for (uint32_t i = 0; i < ib_MaxShaderInputLayoutPerPipeline; i++)
    {
        ib_freeShaderInputLayout(core, &pipeline->InlineShaderInputLayouts[i]);
    }
    *pipeline = (ib_ComputePipeline) { 0 };
}

void ib_bindShaderInputToCompute(VkCommandBuffer commands, ib_ComputePipeline const* pipeline, ib_ShaderInput const* shaderInput)
{
    vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipeline->Layout,
                                0u,
                                1u,
                                &shaderInput->DescriptorSet,
                                0u,
                                NULL);
}

// utility

void ib_printComputePipelineStatistics(ib_Core* core, ib_ComputePipeline const* pipeline)
{
#define maxStatCount 128

    uint32_t statCount = 0;
    VkPipelineExecutableStatisticKHR stats[maxStatCount] = { 0 };
    for (uint32_t i = 0; i < ib_arrayCount(stats); i++)
    {
        stats[i].sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR;
    }

    VkPipelineExecutableInfoKHR executableInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR,
        .pipeline = pipeline->VulkanPipeline,
        .executableIndex = 0
    };

    ib_vkCheck(vkGetPipelineExecutableStatisticsKHR(core->LogicalDevice, &executableInfo, &statCount, NULL));
    statCount = ib_min(statCount, maxStatCount);

    ib_vkCheck(vkGetPipelineExecutableStatisticsKHR(core->LogicalDevice, &executableInfo, &statCount, stats));

    printf("Compute Pipeline\n");
    for (uint32_t i = 0; i < statCount; i++)
    {
        switch (stats[i].format)
        {
            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR:
                printf("    %s: %s\n", stats[i].name, stats[i].value.b32 == VK_TRUE ? "True" : "False");
                break;
            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR:
                printf("    %s: %f\n", stats[i].name, stats[i].value.f64);
                break;
            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR:
                printf("    %s: %" PRId64 "\n", stats[i].name, stats[i].value.i64);
                break;
            case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR:
                printf("    %s: %" PRIu64 "\n", stats[i].name, stats[i].value.u64);
                break;
        }
    }

#undef maxStatCount
}

// Timers
void ib_initTimerManager(ib_TimerManagerDesc desc, ib_TimerManager* outManager)
{
    *outManager = (ib_TimerManager) { 0 };

    outManager->MaxTimestampCount = desc.MaxTimerCount * 2;
    ib_vkCheck(vkCreateQueryPool(desc.Core->LogicalDevice,
                                 &(VkQueryPoolCreateInfo)
                                 {
                                     .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                                     .queryType = VK_QUERY_TYPE_TIMESTAMP,
                                     .queryCount = outManager->MaxTimestampCount,
                                 }, ib_NoVkAllocator, &outManager->TimestampPool));
}

void ib_killTimerManager(ib_Core* core, ib_TimerManager* manager)
{
    vkDestroyQueryPool(core->LogicalDevice, manager->TimestampPool, ib_NoVkAllocator);
}

void ib_resetTimers(ib_TimerManager* manager, VkCommandBuffer commandBuffer)
{
    vkCmdResetQueryPool(commandBuffer, manager->TimestampPool, 0, manager->MaxTimestampCount); // Reset all active timers
    manager->NextTimestamp = 0;
}

void ib_resetTimersCPU(ib_Core* core, ib_TimerManager* manager)
{
    vkResetQueryPool(core->LogicalDevice, manager->TimestampPool, 0, manager->MaxTimestampCount);
    manager->NextTimestamp = 0;
}

ib_Timer ib_beginTimer(ib_TimerManager* manager, VkCommandBuffer commandBuffer)
{
    ib_assert(manager->NextTimestamp < manager->MaxTimestampCount);

    ib_Timer timer = { manager->NextTimestamp };
    manager->NextTimestamp += 2;

    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, manager->TimestampPool, timer.TimestampIndex);

    return timer;
}

void ib_endTimer(ib_TimerManager* manager, VkCommandBuffer commandBuffer, ib_Timer* timer)
{
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, manager->TimestampPool, timer->TimestampIndex + 1);
}

double ib_queryTimer(ib_Core* core, ib_TimerManager* manager, ib_Timer const* timer, bool blocking)
{
    uint32_t flags = VK_QUERY_RESULT_64_BIT;
    if (blocking)
    {
        flags |= VK_QUERY_RESULT_WAIT_BIT;
    }
    uint64_t timestamps[2];
    VkResult result = vkGetQueryPoolResults(core->LogicalDevice, manager->TimestampPool, timer->TimestampIndex, 2, sizeof(uint64_t) * 2, timestamps, sizeof(uint64_t), flags);
	
    if (result == VK_NOT_READY)
    {
        return ib_TimerQueryNotReady;
    }
    else
    {
        double duration = (double)(timestamps[1] - timestamps[0]) * core->DeviceLimits.timestampPeriod;
        duration /= 1000000.0; // nanoseconds to milliseconds

        return duration;
    }
}

// Raytracing

PFN_vkCreateAccelerationStructureKHR ib_vkCreateAccelerationStructureKHR;
PFN_vkDestroyAccelerationStructureKHR ib_vkDestroyAccelerationStructureKHR;
PFN_vkCmdBuildAccelerationStructuresKHR ib_vkCmdBuildAccelerationStructuresKHR;
PFN_vkGetAccelerationStructureDeviceAddressKHR ib_vkGetAccelerationStructureDeviceAddressKHR;
PFN_vkGetAccelerationStructureBuildSizesKHR ib_vkGetAccelerationStructureBuildSizesKHR;

VkResult VKAPI_CALL vkCreateAccelerationStructureKHR(
    VkDevice device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkAccelerationStructureKHR* pAccelerationStructure)
{
    if (ib_vkCreateAccelerationStructureKHR == NULL)
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    return ib_vkCreateAccelerationStructureKHR(
        device,
        pCreateInfo,
        pAllocator,
        pAccelerationStructure);
}

void VKAPI_CALL vkDestroyAccelerationStructureKHR(
    VkDevice device,
    VkAccelerationStructureKHR accelerationStructure,
    const VkAllocationCallbacks* pAllocator)
{
    if (ib_vkDestroyAccelerationStructureKHR == NULL)
    {
        ib_vkCheck(VK_ERROR_EXTENSION_NOT_PRESENT);
        return;
    }

    ib_vkDestroyAccelerationStructureKHR(
        device,
        accelerationStructure,
        pAllocator);
}

void VKAPI_CALL vkCmdBuildAccelerationStructuresKHR(
    VkCommandBuffer commandBuffer,
    uint32_t infoCount,
    const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
    const VkAccelerationStructureBuildRangeInfoKHR * const* ppBuildRangeInfos)
{
    if (ib_vkCmdBuildAccelerationStructuresKHR == NULL)
    {
        ib_vkCheck(VK_ERROR_EXTENSION_NOT_PRESENT);
        return;
    }

    ib_vkCmdBuildAccelerationStructuresKHR(
        commandBuffer,
        infoCount,
        pInfos,
        ppBuildRangeInfos
    );
}

VkDeviceAddress VKAPI_CALL vkGetAccelerationStructureDeviceAddressKHR(
    VkDevice device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo)
{
    if (ib_vkGetAccelerationStructureDeviceAddressKHR == NULL)
    {
        ib_vkCheck(VK_ERROR_EXTENSION_NOT_PRESENT);
        return 0;
    }

    return ib_vkGetAccelerationStructureDeviceAddressKHR(
        device,
        pInfo);
}

void VKAPI_CALL vkGetAccelerationStructureBuildSizesKHR(
    VkDevice device,
    VkAccelerationStructureBuildTypeKHR buildType,
    const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
    const uint32_t* pMaxPrimitiveCounts,
    VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo)
{
    if (ib_vkGetAccelerationStructureBuildSizesKHR == NULL)
    {
        ib_vkCheck(VK_ERROR_EXTENSION_NOT_PRESENT);
        return;
    }

    ib_vkGetAccelerationStructureBuildSizesKHR(
        device,
        buildType,
        pBuildInfo,
        pMaxPrimitiveCounts,
        pSizeInfo);
}

static iba_PageHeader* allocRaytracingStagingMemoryPage(void* userData, size_t pageSize)
{
    StackGpuMemoryPage* page = calloc(1, sizeof(StackGpuMemoryPage));

    ib_Raytracing* raytracing = (ib_Raytracing*)userData;
    VkDevice logicalDevice = raytracing->Core->LogicalDevice;
    VkBufferCreateInfo bufferCreate =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = pageSize,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };
    ib_vkCheck(vkCreateBuffer(logicalDevice, &bufferCreate, ib_NoVkAllocator, &page->Buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, page->Buffer, &memoryRequirements);

    page->PageAlloc = iba_gpuAlloc(&raytracing->Core->Allocator, (iba_GpuAllocationRequest)
                                   {
                                       .Size = memoryRequirements.size,
                                       .Alignment = raytracing->AccelerationStructureScratchBufferAlignment,
                                       .TypeBits = memoryRequirements.memoryTypeBits,
                                       .RequiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   });

    ib_vkCheck(vkBindBufferMemory(logicalDevice, page->Buffer, page->PageAlloc.Memory, page->PageAlloc.Offset));

    return &page->Header;
}

static void freeRaytracingStagingMemoryPage(void* userData, iba_PageHeader* pageHeader)
{
    ib_Raytracing* raytracing = (ib_Raytracing*)userData;
    StackGpuMemoryPage* page = (StackGpuMemoryPage*)pageHeader;

    vkDestroyBuffer(raytracing->Core->LogicalDevice, page->Buffer, ib_NoVkAllocator);
    iba_gpuFree(&raytracing->Core->Allocator, &page->PageAlloc);
    free(page);
}

void ib_initRaytracing(ib_Core* core, ib_Raytracing* raytracing)
{
    raytracing->Core = core;

    ib_vkCreateAccelerationStructureKHR = ib_getVulkanFunc(core->Instance, vkCreateAccelerationStructureKHR);
    ib_vkDestroyAccelerationStructureKHR = ib_getVulkanFunc(core->Instance, vkDestroyAccelerationStructureKHR);
    ib_vkCmdBuildAccelerationStructuresKHR = ib_getVulkanFunc(core->Instance, vkCmdBuildAccelerationStructuresKHR);
    ib_vkGetAccelerationStructureDeviceAddressKHR = ib_getVulkanFunc(core->Instance, vkGetAccelerationStructureDeviceAddressKHR);
    ib_vkGetAccelerationStructureBuildSizesKHR = ib_getVulkanFunc(core->Instance, vkGetAccelerationStructureBuildSizesKHR);

    VkPhysicalDeviceAccelerationStructurePropertiesKHR asProps = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR,
    };
    raytracing->AccelerationStructureScratchBufferAlignment = asProps.minAccelerationStructureScratchOffsetAlignment;
}

void ib_killRaytracing(ib_Raytracing* raytracing)
{
    // Nothing to do here. Function exists for symmetry for now.
    ib_potentiallyUnused(raytracing);
}


void ib_initRaytracingScratch(ib_Raytracing* raytracing, iba_StackAllocator* allocator)
{
    iba_StackAllocatorDesc createInfo = {
        .PageAllocator = {
            .AllocPage = &allocRaytracingStagingMemoryPage,
            .FreePage = &freeRaytracingStagingMemoryPage,
            .UserData = raytracing
        },
        .PageSize = 1024 * 1024 * 256
    };
    iba_initStackAllocator(createInfo, allocator);
}

void ib_killRaytracingScratch(iba_StackAllocator* allocator)
{
    iba_killStackAllocator(allocator);
}

static ib_AccelerationStructureData buildAccelerationStructureInternal(
    ib_Raytracing* raytracing,
    VkAccelerationStructureBuildGeometryInfoKHR* buildInfo,
    uint32_t* primitiveCount,
    iba_StackAllocator* scratchMemoryStack,
    ib_TimelineSemaphore* buildingSemaphore)
{
    ib_AccelerationStructureData out = { 0 };
  
    VkAccelerationStructureBuildSizesInfoKHR sizesInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
    };

    vkGetAccelerationStructureBuildSizesKHR(
        raytracing->Core->LogicalDevice,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        buildInfo,
        primitiveCount,
        &sizesInfo);

    out.Buffer = ib_allocBuffer(raytracing->Core, (ib_BufferDesc)
                                {
                                    .Usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                    .Size = sizesInfo.accelerationStructureSize,
                                    .RequiredMemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                                });

    iba_StackAllocation allocation = iba_stackAlloc(scratchMemoryStack, (iba_StackAllocationRequest) {
        .Alignment = raytracing->AccelerationStructureScratchBufferAlignment,
        .Size = sizesInfo.buildScratchSize,
    });
   
    StackGpuMemoryPage* page = (StackGpuMemoryPage*)allocation.Page;
   
    VkBufferDeviceAddressInfo addressQueryInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = page->Buffer,
    };
    VkDeviceAddress stagingAddress = vkGetBufferDeviceAddressKHR(raytracing->Core->LogicalDevice, &addressQueryInfo) + allocation.Offset;

    VkAccelerationStructureCreateInfoKHR createASInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_GENERIC_KHR,
        .buffer = out.Buffer.VulkanBuffer,
        .size = sizesInfo.accelerationStructureSize,
    };

    ib_vkCheck(vkCreateAccelerationStructureKHR(
        raytracing->Core->LogicalDevice,
        &createASInfo,
        ib_NoVkAllocator,
        &out.AccelerationStructure));
    
    buildInfo->dstAccelerationStructure = out.AccelerationStructure;
    buildInfo->scratchData.deviceAddress = stagingAddress;

    VkAccelerationStructureBuildRangeInfoKHR rangeInfo = (VkAccelerationStructureBuildRangeInfoKHR)
    {
        .firstVertex = 0,
        .primitiveCount = *primitiveCount,
        .primitiveOffset = 0,
        .transformOffset = 0
    };
    VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo = &rangeInfo;

    VkCommandBuffer cmd = ib_allocCommandBuffer(raytracing->Core, (ib_AllocCommandBufferDesc) { .Queue = ib_Queue_Graphics });
    VkCommandBufferBeginInfo beginBufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    ib_vkCheck(vkBeginCommandBuffer(cmd, &beginBufferInfo));

    vkCmdBuildAccelerationStructuresKHR(cmd, 1, buildInfo, &pRangeInfo);

    ib_vkCheck(vkEndCommandBuffer(cmd));

    VkSubmitInfo2 submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &(VkCommandBufferSubmitInfo)
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = cmd
        },
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &(VkSemaphoreSubmitInfo)
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = buildingSemaphore->Semaphore,
            .value = buildingSemaphore->LastSignalValue,
            .stageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
        },
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &(VkSemaphoreSubmitInfo)
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = buildingSemaphore->Semaphore,
            .value = ++buildingSemaphore->LastSignalValue,
            .stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
        },
    };

    ib_vkCheck(vkQueueSubmit2(raytracing->Core->Queues[ib_Queue_Graphics].Queue, 1, &submitInfo, VK_NULL_HANDLE));

    VkAccelerationStructureDeviceAddressInfoKHR ASAddressQueryInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
        .accelerationStructure = out.AccelerationStructure
    };

    out.Address = vkGetAccelerationStructureDeviceAddressKHR(raytracing->Core->LogicalDevice, &ASAddressQueryInfo);
    return out;
}

ib_BLAS ib_allocBLAS(ib_Raytracing* raytracing, ib_BLASDesc desc, iba_StackAllocator* scratchMemoryStack, ib_TimelineSemaphore* buildingSemaphore)
{
    ib_assert(desc.Triangles.Vertices != NULL && desc.Triangles.Indices != NULL);

    VkAccelerationStructureGeometryTrianglesDataKHR trianglesGeometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
        .vertexData.deviceAddress = desc.Triangles.Vertices->DeviceAddress,
        .vertexFormat = desc.Triangles.VertexFormat,
        .vertexStride = desc.Triangles.VertexStride,
        .maxVertex = desc.Triangles.VertexCount - 1,
        .indexData.deviceAddress = desc.Triangles.Indices->DeviceAddress,
        .indexType = desc.Triangles.IndexType
    };

    VkAccelerationStructureGeometryKHR ASGeometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .flags = desc.GeometryFlags,
        .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
        .geometry.triangles = trianglesGeometry
    };

    VkAccelerationStructureBuildGeometryInfoKHR BLASBuildInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .flags = desc.AccelerationStructureFlags,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .geometryCount = 1,
        .pGeometries = &ASGeometry,
    };

    ib_BLAS blas = { 0 };
    blas.Data = buildAccelerationStructureInternal(
        raytracing,
        &BLASBuildInfo,
        &desc.Triangles.TrianglesCount,
        scratchMemoryStack,
        buildingSemaphore);
    return blas;
}

ib_TLAS ib_allocTLAS(ib_Raytracing* raytracing, ib_TLASDesc desc, iba_StackAllocator* scratchMemoryStack, ib_TimelineSemaphore* buildingSemaphore)
{
    ib_assert(desc.InstancesBuffer != NULL);

    VkAccelerationStructureGeometryKHR ASGeometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
        .geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
        .geometry.instances.arrayOfPointers = VK_FALSE,
        .geometry.instances.data.deviceAddress = desc.InstancesBuffer->DeviceAddress
    };

    VkAccelerationStructureBuildGeometryInfoKHR TLASBuildInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
        .flags = desc.AccelerationStructureFlags,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .geometryCount = 1,
        .pGeometries = &ASGeometry
    };

    ib_TLAS tlas = { 0 };
    tlas.Data = buildAccelerationStructureInternal(
        raytracing,
        &TLASBuildInfo,
        &desc.InstancesCount,
        scratchMemoryStack,
        buildingSemaphore);
    return tlas;
}

void ib_freeBLAS(ib_Raytracing* raytracing, ib_BLAS* accelerationStructure)
{
    vkDestroyAccelerationStructureKHR(raytracing->Core->LogicalDevice, accelerationStructure->Data.AccelerationStructure, ib_NoVkAllocator);
    ib_freeBuffer(raytracing->Core, &accelerationStructure->Data.Buffer);
}

void ib_freeTLAS(ib_Raytracing* raytracing, ib_TLAS* accelerationStructure)
{
    vkDestroyAccelerationStructureKHR(raytracing->Core->LogicalDevice, accelerationStructure->Data.AccelerationStructure, ib_NoVkAllocator);
    ib_freeBuffer(raytracing->Core, &accelerationStructure->Data.Buffer);
}

// Utility constants to reduce friction when creating graphics pipelines.
VkPipelineRasterizationStateCreateInfo const ib_RasterizationCullBackFaceCCW =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .lineWidth = 1.0f
};

VkPipelineRasterizationStateCreateInfo const ib_RasterizationNoCull =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    .lineWidth = 1.0f
};

VkPipelineVertexInputStateCreateInfo const ib_VertexInputNull =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
};

VkPipelineInputAssemblyStateCreateInfo const ib_InputAssemblyTriangleList =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
};

VkPipelineInputAssemblyStateCreateInfo const ib_InputAssemblyLineList =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST
};

VkPipelineDepthStencilStateCreateInfo const ib_DepthStateLessTestWriteNoStencil =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_TRUE,
    .depthWriteEnable = VK_TRUE,
    .depthCompareOp = VK_COMPARE_OP_LESS,
    .stencilTestEnable = VK_FALSE,
    .minDepthBounds = 0.0f,
    .maxDepthBounds = 1.0f,
};

VkPipelineDepthStencilStateCreateInfo const ib_DepthStateLessTestNoWriteNoStencil =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_TRUE,
    .depthWriteEnable = VK_FALSE,
    .depthCompareOp = VK_COMPARE_OP_LESS,
    .stencilTestEnable = VK_FALSE,
    .minDepthBounds = 0.0f,
    .maxDepthBounds = 1.0f,
};

VkPipelineDepthStencilStateCreateInfo const ib_DepthStateNoTestNoWrite =
{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
};

VkPipelineColorBlendAttachmentState const ib_BlendDescAlphaBlend =
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

VkPipelineColorBlendAttachmentState const ib_BlendDescNoBlend =
{
    .blendEnable = VK_FALSE,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
};

VkPipelineColorBlendAttachmentState const ib_BlendDescDualSourceBlend =
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

VkPipelineColorBlendAttachmentState const ib_BlendDescAdditiveBlend =
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
