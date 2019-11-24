#include "MemManager.h"

#include <cstring>

MemManager::MemManager(void* mem, size_t memSize, size_t blockSize)
	: mem_(mem)
	, memSize_(memSize)
	, blockSize_(blockSize)
{
	if (nullptr == mem_)
		throw NullPointerException("Memory region is null");

	if (0 == memSize_)
		throw IllegalArgumentException("Memory size is zero");

	if (0 == blockSize_)
		throw IllegalArgumentException("Memory block size is zero");

	if (blockSize_ > memSize_)
		throw IllegalArgumentException("Memory block size greater than memory size");

#ifdef _DEBUG
	std::memset(mem, MEM_DEBUG_FREE, memSize);
#endif
};
