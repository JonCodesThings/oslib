#define WIN32_LEAN_AND_MEAN
#define NULL 0

#include <Windows.h>

#include <include/oslib/platform.h>

typedef struct OSLIB_HotReloadLibrary
{
	HMODULE library;
	FILETIME lastWrite;
	const char *libraryFilename;
} OSLIB_HotReloadLibrary;

i32 OSLIB_LoadLibrary(OSLIB_HotReloadLibrary *const lib, const char *libraryFilename)
{
	lib->library = LoadLibrary(libraryFilename);
	lib->libraryFilename = libraryFilename;

	if (lib->library != NULL)
		return 0;

	return 1;
}

void OSLIB_FreeLibrary(OSLIB_HotReloadLibrary *const lib)
{
	FreeLibrary(lib->library);
}

void *OSLIB_GetFunctionPointer(OSLIB_HotReloadLibrary *const lib, const char *functionName)
{
	//TODO: @Jon
	//Make sure that lib.library != NULL
	if (lib == NULL)
		return NULL;

	if (lib->library == NULL)
		return NULL;

	return GetProcAddress(lib->library, functionName);
}

i32 OSLIB_HotReload(OSLIB_HotReloadLibrary * const lib)
{
	FILETIME prev = lib->lastWrite;

	WIN32_FILE_ATTRIBUTE_DATA Data;
	if (GetFileAttributesEx(lib->libraryFilename, GetFileExInfoStandard, &Data))
		lib->lastWrite = Data.ftLastWriteTime;

	if (prev.dwHighDateTime == lib->lastWrite.dwHighDateTime && prev.dwLowDateTime == lib->lastWrite.dwLowDateTime)
	{
		OSLIB_FreeLibrary(lib);
		OSLIB_LoadLibrary(lib, lib->libraryFilename);
		return 0;
	}

	return 0;
}