//
// Created by Kylan Chen on 2/28/25.
//

#include <gtest/gtest.h>

#include "FSMPage.hpp"
#include "utility.hpp"

TEST(FSMPageTest, NewlyCreatedPageIsAllFree) {
    FSMPage page(1);
    ASSERT_FALSE(page.isFree(0));
    for (size_t i = 1; i < page.getSpaceLeft(); ++i) {
        ASSERT_TRUE(page.isFree(i));
    }
}

TEST(FSMPageTest, AllocatedBitIsNotFree) {
    FSMPage page(1);
    page.allocBit(5);
    ASSERT_FALSE(page.isFree(5));
}

TEST(FSMPageTest, FreeingABitMakesItFreeAgain) {
    FSMPage page(1);
    page.allocBit(10);
    page.freeBit(10);
    ASSERT_TRUE(page.isFree(10));
}

TEST(FSMPageTest, OutOfBoundsCheckThrowsException) {
    FSMPage page(1);
    ASSERT_THROW(page.isFree(page.getBlocksInPage()), std::invalid_argument);
}

TEST(FSMPageTest, DefaultPageHasNoNextPage) {
    FSMPage page(1);
    ASSERT_FALSE(page.hasNextPage());
}

TEST(FSMPageTest, SetNextPageWorks) {
    FSMPage page(1);
    page.setNextPageID(42);
    ASSERT_TRUE(page.hasNextPage());
    ASSERT_EQ(page.getNextPageID(), 42);
}

TEST(FSMPageTest, SerializationPreservesNextPage) {
    FSMPage original(1);
    original.setNextPageID(99);

    ByteVec serialized(cts::PG_SZ);
    original.toBytes(serialized);

    FSMPage deserialized(serialized, 1);
    ASSERT_TRUE(deserialized.hasNextPage());
    ASSERT_EQ(deserialized.getNextPageID(), 99);
}

TEST(FSMPageTest, AllocBitMarksBitAsUsed) {
    FSMPage page(1);
    page.allocBit(5);
    ASSERT_FALSE(page.isFree(5));
}

TEST(FSMPageTest, AllocBitThrowsIfAlreadyAllocated) {
    FSMPage page(1);
    page.allocBit(5);
    ASSERT_THROW(page.allocBit(5), std::invalid_argument);
}

TEST(FSMPageTest, SerializationPreservesAllocatedBits) {
    FSMPage original(1);
    original.allocBit(7);
    original.allocBit(15);

    ByteVec serialized(cts::PG_SZ);
    original.toBytes(serialized);

    FSMPage deserialized(serialized, 1);
    ASSERT_FALSE(deserialized.isFree(7));
    ASSERT_FALSE(deserialized.isFree(15));
}

TEST(FSMPageTest, FindNextFreeReturnsFirstAvailableBit) {
    FSMPage page(1);
    page.allocBit(1);
    ASSERT_EQ(page.findNextFree(), 2);
}

TEST(FSMPageTest, FindNextFreeThrowsIfNoFreeBits) {
    FSMPage page(1);

    for (size_t i = 1; i < page.getBlocksInPage(); ++i) {
        page.allocBit(i);
    }
    ASSERT_EQ(0, page.getSpaceLeft());
    ASSERT_THROW(page.findNextFree(), std::invalid_argument);
}

TEST(FSMPageTest, SerializationPreservesFindNextFree) {
    FSMPage original(1);
    original.allocBit(1);
    original.allocBit(2);

    ByteVec serialized(cts::PG_SZ);
    original.toBytes(serialized);

    FSMPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.findNextFree(), 3);
}

TEST(FSMPageTest, AllocateAllBitsThenFreeAndReallocate) {
    FSMPage page(1);

    // Allocate all bits
    for (size_t i = 1; i < page.getBlocksInPage(); ++i) {
        page.allocBit(i);
        ASSERT_FALSE(page.isFree(i));
    }
    ASSERT_THROW(page.findNextFree(), std::invalid_argument);

    // Free all bits
    for (size_t i = 1; i < page.getBlocksInPage(); ++i) {
        page.freeBit(i);
        ASSERT_TRUE(page.isFree(i));
    }

    // Reallocate all bits again
    for (size_t i = 1; i < page.getBlocksInPage(); ++i) {
        page.allocBit(i);
        ASSERT_FALSE(page.isFree(i));
    }
}

TEST(FSMPageTest, SerializationPreservesFullAllocationCycle) {
    FSMPage original(1);

    // Allocate all bits
    for (size_t i = 1; i < original.getBlocksInPage(); ++i) {
        original.allocBit(i);
    }

    ByteVec serialized(cts::PG_SZ);
    original.toBytes(serialized);
    FSMPage deserialized(serialized, 1);

    for (size_t i = 1; i < original.getBlocksInPage(); ++i) {
        ASSERT_FALSE(deserialized.isFree(i));
    }

    // Free all bits
    for (size_t i = 1; i < original.getBlocksInPage(); ++i) {
        deserialized.freeBit(i);
    }

    ByteVec reserialized(cts::PG_SZ);
    deserialized.toBytes(reserialized);
    FSMPage reloaded(reserialized, 1);

    for (size_t i = 1; i < original.getBlocksInPage(); ++i) {
        ASSERT_TRUE(reloaded.isFree(i));
    }
}