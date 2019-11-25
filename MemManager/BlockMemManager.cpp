#include <memory>
#include <algorithm>

#include "BlockMemManager.h"

///////////////////////////////////////////////////////////////////////////////////////
// FreeBlock class

struct BlockMemManager::FreeBlock {
private:
	FreeBlock(unsigned numberOfBlocks, unsigned blocksTillNextFreeRegion)
		: numberOfBlocks(numberOfBlocks)
		, nextFreeRegionInBlocks(blocksTillNextFreeRegion)
	{}

public:
	unsigned numberOfBlocks;

	static const unsigned next_null = unsigned(-1);
	unsigned nextFreeRegionInBlocks;

	FreeBlock *nextFreeBlock(void *baseAddr, size_t blockSize) {
		if (--numberOfBlocks > 0) {
			void* addr = reinterpret_cast<char*>(this) + blockSize;
			return new (addr)FreeBlock(numberOfBlocks, nextFreeRegionInBlocks);
		}

		if (next_null == nextFreeRegionInBlocks)
			return nullptr;

		void *nextFreeBlockAddr = reinterpret_cast<char*>(baseAddr) + nextFreeRegionInBlocks * blockSize;
		return reinterpret_cast<FreeBlock*>(nextFreeBlockAddr);
	}

	static FreeBlock *from(void *addr, unsigned numberOfBlocks) {
		return new (addr)FreeBlock(numberOfBlocks, next_null);
	}

	void *getAddr() {
		return this;
	}

	static size_t getSizeOverhead() {
		return sizeof(FreeBlock);
	}
};



///////////////////////////////////////////////////////////////////////////////////////
// AllocatedBlock class

struct BlockMemManager::AllocatedBlock {
private:
	int sentinelStart;

public:
	AllocatedBlock(size_t blockSize)
		: sentinelStart(BLOCK_START)
	{

#ifdef _DEBUG
		int* sentinelEndAddr = reinterpret_cast<int*>(
			reinterpret_cast<char*>(this) + sizeof(sentinelStart) + blockSize
		);
		*sentinelEndAddr = BLOCK_END;

		std::memset(getDataAddr(), MEM_DEBUG_ALLOC, blockSize);
#endif
	}

	void free(size_t blockSize) {
#ifdef _DEBUG
		std::memset(this, MEM_DEBUG_FREE, blockSize + getSizeOverhead());
#else
		sentinelStart = 0;
#endif
	}

	void *getDataAddr() {
		return reinterpret_cast<char*>(this) + sizeof(sentinelStart);
	}

	static AllocatedBlock *from(void* dataAddr) {
		return reinterpret_cast<AllocatedBlock*>(
			static_cast<char*>(dataAddr) - sizeof(sentinelStart)
		);
	}

	void checkIntegrity(size_t blockSize) {
#ifdef _DEBUG
		int* sentinelEndAddr = reinterpret_cast<int*>(
			reinterpret_cast<char*>(this) + sizeof(sentinelStart) + blockSize
		);
		if (*sentinelEndAddr != BLOCK_END)
			throw CorruptedMemoryException("Write operation was performed out of the memory block boundary");
#endif
	}

	static bool isAllocatedBlock(void *p) {
		return *static_cast<int*>(p) == BLOCK_START;
	}

	static size_t getSizeOverhead() {
		return sizeof(AllocatedBlock)
#ifdef _DEBUG
			+ sizeof(BLOCK_END)
#endif
			;
	}
};



///////////////////////////////////////////////////////////////////////////////////////
// BlockMemManager class
BlockMemManager::BlockMemManager(void* mem, size_t size, size_t blockSize)
	: MemManager(mem, size, blockSize)
{
	unsigned numOfBlocks = memSize() / getBlockSizeWithOverhead();
	if (0 == numOfBlocks)
		throw IllegalArgumentException("Memory size is not enough to fit a single block");

	firstFreeBlock_ = FreeBlock::from(mem, numOfBlocks);
}

BlockMemManager::~BlockMemManager() {
	checkIntegrity();
}

void *BlockMemManager::allocate() {
	if (nullptr == firstFreeBlock_)
		throw OutOfMemoryException();

	void *allocAddr = firstFreeBlock_->getAddr();
	firstFreeBlock_ = firstFreeBlock_->nextFreeBlock(mem(), getBlockSizeWithOverhead());

	AllocatedBlock* block = new (allocAddr)AllocatedBlock(blockSize());
	return block->getDataAddr();
}

void BlockMemManager::free(void* p) {
	if (nullptr == p)
		throw NullPointerException();

	AllocatedBlock *block = AllocatedBlock::from(p);
	if (!AllocatedBlock::isAllocatedBlock(block)) {
		throw IllegalArgumentException("Try to free unallocated memory or memory block was corrupted");
	}

	block->checkIntegrity(blockSize());
	static_cast<AllocatedBlock*>(block)->free(blockSize());

	char *blockAddr = reinterpret_cast<char*>(block);

	if (nullptr == firstFreeBlock_) {
		firstFreeBlock_ = FreeBlock::from(blockAddr, 1);
		return;
	}

	// todo: add merging of adjacent free blocks?

	char *nextBlockAddr = reinterpret_cast<char*>(firstFreeBlock_);
	firstFreeBlock_ = FreeBlock::from(blockAddr, 1);
	firstFreeBlock_->nextFreeRegionInBlocks = (nextBlockAddr - static_cast<char*>(mem())) / getBlockSizeWithOverhead();
}

size_t BlockMemManager::getBlockSizeWithOverhead() const {
	return blockSize() + std::max(AllocatedBlock::getSizeOverhead(), FreeBlock:: getSizeOverhead());
}

void BlockMemManager::checkIntegrity() const {
#ifdef _DEBUG
	char* p = static_cast<char*>(mem());
	char* memEnd = p + memSize();

	for (; p < memEnd; p += getBlockSizeWithOverhead()) {
		if (AllocatedBlock::isAllocatedBlock(p)) {
			AllocatedBlock* block = reinterpret_cast<AllocatedBlock*>(p);
			block->checkIntegrity(blockSize());
		}
	}
#endif
}
