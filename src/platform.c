#include <include/oslib/platform.h>

#include<stdlib.h>

OSLIB_ALLOC Allocate = &malloc;
OSLIB_DEALLOC Deallocate = &free;

void OSLIB_SetAllocator(OSLIB_ALLOC const alloc, OSLIB_DEALLOC const dealloc)
{
	Allocate = alloc;
	Deallocate = dealloc;
}