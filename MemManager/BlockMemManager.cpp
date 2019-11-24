#include <memory>
#include "BlockMemManager.h"

//struct BlockMemManager::Block {
//	Block(size_t size)
//		: size_(size)
//		, flags(0)
//	{}
//
//	size_t size_;
//
//	enum Flags {Allocated = 0x1};
//	int flags;
//
//	bool isFree() const {
//		return ((flags & Flags::Allocated) == 0);
//	}
//};

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
		std::memset(this, MEM_DEBUG_FREE, blockSize + getAllocatedDataSizeOverhead());
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

	static size_t getAllocatedDataSizeOverhead() {
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
	//Block *addr = reinterpret_cast<Block*>(static_cast<char*>(mem));
	//new (addr)Block(size);
	if (getBlockSizeWithOverhead() > memSize())
		throw IllegalArgumentException("Memory size is not enough to fit a single block");
}

BlockMemManager::~BlockMemManager() {
	checkIntegrity();
}

void *BlockMemManager::allocate() {
	char *p = static_cast<char*>(mem());
	char* memEnd = p + memSize();

	for (; p < memEnd; p += getBlockSizeWithOverhead()) {
		if ((memEnd - p) < getBlockSizeWithOverhead())
			throw OutOfMemoryException();

		if (!AllocatedBlock::isAllocatedBlock(p)) {
			AllocatedBlock* addr = reinterpret_cast<AllocatedBlock*>(p);
			AllocatedBlock* block = new (addr)AllocatedBlock(blockSize());
			return block->getDataAddr();
		}
	}

	throw OutOfMemoryException();
}

void BlockMemManager::free(void* p) {
	if (nullptr == p)
		throw NullPointerException();

	AllocatedBlock *block = AllocatedBlock::from(p);
	if (!AllocatedBlock::isAllocatedBlock(block)) {
		throw IllegalArgumentException("Try to free unallocated memorym or memory block was corrupted");
	}

	block->checkIntegrity(blockSize());
	static_cast<AllocatedBlock*>(block)->free(blockSize());
}

size_t BlockMemManager::getBlockSizeWithOverhead() const {
	return blockSize() + AllocatedBlock::getAllocatedDataSizeOverhead();
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
