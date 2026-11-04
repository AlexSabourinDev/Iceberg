// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#ifndef IB_ALLOCATOR_H
#define IB_ALLOCATOR_H

#define VK_PROTOTYPES
#include <vulkan/vulkan.h>

#include <stdint.h>
#include <stdbool.h>

#include "ib_util.h"

// Tlsf Allocator
#define iba_TlsfSizeBitCount 32
#define iba_TlsfSecondLevelBitCount 5
#define iba_TlsfFirstLevelBitCount (iba_TlsfSizeBitCount - iba_TlsfSecondLevelBitCount)
#define iba_TlsfSecondLevelBlockCount (1 << iba_TlsfSecondLevelBitCount)
#define iba_TlsfFreeListBlockCount ((iba_TlsfFirstLevelBitCount + 1) * iba_TlsfSecondLevelBlockCount)

typedef struct iba_TlsfBlock
{
    uintptr_t RootUserData;
    size_t Offset;
    size_t Size;
    bool Allocated;

    // Address neighbour blocks
    struct iba_TlsfBlock* LeftNeighbour;
    struct iba_TlsfBlock* RightNeighbour;

    // Free list Next
    struct iba_TlsfBlock* NextFree;
    struct iba_TlsfBlock* PrevFree;
} iba_TlsfBlock;

typedef struct
{
    size_t Offset;
    iba_TlsfBlock* Block;
} iba_TlsfAllocation;

typedef struct
{
    uint32_t FirstLevelBitMask;
    uint32_t SecondLevelBitMasks[iba_TlsfFirstLevelBitCount + 1]; // Keep a mask for "denormals"

    iba_TlsfBlock** FreeLists; // Keep a list for "denormals"
} iba_TlsfAllocator;

void iba_initTlsfAllocator(iba_TlsfAllocator* allocator);
void iba_killTlsfAllocator(iba_TlsfAllocator* allocator);
void iba_tlsfAddRoot(iba_TlsfAllocator* allocator, uintptr_t allocId, size_t size);
iba_TlsfAllocation iba_tlsfAlloc(iba_TlsfAllocator* allocator, size_t size, size_t alignment);
void iba_tlsfFree(iba_TlsfAllocator* allocator, iba_TlsfBlock* allocationBlock);

// General GPU Allocator

typedef struct iba_GpuMemoryRoot
{
    VkDeviceMemory Memory;
    void *Map;
} iba_GpuMemoryRoot;

#define iba_MaxGpuRootAllocations 16
typedef struct iba_GpuMemoryPool
{
    iba_TlsfAllocator TlsfAllocator;
    iba_GpuMemoryRoot Roots[iba_MaxGpuRootAllocations];
    uint32_t RootCount;
    uint32_t MemoryType;
    struct iba_GpuMemoryPool* Next;
} iba_GpuMemoryPool;

typedef struct
{
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;
    size_t RootMemorySize;
    iba_GpuMemoryPool* MemoryPools;
} iba_GpuAllocator;

typedef enum
{
    iba_GpuAllocationType_Default = 0,
    iba_GpuAllocationType_Device,
} iba_GpuAllocationType;

typedef struct
{
    VkDeviceSize Size;
    VkDeviceSize Alignment;
    uint32_t TypeBits;
    VkMemoryPropertyFlags RequiredFlags;
    VkMemoryPropertyFlags PreferredFlags;
    iba_GpuAllocationType AllocationType;
} iba_GpuAllocationRequest;

// General allocation Id for allocator tracking
typedef uint64_t iba_GpuAllocationId;
static uint64_t const iba_InvalidGpuAllocationId = UINT64_MAX;

typedef struct
{
    VkDeviceMemory Memory;
    VkDeviceSize Offset;
    iba_GpuAllocationId AllocId;
    uint8_t* CPUMemory;
    iba_GpuAllocationType Type;
} iba_GpuAllocation;

typedef struct
{
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;
    size_t MaxAllocationSize;
} iba_GpuAllocatorDesc;

void iba_initGpuAllocator(iba_GpuAllocatorDesc desc, iba_GpuAllocator *allocator);
void iba_killGpuAllocator(iba_GpuAllocator *allocator);

iba_GpuAllocation iba_gpuAlloc(iba_GpuAllocator *allocator, iba_GpuAllocationRequest request);
void iba_gpuFree(iba_GpuAllocator *allocator, iba_GpuAllocation* allocation);

// Stack Allocator

// Pages must match this header
typedef struct iba_PageHeader
{
    struct iba_PageHeader* NextPage;
} iba_PageHeader;

typedef struct
{
    iba_PageHeader* (*AllocPage)(void* userData, size_t pageSize);
    void (*FreePage)(void* userData, iba_PageHeader* page);
    void* UserData;
} iba_PageAllocatorInterface;

typedef struct
{
    iba_PageAllocatorInterface PageAllocator;

    // Alloc Info
    size_t PageSize;

    // Pages
    iba_PageHeader* Head;
    iba_PageHeader* CurrentPage;
    size_t NextAllocInCurrent;
} iba_StackAllocator;

typedef struct
{
    iba_PageAllocatorInterface PageAllocator;
    size_t PageSize;
} iba_StackAllocatorDesc;

void iba_initStackAllocator(iba_StackAllocatorDesc desc, iba_StackAllocator *allocator);
void iba_killStackAllocator(iba_StackAllocator *allocator);
void iba_stackReset(iba_StackAllocator *allocator);

typedef struct
{
    size_t PageSize;
} iba_CpuStackAllocatorDesc;
void iba_initCpuStackAllocator(iba_CpuStackAllocatorDesc desc, iba_StackAllocator *allocator);

typedef struct
{
    size_t Size;
    size_t Alignment;
} iba_StackAllocationRequest;

typedef struct
{
    iba_PageHeader* Page;
    size_t Offset;
} iba_StackAllocation;

iba_StackAllocation iba_stackAlloc(iba_StackAllocator* allocator, iba_StackAllocationRequest request);
void iba_stackFree(iba_StackAllocator* allocator, iba_StackAllocation* allocation);

static void* iba_cpuStackAllocToMemory(iba_StackAllocation stackAlloc)
{
	return ((uint8_t *)stackAlloc.Page) + sizeof(iba_PageHeader) + stackAlloc.Offset;
}

#endif // IB_ALLOCATOR_H