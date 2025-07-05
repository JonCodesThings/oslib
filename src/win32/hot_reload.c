#define WIN32_LEAN_AND_MEAN
#define NULL 0

#include <windows.h>

#include <include/oslib/platform.h>

#include <include/oslib/io.h>

#include <stdlib.h>
#include <wchar.h>

typedef struct OSLIB_HotReloadLibrary
{
	HMODULE library;
	FILETIME lastWrite;
	LPWSTR libraryFilename;
} OSLIB_HotReloadLibrary;

typedef struct OSLIB_HotReloadFile
{
	FILETIME lastWrite;
	LPWSTR fileName;
	u8 *buffer;
	u32 bufferSize;
} OSLIB_HotReloadFile;

static LPWSTR CopyAllocCharStrToWStr(const char* str)
{
	size_t strLen = strlen(str) + 1;
	LPWSTR wstr = Allocate(sizeof(wchar_t) * strLen);
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wstr, strLen, str, strLen);
	return wstr;
}

i32 OSLIB_GetFileSizeInternal(LPWSTR wFilepath);
i32 OSLIB_ReadBytesFromFileInternal(LPWSTR wFilepath, i8* buffer, i32 bufferSize);

OSLIB_HotReloadLibrary * OSLIB_CreateHotReloadLibrary(const char *libraryFilename)
{
	OSLIB_HotReloadLibrary *alloc = Allocate(sizeof(OSLIB_HotReloadLibrary));

	alloc->libraryFilename = CopyAllocCharStrToWStr(libraryFilename);

	return alloc;
}

OSLIB_HotReloadFile * OSLIB_CreteHotReloadFile(const char *filename, u8 *fileBuffer, u32 fileBufferSize)
{
	OSLIB_HotReloadFile *alloc = Allocate(sizeof(OSLIB_HotReloadFile));

	alloc->fileName = CopyAllocCharStrToWStr(filename);
	alloc->buffer = fileBuffer;
	alloc->bufferSize = fileBufferSize;

	return alloc;
}

void OSLIB_FreeHotReloadLibrary(OSLIB_HotReloadLibrary *const lib)
{
	Deallocate(lib->libraryFilename);
	Deallocate(lib);
}

void OSLIB_FreeHotReloadFile(OSLIB_HotReloadFile *const file)
{
	Deallocate(file->fileName);
	Deallocate(file);
}

i32 OSLIB_LoadLibrary(OSLIB_HotReloadLibrary *const lib)
{
	lib->library = LoadLibraryW(lib->libraryFilename);

	if (lib->library != NULL)
		return 0;

	return 1;
}

i32 OSLIB_LoadFile(OSLIB_HotReloadFile *const file)
{
	i32 fs = OSLIB_GetFileSizeInternal(file->fileName);

	if (fs > (i32)file->bufferSize)
		return fs;
	else if (fs == -1)
		return 0;
	OSLIB_ReadBytesFromFileInternal(file->fileName, file->buffer, file->bufferSize);

	if (fs != file->bufferSize)
		return fs;
	return 0;
}

void OSLIB_FreeLibrary(OSLIB_HotReloadLibrary *const lib)
{
	FreeLibrary(lib->library);
	lib->library = NULL;
}

void *OSLIB_GetFunctionPointer(OSLIB_HotReloadLibrary *const lib, const char *functionName)
{
	if (lib == NULL)
		return NULL;

	if (lib->library == NULL)
		return NULL;

	return GetProcAddress(lib->library, functionName);
}

i32 OSLIB_ReloadLibrary(OSLIB_HotReloadLibrary * const lib)
{
	FILETIME prev = lib->lastWrite;

	WIN32_FILE_ATTRIBUTE_DATA Data;
	if (GetFileAttributesExW(lib->libraryFilename, GetFileExInfoStandard, &Data))
		lib->lastWrite = Data.ftLastWriteTime;

	if (prev.dwHighDateTime == lib->lastWrite.dwHighDateTime && prev.dwLowDateTime == lib->lastWrite.dwLowDateTime)
		return 0;
	
	OSLIB_FreeLibrary(lib);
	OSLIB_LoadLibrary(lib);
	return 0;
}

i32 OSLIB_ReloadFile(OSLIB_HotReloadFile * const file)
{
	FILETIME prev = file->lastWrite;

	WIN32_FILE_ATTRIBUTE_DATA Data;
	if (GetFileAttributesExW(file->fileName, GetFileExInfoStandard, &Data))
		file->lastWrite = Data.ftLastWriteTime;

	if (prev.dwHighDateTime != file->lastWrite.dwHighDateTime || prev.dwLowDateTime != file->lastWrite.dwLowDateTime)
	{
		return OSLIB_LoadFile(file);
	}

	return OSLIB_GetFileSizeInternal(file->fileName);
}