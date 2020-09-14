#include <include/oslib/platform.h>

#include<stdlib.h>

OSLIB_ALLOC Allocate = &malloc;
OSLIB_DEALLOC Deallocate = &free;

void OSLIB_SetAllocationFunction(OSLIB_ALLOC const alloc)
{
	Allocate = alloc;
}

void OSLIB_SetDeallocationFunction(OSLIB_DEALLOC  const dealloc)
{
	Deallocate = dealloc;
}