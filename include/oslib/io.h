#ifndef OSLIB_IO_H
#define OSLIB_IO_H

#include <include/oslib/platform.h>

i32 ReadAllTextFromFile(const char *filepath, char *buffer, i32 bufferSize);

#endif