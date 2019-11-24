#pragma once

#include "MemManager.h"

class BlockMemManager : public MemManager {
public:
	BlockMemManager(void* mem, size_t size, size_t blockSize);
	~BlockMemManager();

	virtual void* allocate();
	virtual void free(void *p);

private:
	struct AllocatedBlock;
	struct Block;

	static const int BLOCK_START = 0xdeadbeef;
#ifdef _DEBUG
	static const int BLOCK_END = 0xdead2bad;
#endif

	size_t getBlockSizeWithOverhead() const;

	void checkIntegrity() const;
};

