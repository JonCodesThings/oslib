#include <include/oslib/hot_reload.h>

#include <stdlib.h>
#include <stdio.h>

typedef void(*HelloFunction)();

int main()
{
	const char *libName = "testlib.dll";
	OSLIB_HotReloadLibrary *lib = OSLIB_CreateHotReloadLibrary(libName);

	OSLIB_LoadLibrary(lib);

	HelloFunction Hello = OSLIB_GetFunctionPointer(lib, "Hello");

	Hello();
	OSLIB_FreeLibrary(lib);
	OSLIB_FreeHotReloadLibrary(lib);
	return 0;
}