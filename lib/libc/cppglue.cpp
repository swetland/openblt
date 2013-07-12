#include <stdlib.h>

void* operator new(size_t size)
{
	return malloc(size);
}

void operator delete(void *ptr)
{
	free(ptr);
}

void* operator new[](size_t size)
{
	return malloc(size);
}

void operator delete[](void *ptr)
{
	free(ptr);
}

extern "C" { 
void __pure_virtual();
}

/*
 *	This needs to do something a little more useful!
 */
void __pure_virtual()
{
	*((char*) 0) = 0;
}
