//
// Created by Kylan Chen on 2/28/25.
//

#include <gtest/gtest.h>

#include "FSMPage.hpp"
#include "utility.hpp"

TEST(FSMPageTest, NewlyCreatedPageIsAllFree) {
    FSMPage page(1);
    for (size_t i = 0; i < page.getSpaceLeft(); ++i) {
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
    ASSERT_THROW(page.isFree(page.getSpaceLeft()), std::invalid_argument);
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
    original.to_bytes(serialized);

    FSMPage deserialized(1, serialized);
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
    original.to_bytes(serialized);

    FSMPage deserialized(1, serialized);
    ASSERT_FALSE(deserialized.isFree(7));
    ASSERT_FALSE(deserialized.isFree(15));
}

TEST(FSMPageTest, FindNextFreeReturnsFirstAvailableBit) {
    FSMPage page(1);
    page.allocBit(0);
    ASSERT_EQ(page.findNextFree(), 1);
}

TEST(FSMPageTest, FindNextFreeThrowsIfNoFreeBits) {
    FSMPage page(1);

    int n = page.getSpaceLeft();
    for (size_t i = 0; i < n; ++i) {
        page.allocBit(i);
    }
    ASSERT_EQ(0, page.getSpaceLeft());
    ASSERT_THROW(page.findNextFree(), std::invalid_argument);
}

TEST(FSMPageTest, SerializationPreservesFindNextFree) {
    FSMPage original(1);
    original.allocBit(0);
    original.allocBit(1);

    ByteVec serialized(cts::PG_SZ);
    original.to_bytes(serialized);

    FSMPage deserialized(1, serialized);
    ASSERT_EQ(deserialized.findNextFree(), 2);
}

TEST(FSMPageTest, AllocateAllBitsThenFreeAndReallocate) {
    FSMPage page(1);

    // Allocate all bits
    int n1 = page.getSpaceLeft();
    for (size_t i = 0; i < n1; ++i) {
        page.allocBit(i);
        ASSERT_FALSE(page.isFree(i));
    }
    ASSERT_THROW(page.findNextFree(), std::invalid_argument);

    // Free all bits
    for (size_t i = 0; i < n1; ++i) {
        page.freeBit(i);
        ASSERT_TRUE(page.isFree(i));
    }

    // Reallocate all bits again
    int n2 = page.getSpaceLeft();
    for (size_t i = 0; i < n2; ++i) {
        page.allocBit(i);
        ASSERT_FALSE(page.isFree(i));
    }
}

TEST(FSMPageTest, SerializationPreservesFullAllocationCycle) {
    FSMPage original(1);

    // Allocate all bits
    int n = original.getSpaceLeft();
    for (size_t i = 0; i < original.getSpaceLeft(); ++i) {
        original.allocBit(i);
    }

    ByteVec serialized(cts::PG_SZ);
    original.to_bytes(serialized);
    FSMPage deserialized(1, serialized);

    int n2 = deserialized.getSpaceLeft();
    for (size_t i = 0; i < n2; ++i) {
        ASSERT_FALSE(deserialized.isFree(i));
    }

    // Free all bits
    for (size_t i = 0; i < n2; ++i) {
        deserialized.freeBit(i);
    }

    ByteVec reserialized(cts::PG_SZ);
    deserialized.to_bytes(reserialized);
    FSMPage reloaded(1, reserialized);

    int n3 = reloaded.getSpaceLeft();
    for (size_t i = 0; i < n3; ++i) {
        ASSERT_TRUE(reloaded.isFree(i));
    }
}