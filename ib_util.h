// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#ifndef IB_UTIL_H
#define IB_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>

#ifdef IB_ENABLE_TESTS
#include "test.h"
#define ib_assert(...) test_assert(__VA_ARGS__)
#endif // IB_ENABLE_TESTS

#define ib_potentiallyUnused(a) (a)
#define ib_unused(a) (a)
#define ib_stringify(...) #__VA_ARGS__

#ifdef IB_DEBUG

#if !defined(ib_assert)
#include <stdbool.h>
void ib_assertHarness(char const* file, uint32_t line, char const* func, bool test, ...);

#define ib_assert(...) ib_assertHarness(__FILE__, __LINE__, __func__, __VA_ARGS__, 0)
#endif // !defined(ib_assert)

#else // IB_DEBUG
#define ib_assert(test, ...)
#endif // IB_DEBUG

#ifdef IB_DEBUG
#define ib_check(...) ib_assert(__VA_ARGS__)
#else
#define ib_check(test, ...) (test)
#endif // IB_DEBUG

#define ib_vkCheck(test) \
{ \
    VkResult const _vkResult = (test); \
    bool const _result = (VK_SUCCESS == _vkResult); \
    ib_potentiallyUnused(_result); \
    ib_potentiallyUnused(_vkResult); \
    ib_assert(_result, "VkCheck failed with code %d", _vkResult); \
}

#define ib_arrayCount(arr) (sizeof(arr) / sizeof(arr[0])) // Careful! Doesn't work on pointers, will give incorrect results.
#define ib_min(lhs, rhs) (lhs < rhs ? lhs : rhs)
#define ib_max(lhs, rhs) (lhs > rhs ? lhs : rhs)
#define ib_clamp(x, min, max) ib_max(ib_min(x, max), min)

// Fixed Sized Queue (fsq)
#define ib_fsq(ElemType, MaxSize) \
    struct \
    { \
        ElemType Data[MaxSize]; \
        uint32_t Count; \
    }

#define ib_fsqMaxSize(queue) ib_arrayCount((queue).Data)
#define ib_fsqPush(queue) \
        ( (queue)->Data + ib_fsqGrowImpl(&((queue)->Count), ib_arrayCount((queue)->Data)) )
#define ib_fsqPop(queue) ib_fsqShrinkImpl(&((queue)->Count))
#define ib_fsqClear(queue) ((queue)->Count = 0)
#define ib_fsqPeek(queue) ((queue)->Data + ((queue)->Count - 1))

inline void ib_fsqShrinkImpl(uint32_t* count)
{
    if(*count > 0)
    {
        (*count)--;
    }
    else
    {
        ib_assert(false);
    }
}

inline uint32_t ib_fsqGrowImpl(uint32_t* count, uint32_t maxSize)
{
    if(*count < maxSize)
    {
        return (*count)++;
    }
    else
    {
        ib_assert(false);
        return 0; // Stomp on first element. But don't write into other memory.
    }
}

// Range
#define ib_range(Type) \
    struct \
    { \
        Type* Data; \
        uint32_t Count; \
    }

// This only works on fixed sized array. A pointer will give you an incorrect result here.
#define ib_staticArrayRange(...) \
    { \
        (__VA_ARGS__), \
        ib_arrayCount((__VA_ARGS__)) \
    }

// Takes a single element pointer
#define ib_singlePtrRange(...) \
    { \
        __VA_ARGS__, \
        1 \
    }

// Path utils

extern char const* ib_IcebergWorkingDirectory;

char* ib_extractFilenameFromPath(char const* path);
char* ib_concatenatePaths(char const* a, char const* b);
void ib_splitPath(char const* path, char** outFolderPath, char** outFilename);

uint32_t ib_firstBitHighU32(uint32_t value);
uint32_t ib_firstBitLowU32(uint32_t value);
uint32_t ib_bitCountU32(uint32_t value);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // IB_UTIL_H