#ifndef OSLIB_HOT_RELOAD_H
#define OSLIB_HOT_RELOAD_H

#include <include/oslib/platform.h>

typedef struct OSLIB_HotReloadLibrary OSLIB_HotReloadLibrary;

i32 OSLIB_LoadLibrary(OSLIB_HotReloadLibrary * const lib);

void *OSLIB_GetFunctionPointer(OSLIB_HotReloadLibrary *const lib, const char *functionName);

i32 OSLIB_HotReload(OSLIB_HotReloadLibrary * const lib);

#endif