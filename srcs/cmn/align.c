#include "common.h"

size_t align_down(uint64_t size, uint64_t alignment)
{
	return ((size / alignment) * alignment);
}
size_t align_up(uint64_t size, uint64_t alignment)
{
	return (((size + alignment - 1) / alignment) * alignment);
}
