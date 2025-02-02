#ifndef OSLIB_HOT_RELOAD_H
#define OSLIB_HOT_RELOAD_H

#include <include/oslib/platform.h>

typedef struct OSLIB_HotReloadLibrary OSLIB_HotReloadLibrary;
typedef struct OSLIB_HotReloadFile OSLIB_HotReloadFile;

OSLIB_HotReloadLibrary * OSLIB_CreateHotReloadLibrary(const char *libraryFilename);

OSLIB_HotReloadFile * OSLIB_CreteHotReloadFile(const char *filename, u8 *fileBuffer, u32 fileBufferSize);

void OSLIB_FreeHotReloadFile(OSLIB_HotReloadFile *const file);

void OSLIB_FreeHotReloadLibrary(OSLIB_HotReloadLibrary *const lib);

i32 OSLIB_LoadLibrary(OSLIB_HotReloadLibrary * const lib);

i32 OSLIB_LoadFile(OSLIB_HotReloadFile *const file);

void OSLIB_FreeLibrary(OSLIB_HotReloadLibrary *const lib);

void *OSLIB_GetFunctionPointer(OSLIB_HotReloadLibrary *const lib, const char *functionName);

i32 OSLIB_ReloadLibrary(OSLIB_HotReloadLibrary * const lib);

i32 OSLIB_ReloadFile(OSLIB_HotReloadFile * const file);

#endif