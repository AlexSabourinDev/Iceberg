// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#include <iceberg/ib_util.h>

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

__declspec(dllimport) int __stdcall IsDebuggerPresent(void);
__declspec(dllimport) void __stdcall DebugBreak(void);
void ib_assertHarness(char const* file, uint32_t line, char const* func, bool test, ...)
{
	if (!test)
	{
		printf("[File: %s, Line: %u, Function: %s] Assert! ", file, line, func);

		va_list args;
		va_start (args, test);
		char const* format = va_arg(args, char const *);
		if (format != NULL)
		{
			vprintf(format, args);
		}
		printf("\n");
		va_end (args);

		if (IsDebuggerPresent())
		{
			DebugBreak();
		}
	}
}

uint32_t ib_firstBitHighU32(uint32_t value)
{
	unsigned long index;
	ib_check(_BitScanReverse(&index, value));
	return index;
}

uint32_t ib_firstBitLowU32(uint32_t value)
{
	unsigned long index;
	ib_check(_BitScanForward(&index, value));
	return index;
}

uint32_t ib_bitCountU64(uint32_t value)
{
	return (uint32_t)_mm_popcnt_u64(value);
}


uint32_t ib_firstBitHighU64(uint64_t value)
{
	unsigned long index;
	ib_check(_BitScanReverse64(&index, value));
	return index;
}

uint32_t ib_firstBitLowU64(uint64_t value)
{
	unsigned long index;
	ib_check(_BitScanForward64(&index, value));
	return index;
}

uint32_t ib_bitCountU32(uint64_t value)
{
	return (uint32_t)_mm_popcnt_u64(value);
}