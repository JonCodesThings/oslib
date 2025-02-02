#include <include/oslib/hot_reload.h>

#if defined(_MSC_VER)
	static const char *libName = "testlib.dll";
#elif defined(__GNUC__)
    static const char *libName = "./libtestlib.so";
#else
    static const char *libName = NULL;
#endif

typedef void(*HelloFunction)();

int main()
{
	OSLIB_HotReloadLibrary *lib = OSLIB_CreateHotReloadLibrary(libName);
	OSLIB_LoadLibrary(lib);
	HelloFunction Hello = OSLIB_GetFunctionPointer(lib, "Hello");
	Hello();
	OSLIB_FreeLibrary(lib);
	OSLIB_FreeHotReloadLibrary(lib);
	return 0;
}