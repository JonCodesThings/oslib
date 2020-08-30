#ifndef OSLIB_IO_H
#define OSLIB_IO_H

#include <include/oslib/platform.h>

i32 OSLIB_ReadAllBytesFromFile(const char *filepath, i8 *buffer, i32 bufferSize);

#endif