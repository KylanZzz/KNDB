//
// Created by kylan on 7/20/2025.
//

#include <gtest/gtest.h>
#include <fstream>
#include <random>

#include "PageCache.hpp"
#include "FreeSpaceMap.hpp"
#include "FSMPage.hpp"
#include "IOHandler.hpp"

using namespace backend;

static void truncateFile(const char* filename) {
    std::ofstream ofs(filename, std::ios::trunc);
    ofs.close();
}

class FreeSpaceMapTest : public testing::Test {
protected:
    const std::string kTestFile = "fsm_test.db";
    std::unique_ptr<IOHandler> io;
    std::unique_ptr<PageCache> cache;
    std::unique_ptr<FreeSpaceMap> fsm;

    void SetUp() override {
        std::remove(kTestFile.c_str());

        io = std::make_unique<IOHandler>(kTestFile);
        io->createMultipleBlocks(FSMPage::getBlocksInPage());

        cache = std::make_unique<PageCache>(*io, cts::CACHE_SZ);

        auto fsmPage = std::make_unique<FSMPage>(0);
        cache->insertPage(std::move(fsmPage));

        fsm = std::make_unique<FreeSpaceMap>(*cache);
    }

    void TearDown() override {
        fsm.reset();
        cache.reset();
        io.reset();

        std::remove(kTestFile.c_str());
    }
};

TEST_F(FreeSpaceMapTest, CanAllocateFirstPageID) {
    pgid_t pageID = fsm->allocBit();
    EXPECT_EQ(pageID, 1);
    EXPECT_FALSE(fsm->isFree(pageID));
}

TEST_F(FreeSpaceMapTest, CanAllocateAllBitsInSingleFSMPage) {
    int capacity = FSMPage::getBlocksInPage();
    std::vector<pgid_t> allocated;

    for (int i = 1; i < capacity; ++i) { // skip bit 0, reserved for FSMPage itself
        pgid_t id = fsm->allocBit();
        allocated.push_back(id);
        EXPECT_EQ(id, i);
        EXPECT_FALSE(fsm->isFree(id));
    }

    EXPECT_TRUE(fsm->isFull());
}

TEST_F(FreeSpaceMapTest, CanFreeBitInSingleFSMPage) {
    int capacity = FSMPage::getBlocksInPage();
    std::vector<pgid_t> allocated;

    for (int i = 1; i < capacity; ++i) { // skip bit 0, reserved for FSMPage itself
        pgid_t id = fsm->allocBit();
        allocated.push_back(id);
        EXPECT_EQ(id, i);
        EXPECT_FALSE(fsm->isFree(id));
    }

    fsm->freeBit(1);

    EXPECT_FALSE(fsm->isFull());
}

TEST_F(FreeSpaceMapTest, CanReallocBitInSingleFSMPage) {
    int capacity = FSMPage::getBlocksInPage();
    std::vector<pgid_t> allocated;

    for (int i = 1; i < capacity; ++i) { // skip bit 0, reserved for FSMPage itself
        pgid_t id = fsm->allocBit();
        allocated.push_back(id);
        EXPECT_EQ(id, i);
        EXPECT_FALSE(fsm->isFree(id));
    }

    fsm->freeBit(10);
    EXPECT_EQ(fsm->allocBit(), 10);

    EXPECT_TRUE(fsm->isFull());
}

TEST_F(FreeSpaceMapTest, CanFreeAndReusePage) {
    pgid_t id = fsm->allocBit();
    fsm->freeBit(id);
    EXPECT_TRUE(fsm->isFree(id));

    pgid_t reused = fsm->allocBit();
    EXPECT_EQ(reused, id);
}

TEST_F(FreeSpaceMapTest, CanAllocateAcrossMultipleFSMPages) {
    int capacity = FSMPage::getBlocksInPage();

    for (int i = 1; i < capacity; ++i) {
        fsm->allocBit(); // fill first FSMPage
    }

    EXPECT_TRUE(fsm->isFull());

    // Simulate Pager logic: allocate new FSMPage and link it
    pgid_t newFSMPageID = capacity; // e.g., if first FSMPage manages 0–31, next FSMPage is at 32
    io->createMultipleBlocks(FSMPage::getBlocksInPage());
    auto fsmPage = std::make_unique<FSMPage>(newFSMPageID);
    cache->insertPage(std::move(fsmPage));

    // Link the next page
    fsm->linkFSMPage(newFSMPageID);

    EXPECT_FALSE(fsm->isFull());

    pgid_t nextID = fsm->allocBit();
    EXPECT_EQ(nextID, newFSMPageID + 1); // first usable bit in second FSMPage
}

TEST_F(FreeSpaceMapTest, DoubleFreeCausesAbort) {
    pgid_t id = fsm->allocBit();
    fsm->freeBit(id);

    // EXPECT_DEATH only works in its own process, so isolate this:
    EXPECT_DEATH(fsm->freeBit(id), ".*");
}

TEST_F(FreeSpaceMapTest, IsFreeReturnsCorrectStatus) {
    pgid_t id = fsm->allocBit();
    EXPECT_FALSE(fsm->isFree(id));

    fsm->freeBit(id);
    EXPECT_TRUE(fsm->isFree(id));
}

TEST_F(FreeSpaceMapTest, CanReuseBitMultipleTimes) {
    pgid_t id1 = fsm->allocBit();
    fsm->freeBit(id1);
    pgid_t id2 = fsm->allocBit();
    EXPECT_EQ(id2, id1);
    fsm->freeBit(id2);
    pgid_t id3 = fsm->allocBit();
    EXPECT_EQ(id3, id1);
}

TEST_F(FreeSpaceMapTest, CanAllocateAcrossThreeFSMPages) {
    int cap = FSMPage::getBlocksInPage();

    for (int i = 1; i < cap; ++i)
        fsm->allocBit(); // fill first FSMPage

    pgid_t fsm2 = cap;
    io->createMultipleBlocks(FSMPage::getBlocksInPage());
    cache->insertPage(std::make_unique<FSMPage>(fsm2));
    cache->retrievePage<FSMPage>(0).setNextPageID(fsm2);

    for (int i = 1; i < cap; ++i)
        fsm->allocBit(); // fill second FSMPage

    pgid_t fsm3 = cap * 2;
    io->createMultipleBlocks(FSMPage::getBlocksInPage());
    cache->insertPage(std::make_unique<FSMPage>(fsm3));
    cache->retrievePage<FSMPage>(0).setNextPageID(fsm3);

    pgid_t id = fsm->allocBit();
    EXPECT_EQ(id, fsm3 + 1);
}

TEST_F(FreeSpaceMapTest, ReallocatesFreedBitsAcrossMultipleFSMPages) {
    int cap = FSMPage::getBlocksInPage();

    // Fill FSM page 0 (bits 1..cap-1)
    for (int i = 1; i < cap; ++i)
        fsm->allocBit();

    // Insert and link FSM page 1
    pgid_t fsm1 = cap;
    io->createMultipleBlocks(cap);
    cache->insertPage(std::make_unique<FSMPage>(fsm1));
    cache->retrievePage<FSMPage>(0).setNextPageID(fsm1);

    // Fill FSM page 1 (bits 1..cap-1)
    for (int i = 1; i < cap; ++i)
        fsm->allocBit();

    // Insert and link FSM page 2
    pgid_t fsm2 = 2 * cap;
    io->createMultipleBlocks(cap);
    cache->insertPage(std::make_unique<FSMPage>(fsm2));
    fsm->linkFSMPage(fsm2);

    // Fill FSM page 2 (bits 1..cap-1)
    for (int i = 1; i < cap; ++i)
        fsm->allocBit();

    // Now free a few pages from different FSM pages
    fsm->freeBit(2);                  // FSM 0
    fsm->freeBit(fsm1 + 5);           // FSM 1
    fsm->freeBit(fsm2 + cap - 1);     // FSM 2

    // Reallocate and check that each one is reused
    pgid_t r1 = fsm->allocBit();
    EXPECT_FALSE(fsm->isFree(r1));

    pgid_t r2 = fsm->allocBit();
    EXPECT_FALSE(fsm->isFree(r2));

    pgid_t r3 = fsm->allocBit();
    EXPECT_FALSE(fsm->isFree(r3));

    EXPECT_TRUE(fsm->isFull());
}

TEST_F(FreeSpaceMapTest, StressTest_ReallocateFreedPagesAcrossManyFSMPages) {
    const int fsmPageCount = 10; // will create 10 FSMPages
    const int cap = FSMPage::getBlocksInPage();

    // Fill all FSMPages
    for (int i = fsmPageCount; i < fsmPageCount * cap; ++i) {
        if (fsm->isFull()) {
            // Allocate and link a new FSMPage
            pgid_t fsmPageID = io->createMultipleBlocks(cap);
            cache->insertPage(std::make_unique<FSMPage>(fsmPageID));

            // Link new page to first page
            fsm->linkFSMPage(fsmPageID);
        }
        fsm->allocBit();
    }

    EXPECT_TRUE(fsm->isFull());

    // Free every Nth page across all FSMs
    std::vector<pgid_t> freed;
    for (pgid_t i = fsmPageCount; i < fsmPageCount * cap; i += 7) {
        fsm->freeBit(i);
        freed.push_back(i);
    }

    // Shuffle for fun (optional)
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(freed.begin(), freed.end(), g);

    // Reallocate all freed pages
    std::set<pgid_t> reallocated;
    for (size_t i = 0; i < freed.size(); ++i) {
        pgid_t id = fsm->allocBit();
        reallocated.insert(id);
    }

    // All freed pages should have been reused exactly once
    EXPECT_EQ(reallocated.size(), freed.size());
    for (pgid_t id : freed)
        EXPECT_TRUE(reallocated.count(id));
}