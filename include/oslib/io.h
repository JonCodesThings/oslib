#ifndef OSLIB_IO_H
#define OSLIB_IO_H

#include <include/oslib/platform.h>

i32 OSLIB_ReadBytesFromFile(const char *filepath, i8 *buffer, i32 bufferSize);

i32 OSLIB_GetDirectoryFileCount(const char *filepath);

i32 OSLIB_GetDirectorySubDirectoryCount(const char *path);

i32 OSLIB_GetDirectoryFileCountWithExtension(const char *filepath, const char *extension);

i32 OSLIB_GetFileSize(const char *filepath);

i32 OSLIB_WriteBytesToFile(const char *filepath, i8 *buffer, i32 bufferSize);

i32 OSLIB_AppendBytesToFile(const char *filepath, i8 *buffer, i32 bufferSize);

const char ** OSLIB_GetFilesWithExtensionInDirectory(const char *filepath, const char *extension);

i32 OSLIB_DeleteFile(const char* filepath);

#endif