#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "../BlockMemManager.h"

using testing::Test;

namespace test {
	template <class TMemManager>
	class MemManagerTest : public Test {
	protected:
		typedef std::unique_ptr<TMemManager> MemManagerPtr;
	};

	using MemManagerTypes = testing::Types<BlockMemManager>;
	TYPED_TEST_SUITE(MemManagerTest, MemManagerTypes);

	TYPED_TEST(MemManagerTest, errorThrown_whenIncorrectCtorParams) {
		char memory[12];

		// null memory block
		EXPECT_THROW(new TypeParam(nullptr, 2, 1), NullPointerException);

		// one if the sizes is 0
		EXPECT_THROW(new TypeParam(memory, 0, 1), IllegalArgumentException);
		EXPECT_THROW(new TypeParam(memory, 2, 0), IllegalArgumentException);

		// sizeBlock > sizeMem
		EXPECT_THROW(new TypeParam(memory, 1, 2), IllegalArgumentException);

		// TODO: for blocked mem manager take into account 'Block' overhead
	}

	TYPED_TEST(MemManagerTest, memoryAllocated_whenParamsCorrect) {
		char memory[12];
		typename TestFixture::MemManagerPtr memManager(new TypeParam(memory, sizeof(memory), sizeof(int)));

		int* p = static_cast<int*>(memManager->allocate());
		*p = 0xfefefefe;

		memManager->free(p);
	}

	TYPED_TEST(MemManagerTest, errorThrown_whenFreeUnnalocatedBlock) {
		char memory[12];
		typename TestFixture::MemManagerPtr memManager(new TypeParam(memory, sizeof(memory), sizeof(int)));

		EXPECT_THROW(memManager->free(nullptr), NullPointerException);
		EXPECT_THROW(memManager->free(memory), IllegalArgumentException);

		void* p = memManager->allocate();
		EXPECT_THROW(memManager->free(static_cast<char*>(p) + 1), IllegalArgumentException);
	}

	TYPED_TEST(MemManagerTest, errorThrown_whenFreeTwice) {
		char memory[24];
		typename TestFixture::MemManagerPtr memManager(new TypeParam(memory, sizeof(memory), 16));

		void* p = memManager->allocate();
		EXPECT_NO_THROW(memManager->free(p));
		EXPECT_THROW(memManager->free(p), IllegalArgumentException);
	}

	TYPED_TEST(MemManagerTest, errorThrown_whenOutOfmemoryAlignedRegion) {
		char memory[24];
		typename TestFixture::MemManagerPtr memManager(new TypeParam(memory, sizeof(memory), 16));

		memManager->allocate();
		EXPECT_THROW(memManager->allocate(), OutOfMemoryException);
	}

	TYPED_TEST(MemManagerTest, errorThrown_whenOutOfmemoryUnalignedRegion) {
		char memory[27];
		typename TestFixture::MemManagerPtr memManager(new TypeParam(memory, sizeof(memory), 16));

		memManager->allocate();
		EXPECT_THROW(memManager->allocate(), OutOfMemoryException);
	}
}
