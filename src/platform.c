#include <include/oslib/platform.h>

#include<stdlib.h>

OSLIB_ALLOC OSLIB_Allocate = &malloc;
OSLIB_DEALLOC OSLIB_Deallocate = &free;

void OSLIB_SetAllocator(OSLIB_ALLOC const alloc, OSLIB_DEALLOC const dealloc)
{
	OSLIB_Allocate = alloc;
	OSLIB_Deallocate = dealloc;
}