// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#include "ib_allocator.h"
#include <stdlib.h>
#include <string.h>

// TLSF Allocator

// http://www.gii.upv.es/tlsf/files/papers/jrts2008.pdf
#define tlsf_MinSize (1 << iba_TlsfSecondLevelBitCount)
#define tlsf_32bitMask 0xFFFFFFFF

static iba_TlsfBlock* tlsfAllocBlock(iba_TlsfAllocator* allocator)
{
	ib_potentiallyUnused(allocator);
	// TODO: Use a more robust dynamic allocator for this use case like a block allocator.
	return (iba_TlsfBlock*)calloc(1, sizeof(iba_TlsfBlock));
}

static void tlsfFreeBlock(iba_TlsfAllocator* allocator, iba_TlsfBlock* block)
{
	ib_potentiallyUnused(allocator);
	// TODO: Improve block allocator
	free(block);
}

static void tlsfFindUpperBoundIndices(uint32_t size, uint32_t* firstLevelIndex, uint32_t* secondLevelIndex)
{
	// You can actually implement this function by abusing the floating point instruction sets as well.
	//
	// It generally generates fewer instructions but isn't necessarily any faster (i.e. I haven't profiled it)
	// See: https://godbolt.org/z/sE51zPrhM
	//
	// That implementation would look something like this:
	//
	//if(size >= tlsf_MinSize)
	//{
	//	union
	//	{
	//		float f;
	//		uint32_t u;
	//	} typePun;
	//	typePun.f = (float)size;
	//
	//	uint32_t bitMask = (1 << 18) - 1;
	//	typePun.u = (typePun.u + bitMask);
	//
	//	*firstLevelIndex = (typePun.u >> 23) - 127 - iba_TlsfSecondLevelBitCount + 1;
	//	*secondLevelIndex = (typePun.u >> 18) & 0x1F;
	//}
	//else // Less than min size ("denormals")
	//{
	//	*firstLevelIndex = 0;
	//	*secondLevelIndex = size - 1;
	//}
	
	if(size >= tlsf_MinSize)
	{
		uint32_t highBit = ib_firstBitHighU32(size);

		// Round to next highest size class
		// Lets say for size classes, 32, 36, 40, 44, etc (Increments of 4)
		// If we're at a size class like 33,
		// we want to look at the size class for 36
		// So we bump up by 3 to move 33 into the 36 size class.
		uint32_t sizeClassBump = (1 << (highBit - iba_TlsfSecondLevelBitCount)) - 1;
		size += sizeClassBump;

		// Recalculate first level, we might have moved up.
		highBit = ib_firstBitHighU32(size);
		*firstLevelIndex = highBit - iba_TlsfSecondLevelBitCount + 1;
		*secondLevelIndex = (size >> (highBit - iba_TlsfSecondLevelBitCount)) - iba_TlsfSecondLevelBlockCount;	
	}
	else // Less than min size ("denormals")
	{
		*firstLevelIndex = 0;
		*secondLevelIndex = size - 1;
	}
}

static void tlsfFindLowerBoundIndices(uint32_t size, uint32_t* firstLevelIndex, uint32_t* secondLevelIndex)
{
	if(size >= tlsf_MinSize)
	{
		uint32_t highBit = ib_firstBitHighU32(size);

		*firstLevelIndex = highBit - iba_TlsfSecondLevelBitCount + 1;
		*secondLevelIndex = (size >> (highBit - iba_TlsfSecondLevelBitCount)) - iba_TlsfSecondLevelBlockCount;	
	}
	else // Less than min size ("denormals")
	{
		*firstLevelIndex = 0;
		*secondLevelIndex = size - 1;
	}
}

static void tlsfFreeListPush(iba_TlsfAllocator* allocator, uint32_t firstLevelIndex, uint32_t secondLevelIndex, iba_TlsfBlock* block)
{
	iba_TlsfBlock** freeList = &allocator->FreeLists[firstLevelIndex][secondLevelIndex];
	iba_TlsfBlock* prevHead = *freeList;
	if (prevHead != NULL)
	{
		block->NextFree = prevHead;
		prevHead->PrevFree = block;
	}
	ib_assert(block->PrevFree == NULL);
	*freeList = block;

	allocator->FirstLevelBitMask |= 1 << firstLevelIndex;
	allocator->SecondLevelBitMasks[firstLevelIndex] |= 1 << secondLevelIndex;
}

static iba_TlsfBlock* tlsfFreeListPop(iba_TlsfAllocator* allocator, uint32_t firstLevelIndex, uint32_t secondLevelIndex)
{
	iba_TlsfBlock** freeList = &allocator->FreeLists[firstLevelIndex][secondLevelIndex];

	iba_TlsfBlock* prevHead = *freeList;
	iba_TlsfBlock* nextHead = prevHead->NextFree;
	ib_assert(prevHead != NULL);
	ib_assert(prevHead->PrevFree == NULL);
	if (nextHead != NULL)
	{
		nextHead->PrevFree = NULL;
	}
	*freeList = nextHead;

	if (*freeList == NULL)
	{
		allocator->FirstLevelBitMask &= ~(1 << firstLevelIndex);
		allocator->SecondLevelBitMasks[firstLevelIndex] &= ~(1 << secondLevelIndex);
	}

	prevHead->NextFree = NULL;
	prevHead->PrevFree = NULL;
	return prevHead;
}

static void tlsfRemoveFromFreeList(iba_TlsfAllocator* allocator, iba_TlsfBlock* block)
{
	uint32_t firstLevelIndex;
	uint32_t secondLevelIndex;
	tlsfFindLowerBoundIndices(block->Size, &firstLevelIndex, &secondLevelIndex);

	iba_TlsfBlock** freeList = &allocator->FreeLists[firstLevelIndex][secondLevelIndex];
	if (*freeList == block)
	{
		// If we're the head of our list, just pop it.
		tlsfFreeListPop(allocator, firstLevelIndex, secondLevelIndex);
	}
	else
	{
		iba_TlsfBlock* prev = block->PrevFree;
		ib_assert(prev != NULL); // We're not the head, we should have a previous.
		prev->NextFree = block->NextFree;

		iba_TlsfBlock* next = block->NextFree;
		if(next != NULL)
		{
			next->PrevFree = prev;
		}

		block->NextFree = NULL;
		block->PrevFree = NULL;
	}
}

static void tlsfInsertNeighbourRight(iba_TlsfBlock* left, iba_TlsfBlock* right)
{
	ib_assert(right->RightNeighbour == NULL);
	ib_assert(right->LeftNeighbour == NULL);
	right->RightNeighbour = left->RightNeighbour;
	if(right->RightNeighbour != NULL)
	{
		right->RightNeighbour->LeftNeighbour = right;
	}
	right->LeftNeighbour = left;
	left->RightNeighbour = right;
}

static void tlsfRemoveNeighbour(iba_TlsfBlock* block)
{
	iba_TlsfBlock* left = block->LeftNeighbour;
	iba_TlsfBlock* right = block->RightNeighbour;
	
	if(left != NULL)
	{
		left->RightNeighbour = right;
	}

	if(right != NULL)
	{
		right->LeftNeighbour = left;
	}
}

static iba_TlsfBlock* tlsfMergeWithNeighbours(iba_TlsfAllocator* allocator, iba_TlsfBlock* block)
{
	if (block->LeftNeighbour != NULL && !block->LeftNeighbour->Allocated)
	{
		iba_TlsfBlock* left = block->LeftNeighbour;
		ib_assert(left->RootUserData == block->RootUserData);
		ib_assert(left->Offset < block->Offset);
		tlsfRemoveFromFreeList(allocator, left);
		
		left->Size += block->Size;
		tlsfRemoveNeighbour(block);

		tlsfFreeBlock(allocator, block); // Free the right neighbour
		block = left;
	}
	
	if (block->RightNeighbour != NULL && !block->RightNeighbour->Allocated)
	{
		iba_TlsfBlock* right = block->RightNeighbour;
		ib_assert(block->RootUserData == right->RootUserData);
		ib_assert(block->Offset < right->Offset);
		tlsfRemoveFromFreeList(allocator, right);

		block->Size += right->Size;
		tlsfRemoveNeighbour(right);

		tlsfFreeBlock(allocator, right); // Free the right neighbour
	}

	ib_assert(block->LeftNeighbour == NULL || block->LeftNeighbour->Allocated);
	ib_assert(block->RightNeighbour == NULL || block->RightNeighbour->Allocated);
	return block;
}

void tlsfInsert(iba_TlsfAllocator* allocator, iba_TlsfBlock* block)
{
	ib_assert(block->NextFree == NULL);
	ib_assert(block->PrevFree == NULL);

	uint32_t firstLevelIndex;
	uint32_t secondLevelIndex;
	tlsfFindLowerBoundIndices(block->Size, &firstLevelIndex, &secondLevelIndex);

	tlsfFreeListPush(allocator, firstLevelIndex, secondLevelIndex, block);
}

void iba_initTlsfAllocator(iba_TlsfAllocator* allocator)
{
	*allocator = (iba_TlsfAllocator) { 0 };
}

void iba_killTlsfAllocator(iba_TlsfAllocator* allocator)
{
	while(allocator->FirstLevelBitMask != 0)
	{
		uint32_t firstLevelIndex = ib_firstBitLowU32(allocator->FirstLevelBitMask);

		while (allocator->SecondLevelBitMasks[firstLevelIndex] != 0)
		{
			uint32_t secondLevelIndex = ib_firstBitLowU32(allocator->SecondLevelBitMasks[firstLevelIndex]);

			iba_TlsfBlock* block = allocator->FreeLists[firstLevelIndex][secondLevelIndex];

			// Make sure its a root
			// If it isn't, did we forget to free something?
			ib_assert(block->LeftNeighbour == NULL && block->RightNeighbour == NULL);
			tlsfFreeBlock(allocator, block);

			allocator->SecondLevelBitMasks[firstLevelIndex] &= ~(1 << secondLevelIndex);
		}

		allocator->FirstLevelBitMask &= ~(1 << firstLevelIndex);
	}

	*allocator = (iba_TlsfAllocator) { 0 };
}

void iba_tlsfAddRoot(iba_TlsfAllocator* allocator, uintptr_t userData, uint32_t size)
{
	iba_TlsfBlock* rootBlock = tlsfAllocBlock(allocator);
	rootBlock->RootUserData = userData;
	rootBlock->Size = size;
	tlsfInsert(allocator, rootBlock);
}

iba_TlsfAllocation iba_tlsfAlloc(iba_TlsfAllocator* allocator, size_t requestSize, size_t alignment)
{
	ib_assert(requestSize != 0);
	ib_assert(requestSize < UINT32_MAX); // Maximum 4GB allocation.

	if(alignment == 0)
	{
		alignment = 1;
	}
	ib_assert(ib_bitCountU32((uint32_t)alignment) == 1);

	// Over allocate by our minimum alignment in order to align our memory after the fact.
	// There's probably a better way to do this, but this is okay.
	size_t alignedSize = requestSize + alignment - 1;

	// Find free list
	uint32_t firstLevelIndex;
	uint32_t secondLevelIndex;
	tlsfFindUpperBoundIndices((uint32_t)alignedSize, &firstLevelIndex, &secondLevelIndex);

	bool foundFreeList = false;
	{
		ib_assert(secondLevelIndex < 32); // 32 bit masks for second level
		uint32_t secondLevelMask = tlsf_32bitMask << secondLevelIndex;
		uint32_t secondLevelBits = allocator->SecondLevelBitMasks[firstLevelIndex] & secondLevelMask;

		// Is there a free list in our current first level size class?
		if (secondLevelBits != 0)
		{
			secondLevelIndex = ib_firstBitLowU32(secondLevelBits);
			foundFreeList = true;
		}
		else
		{
			uint32_t firstLevelMask = tlsf_32bitMask << (firstLevelIndex + 1);
			uint32_t firstLevelBits = allocator->FirstLevelBitMask & firstLevelMask;
			if (firstLevelBits != 0)
			{
				firstLevelIndex = ib_firstBitLowU32(firstLevelBits);
				secondLevelIndex = ib_firstBitLowU32(allocator->SecondLevelBitMasks[firstLevelIndex]);
				foundFreeList = true;
			}
		}
	}
	
	iba_TlsfAllocation allocation = { 0 };
	if (foundFreeList)
	{
		iba_TlsfBlock* popped = tlsfFreeListPop(allocator, firstLevelIndex, secondLevelIndex);
		popped->Allocated = true;

		// Split our block
		if (popped->Size > alignedSize)
		{
			uint32_t splitBlockSize = (uint32_t)(popped->Size - alignedSize);
			iba_TlsfBlock* newBlock = tlsfAllocBlock(allocator);
			newBlock->RootUserData = popped->RootUserData;
			newBlock->Offset = popped->Offset + (uint32_t)alignedSize;
			newBlock->Size = splitBlockSize;
			popped->Size = (uint32_t)alignedSize;

			tlsfInsertNeighbourRight(popped, newBlock);

			tlsfInsert(allocator, newBlock);
		}

		// Assume alignment is pow2.
		uint32_t offset = popped->Offset;
		uint32_t alignmentMask = (uint32_t)(alignment - 1);
		offset = (offset + alignmentMask) & (~alignmentMask);

		 // Make sure we didn't over align from our bump that we added above.
		ib_assert(offset - popped->Offset <= alignment - 1);
		ib_assert(offset + requestSize <= popped->Offset + popped->Size);

		allocation = (iba_TlsfAllocation) { popped->RootUserData, offset, popped };
	}

	return allocation;
}

void iba_tlsfFree(iba_TlsfAllocator* allocator, iba_TlsfBlock* allocationBlock)
{
	if(allocationBlock == NULL) // No alloc
	{
		return;
	}

	iba_TlsfBlock* block = allocationBlock;
	ib_assert(block->Allocated);
	ib_assert(block->NextFree == NULL);
	ib_assert(block->PrevFree == NULL);
	block->Allocated = false;
	block = tlsfMergeWithNeighbours(allocator, block);
	tlsfInsert(allocator, block);
}

// General GPU Allocator

typedef struct
{
    uint32_t Index;
    VkMemoryPropertyFlags Flags;
} iba_MemoryType;

typedef struct 
{
    uint32_t TypeBits;
    VkMemoryPropertyFlags RequiredFlags;
    VkMemoryPropertyFlags PreferredFlags;
    size_t MaximumAllocationSize;
} iba_MemoryTypeRequest;

static iba_MemoryType iba_findMemoryType(VkPhysicalDevice physicalDevice, iba_MemoryTypeRequest request)
{
    uint32_t preferedMemoryIndex = UINT32_MAX;
    VkPhysicalDeviceMemoryProperties physicalDeviceProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceProperties);

    VkMemoryType *types = physicalDeviceProperties.memoryTypes;
    VkMemoryHeap *heaps = physicalDeviceProperties.memoryHeaps;

    for (uint32_t i = 0; i < physicalDeviceProperties.memoryTypeCount; ++i)
    {
        uint32_t heapIndex = types[i].heapIndex;
        if (   (request.TypeBits & (1 << i))
            && (types[i].propertyFlags & (request.RequiredFlags | request.PreferredFlags)) == (request.RequiredFlags | request.PreferredFlags)
            && (heaps[heapIndex].size >= request.MaximumAllocationSize))
        {
            return (iba_MemoryType){ i, types[i].propertyFlags };
        }
    }

    if (preferedMemoryIndex == UINT32_MAX)
    {
        for (uint32_t i = 0; i < physicalDeviceProperties.memoryTypeCount; ++i)
        {
            uint32_t heapIndex = types[i].heapIndex;
            if (   (request.TypeBits & (1 << i))
                && (types[i].propertyFlags & request.RequiredFlags) == request.RequiredFlags
                && (heaps[heapIndex].size >= request.MaximumAllocationSize))
            {
                return (iba_MemoryType){ i, types[i].propertyFlags };
            }
        }
    }

    return (iba_MemoryType){ UINT32_MAX };
}

void iba_initGpuAllocator(iba_GpuAllocatorDesc desc, iba_GpuAllocator *allocator)
{
	*allocator = (iba_GpuAllocator) { 0 };
    allocator->PhysicalDevice = desc.PhysicalDevice;
    allocator->LogicalDevice = desc.LogicalDevice;
}

void iba_killGpuAllocator(iba_GpuAllocator *allocator)
{
    for (uint32_t i = 0; i < iba_MaxGpuAllocatorPools; i++)
    {
		for (uint32_t r = 0; r < allocator->MemoryPools[i].RootCount; r++)
		{
			vkFreeMemory(allocator->LogicalDevice, allocator->MemoryPools[i].Roots[r].Memory, NULL);
		}
		iba_killTlsfAllocator(&allocator->MemoryPools[i].TlsfAllocator);
    }
}

#define iba_RootMemorySize (1024 * 1024 * 1024)
static uintptr_t toRootUserData(uint32_t poolIndex, uint32_t rootIndex)
{
	return (uintptr_t)poolIndex | ((uintptr_t)rootIndex << 32ull);
}

static uint32_t getPoolIndex(uintptr_t userData)
{
	return (uint32_t)(userData & 0xFFFFFFFF);
}

static uint32_t getRootIndex(uintptr_t userData)
{
	return (uint32_t)(userData >> 32ull);
}

static void addMemoryRoot(iba_GpuAllocator* allocator, uint32_t poolIndex, iba_GpuMemoryPool* pool, iba_MemoryType memoryType)
{
	ib_assert(pool->RootCount < iba_MaxGpuRootAllocations);
	uint32_t rootId = pool->RootCount++;
	iba_GpuMemoryRoot* root = &pool->Roots[rootId];

	VkMemoryAllocateFlagsInfoKHR memoryAllocFlagsInfo =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR
	};

	VkMemoryAllocateInfo memoryAllocInfo =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &memoryAllocFlagsInfo,
		.allocationSize = iba_RootMemorySize,
		.memoryTypeIndex = memoryType.Index
	};

	ib_vkCheck(vkAllocateMemory(allocator->LogicalDevice, &memoryAllocInfo, NULL, &root->Memory));

	if(memoryType.Flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		uint32_t flags = 0;
		ib_vkCheck(vkMapMemory(allocator->LogicalDevice, root->Memory, 0, VK_WHOLE_SIZE, flags, &root->Map));
	}

	iba_tlsfAddRoot(&pool->TlsfAllocator, toRootUserData(poolIndex, rootId), iba_RootMemorySize);
}

iba_GpuAllocation iba_gpuAlloc(iba_GpuAllocator* allocator, iba_GpuAllocationRequest request)
{
    size_t memorySize = request.Size;
    size_t memoryAlignment = request.Alignment;

	ib_assert(memorySize <= iba_RootMemorySize);

    iba_MemoryType const memoryType = iba_findMemoryType(allocator->PhysicalDevice,
        (iba_MemoryTypeRequest)
        {
            .TypeBits = request.TypeBits,
            .RequiredFlags = request.RequiredFlags,
            .PreferredFlags = request.PreferredFlags,
            .MaximumAllocationSize = iba_RootMemorySize
        });
    ib_assert(memoryType.Index != UINT32_MAX, "Invalid memory type index.");

	uint32_t foundPoolIndex = UINT32_MAX;
    for (uint32_t i = 0; i < iba_MaxGpuAllocatorPools; i++)
    {
        if (allocator->MemoryPools[i].MemoryType == memoryType.Index)
        {
			foundPoolIndex = i;
            break;
        }
    }

    if (foundPoolIndex == UINT32_MAX)
    {
        for (uint32_t i = 0; i < iba_MaxGpuAllocatorPools; i++)
        {
            iba_GpuMemoryPool *memoryPool = &allocator->MemoryPools[i];
            if (memoryPool->RootCount == 0)
            {
                foundPoolIndex = i;
				memoryPool->MemoryType = memoryType.Index;
                break;
            }
        }
    }
	ib_assert(foundPoolIndex != UINT32_MAX);

	iba_GpuMemoryPool* foundPool = &allocator->MemoryPools[foundPoolIndex];
	iba_TlsfAllocation tlsfAlloc = iba_tlsfAlloc(&foundPool->TlsfAllocator, memorySize, memoryAlignment);
	if (tlsfAlloc.Block == NULL)
	{
		addMemoryRoot(allocator, foundPoolIndex, foundPool, memoryType);
		tlsfAlloc = iba_tlsfAlloc(&foundPool->TlsfAllocator, memorySize, memoryAlignment);
		ib_assert(tlsfAlloc.Block != NULL); // If its null, abort.
	}

	uint32_t rootIndex = getRootIndex(tlsfAlloc.RootUserData);
	uint8_t* mappedMem = foundPool->Roots[rootIndex].Map;
    iba_GpuAllocation allocation =
    {
		.Memory = foundPool->Roots[rootIndex].Memory,
        .Offset = tlsfAlloc.Offset,
        .AllocId = (uint64_t)tlsfAlloc.Block,
		.CPUMemory = mappedMem != NULL ? mappedMem + tlsfAlloc.Offset : NULL
    };

    return allocation;
}

void iba_gpuFree(iba_GpuAllocator *allocator, iba_GpuAllocation* allocation)
{
    if(allocation->Memory == NULL)
    {
        return;
    }

	iba_TlsfBlock* block = (iba_TlsfBlock*)allocation->AllocId;

	uint32_t poolIndex = getPoolIndex(block->RootUserData);
    iba_GpuMemoryPool *memoryPool = &allocator->MemoryPools[poolIndex];
	iba_tlsfFree(&memoryPool->TlsfAllocator, block);
}

static VkAllocationCallbacks* ibsa_NoVkAllocator = NULL;
void iba_initStackAllocator(iba_StackAllocatorDesc desc, iba_StackAllocator *allocator)
{
    allocator->PageAllocator = desc.PageAllocator;
    allocator->PageSize = desc.PageSize;
    allocator->CurrentPage = NULL;
    allocator->NextAllocInCurrent = 0;

    ib_assert(desc.PageSize > 0);
}

void iba_killStackAllocator(iba_StackAllocator *allocator)
{
    iba_PageHeader* pageIter = allocator->Head;
    while(pageIter != NULL)
    {
        iba_PageHeader* currentPage = pageIter;
        pageIter = currentPage->NextPage;

		allocator->PageAllocator.FreePage(allocator->PageAllocator.UserData, currentPage);
    }
    *allocator = (iba_StackAllocator){ 0 };
}

void iba_stackReset(iba_StackAllocator *allocator)
{
    allocator->CurrentPage = allocator->Head;
    allocator->NextAllocInCurrent = 0;
}

iba_StackAllocation iba_stackAlloc(iba_StackAllocator* allocator, iba_StackAllocationRequest request)
{
    size_t maxAllocationSize = request.Size + request.Alignment;
    ib_assert(maxAllocationSize <= allocator->PageSize);
    if(maxAllocationSize > allocator->PageSize)
    {
        return (iba_StackAllocation){ 0 };
    }

    if(allocator->Head == NULL)
    {
		allocator->Head = allocator->PageAllocator.AllocPage(allocator->PageAllocator.UserData, allocator->PageSize);
        allocator->CurrentPage = allocator->Head;
    }

    // Fit to alignment
    if(request.Alignment != 0 && (allocator->NextAllocInCurrent % request.Alignment) != 0)
    {
        allocator->NextAllocInCurrent += request.Alignment - (allocator->NextAllocInCurrent % request.Alignment);
    }

    size_t availableMemory = allocator->PageSize - allocator->NextAllocInCurrent;
    if(availableMemory < request.Size)
    {
        if(allocator->CurrentPage->NextPage != NULL)
        {
            allocator->CurrentPage = allocator->CurrentPage->NextPage;
        }
        else
        {

            // Push our new page to the back
            {
                iba_PageHeader* previousPage = allocator->CurrentPage;
                allocator->CurrentPage = allocator->PageAllocator.AllocPage(allocator->PageAllocator.UserData, allocator->PageSize);
                previousPage->NextPage = allocator->CurrentPage;
            }
        }

        // Reset our next alloc for our current page
        allocator->NextAllocInCurrent = 0;
    }

    size_t allocOffset = allocator->NextAllocInCurrent;
    allocator->NextAllocInCurrent += request.Size;

    return (iba_StackAllocation)
    {
        .Page = allocator->CurrentPage,
        .Offset = allocOffset
    };
}

void iba_stackFree(iba_StackAllocator* allocator, iba_StackAllocation* allocation)
{
    // Stack allocator does nothing for freeing.
    ib_potentiallyUnused(allocation);
    ib_potentiallyUnused(allocator);
}