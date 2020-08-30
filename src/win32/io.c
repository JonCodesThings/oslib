#define WIN32_LEAN_AND_MEAN
#define NULL 0

#include <Windows.h>

#include <include/oslib/platform.h>

i32 OSLIB_ReadAllBytesFromFile(const char *filepath, i8 *buffer, i32 bufferSize)
{
	HANDLE file = CreateFileA(filepath, GENERIC_READ, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_READONLY, NULL);

	if (file == NULL)
		return -1;

	DWORD bytes;

	ReadFile(file, buffer, bufferSize, &bytes, NULL);

	CloseHandle(file);

	return bufferSize;
}