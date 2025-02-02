#include <include/oslib/io.h>

#include <stdlib.h>
#include <stdio.h>

int main()
{
	const i32 bufferSize = 32;
	i8 *buffer = malloc(sizeof(i8) * bufferSize);

	if (OSLIB_ReadBytesFromFile("testio.txt", buffer, bufferSize) == -1)
		return 1;

	printf("%s\n", buffer);

	return 0;
}