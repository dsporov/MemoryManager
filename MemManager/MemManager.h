#pragma once

#include "definitions.h"

class MemManager {
public:
	MemManager(void *mem, size_t memSize, size_t blockSize);
	virtual ~MemManager() {};

	virtual void *allocate() = 0;
	virtual void free(void* p) = 0;

protected:
	void *mem() const { return mem_; }
	size_t memSize() const { return memSize_; }
	size_t blockSize() const { return blockSize_; }

	static const unsigned char MEM_DEBUG_FREE = 0xfe;
	static const unsigned char MEM_DEBUG_ALLOC = 0xcd;

private:
	void *mem_;
	size_t memSize_, blockSize_;
};
