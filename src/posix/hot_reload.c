#define NULL 0

#include <stdio.h>

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <include/oslib/platform.h>

#include <include/oslib/io.h>

typedef struct OSLIB_HotReloadLibrary
{
    void *library;
    time_t lastWrite;
	const char *libraryFilename;
} OSLIB_HotReloadLibrary;

typedef struct OSLIB_HotReloadFile
{
    time_t lastWrite;
	const char *fileName;
	u8 *buffer;
	u32 bufferSize;
} OSLIB_HotReloadFile;

OSLIB_HotReloadLibrary * OSLIB_CreateHotReloadLibrary(const char *libraryFilename)
{
	OSLIB_HotReloadLibrary *alloc = Allocate(sizeof(OSLIB_HotReloadLibrary));

	alloc->libraryFilename = libraryFilename;

	return alloc;
}

OSLIB_HotReloadFile * OSLIB_CreteHotReloadFile(const char *filename, u8 *fileBuffer, u32 fileBufferSize)
{
	OSLIB_HotReloadFile *alloc = Allocate(sizeof(OSLIB_HotReloadFile));

	alloc->fileName = filename;
	alloc->buffer = fileBuffer;
	alloc->bufferSize = fileBufferSize;

	return alloc;
}

void OSLIB_FreeHotReloadLibrary(OSLIB_HotReloadLibrary *const lib)
{
	Deallocate(lib);
}

void OSLIB_FreeHotReloadFile(OSLIB_HotReloadFile *const file)
{
	Deallocate(file);
}

i32 OSLIB_LoadLibrary(OSLIB_HotReloadLibrary *const lib)
{
    lib->library = dlopen(lib->libraryFilename, RTLD_LAZY);
    if (lib->library != NULL)
        return 0;

	return 1;
}

i32 OSLIB_LoadFile(OSLIB_HotReloadFile *const file)
{
	i32 fs = OSLIB_GetFileSize(file->fileName);

    if (fs > (i32)file->bufferSize)
		return fs;
	else if (fs == -1)
		return 0;

	OSLIB_ReadBytesFromFile(file->fileName, file->buffer, file->bufferSize);

	if (fs != file->bufferSize)
		return fs;
	return 0;
}

void OSLIB_FreeLibrary(OSLIB_HotReloadLibrary *const lib)
{
    dlclose(lib->library);
}

void *OSLIB_GetFunctionPointer(OSLIB_HotReloadLibrary *const lib, const char *functionName)
{
	if (lib == NULL)
		return NULL;

	if (lib->library == NULL)
		return NULL;

    return dlsym(lib->library, functionName);
}

i32 OSLIB_ReloadLibrary(OSLIB_HotReloadLibrary * const lib)
{
    time_t prev = lib->lastWrite;
    struct stat *s = NULL;
    stat(lib->libraryFilename, s);

    if (s->st_mtime != prev)
    {
        lib->lastWrite = s->st_mtime;
        OSLIB_FreeLibrary(lib);
		OSLIB_LoadLibrary(lib);
    }

	return 0;
}

i32 OSLIB_ReloadFile(OSLIB_HotReloadFile * const file)
{
    time_t prev = file->lastWrite;
    struct stat *s = NULL;
    stat(file->fileName, s);

    if (s->st_mtime != prev)
    {
        file->lastWrite = s->st_mtime;
        return OSLIB_LoadFile(file);
    }

	return OSLIB_GetFileSize(file->fileName);
}