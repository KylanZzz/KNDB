//
// Created by Kylan Chen on 3/1/25.
//

#include <gtest/gtest.h>

#include "FreeSpaceMap.hpp"

TEST(FreeSpaceMapTest, ConstructionInitializesDatabaseSize) {
    IOHandler ioHandler("testfile.db");
    FreeSpaceMap fsm(ioHandler, 0);

    ASSERT_EQ(ioHandler.getNumBlocks(), 1); // FSMPage should initialize at least one block
}

TEST(FreeSpaceMapTest, AllocatingPagesIncreasesBlockCount) {
    IOHandler ioHandler("testfile.db");
    FreeSpaceMap fsm(ioHandler, 0);

    size_t initialBlocks = ioHandler.getNumBlocks();

    fsm.allocPage();
    ASSERT_EQ(ioHandler.getNumBlocks(), initialBlocks + 1);

    fsm.allocPage();
    ASSERT_EQ(ioHandler.getNumBlocks(), initialBlocks + 2);
}