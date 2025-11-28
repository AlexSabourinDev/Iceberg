// Copyright (c) 2019 Cranberry King; 2025 Snowed In Studios Inc.

#include "ib_util.h"

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
	if(!test)
	{
        printf("[File: %s, Line: %u, Function: %s] Assert! ", file, line, func);

		va_list args;
		va_start (args, test);
		char const* format = va_arg(args, char const*);
		if(format != NULL)
		{
			vprintf(format, args);
		}
        printf("\n");
		va_end (args);

		if(IsDebuggerPresent())
		{
			DebugBreak();
		}
	}
}

char const* ib_IcebergWorkingDirectory = "";

char* ib_extractFilenameFromPath(char const* path)
{
	char* outFilename = NULL;
	char* outFolder = NULL;

	ib_splitPath(path, &outFolder, &outFilename);

	free(outFolder);

	return outFilename;
}

char* ib_concatenatePaths(char const* a, char const* b)
{
#ifdef IB_DEBUG
	char* filename = ib_extractFilenameFromPath(a);
	ib_assert(filename == NULL);
	free(filename);
#endif // IB_DEBUG

	uint32_t const aLength = (uint32_t)strlen(a);
	uint32_t const concatenatedPathLength = aLength + (uint32_t)strlen(b) + 1;

	char* concatenatedPath = malloc(concatenatedPathLength);
	strncpy(concatenatedPath, a, aLength + 1);
	strcat(concatenatedPath, b);

	return concatenatedPath;
}

void ib_splitPath(char const* path, char** outFolderPath, char** outFilename)
{
	uint32_t const pathLength = (uint32_t)strlen(path);
	if (pathLength == 0)
	{
		outFolderPath = NULL;
		outFilename = NULL;
		return;
	}

	char const* lastSlash = strrchr(path, '/');

	ib_assert(pathLength > 0);
	ib_assert(path[0] != '/');

	if (lastSlash == NULL)
	{
		*outFolderPath = NULL;
		*outFilename = malloc(pathLength + 1);
		strcpy(*outFilename, path);
	}
	else
	{
		uint32_t const folderLength = (uint32_t)(lastSlash - path + 1);
		uint32_t const filenameLength = pathLength - folderLength;

		*outFolderPath = malloc(folderLength + 1);
		strncpy(*outFolderPath, path, folderLength);
		(*outFolderPath)[folderLength] = '\0';

		if (filenameLength > 0)
		{
			*outFilename = malloc(filenameLength + 1);
			strncpy(*outFilename, path + folderLength, filenameLength);
			(*outFilename)[filenameLength] = '\0';
		}
		else
		{
			*outFilename = NULL;
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

uint32_t ib_bitCountU32(uint32_t value)
{
	return _mm_popcnt_u32(value);
}