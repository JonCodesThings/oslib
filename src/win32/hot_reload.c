#define WIN32_LEAN_AND_MEAN
#define NULL 0

#include <Windows.h>

#include <include/oslib/platform.h>

typedef struct OSLIB_HotReloadLibrary
{
	HMODULE library;
	FILETIME lastWrite;
	const char *libraryName;
} OSLIB_HotReloadLibrary;

i32 OSLIB_LoadLibrary(OSLIB_HotReloadLibrary *const lib, const char *libraryName)
{
	lib->library = LoadLibrary(libraryName);
	lib->libraryName = libraryName;

	if (lib->library != NULL)
		return 0;

	return 1;
}

void *OSLIB_GetFunctionPointer(OSLIB_HotReloadLibrary *const lib, const char *functionName)
{
	//TODO: @Jon
	//Make sure that lib.library != NULL
	return GetProcAddress(lib.library, functionName);
}

i32 OSLIB_HotReload(OSLIB_HotReloadLibrary * const lib)
{
	FILETIME prev = lib->lastWrite;

	WIN32_FILE_ATTRIBUTE_DATA Data;
	if (GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
		lib->lastWrite = Data.ftLastWriteTime;

	if (prev == lib->lastWrite)
		return 0;

	UnloadLibrary(lib->library);
	OSLIB_LoadLibrary(lib, lib->libraryName);

	return 0;
}