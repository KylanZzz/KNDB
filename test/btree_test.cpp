//
// Created by Kylan Chen on 3/8/25.
//

#include <gtest/gtest.h>
#include <fstream>
#include <random>

#include "Btree.hpp"
#include "BtreeNodePage.hpp"

using namespace backend;

class BtreeTest : public testing::Test {
protected:
    static constexpr int SEED = 152;
    static constexpr u16 DEGREE = 60;
    const std::string kTestFile = "testfile.db";
    u32 root_id;

    std::unique_ptr<IOHandler> ioHandler;
    std::unique_ptr<PageCache> pageCache;
    std::unique_ptr<FreeSpaceMap> fsm;
    std::unique_ptr<Pager> pager;
    std::unique_ptr<Btree<Vec<Vari>>> btree;

    void SetUp() override {
        std::remove(kTestFile.c_str());
        initEnv();
    }

    void TearDown() override {
        resetEnv();
        std::remove(kTestFile.c_str());
    }

    void initEnv() {
        ioHandler = std::make_unique<IOHandler>(kTestFile);
        pageCache = std::make_unique<PageCache>(*ioHandler, cts::CACHE_SZ);
        fsm = std::make_unique<FreeSpaceMap>(*pageCache);
        pager = std::make_unique<Pager>(*fsm, *ioHandler, *pageCache);
        root_id = pager->createNewPage<BtreeNodePage<Vec<Vari>>>(
            DEGREE, cts::U32_INVALID, true, true
        ).getPageID();
        btree = std::make_unique<Btree<Vec<Vari>>>(root_id, *pager, DEGREE);
    }

    void resetEnv() {
        btree.reset();
        pager.reset();
        pageCache.reset();
        fsm.reset();
        ioHandler.reset();
    }
};

TEST_F(BtreeTest, BasicInsertWorks) {
    Vari key = "kylan";
    Vec<Vari> tuple = {"kylan", double(3.0), 4};
    btree->insert(tuple, key);
    ASSERT_EQ(btree->search(key), tuple);
}

TEST_F(BtreeTest, BasicSearchWorks) {
    Vari key = "kylan";
    Vec<Vari> tuple = {"kylan", double(3.0), 4.0};
    auto& node = pager->getPage<BtreeNodePage<Vec<Vari>>>(root_id);
    node.cells().push_back({key, tuple});
    ASSERT_EQ(btree->search(key), tuple);
}

TEST_F(BtreeTest, BasicUpdateWorks) {
    Vari key = "kylan";
    Vec<Vari> tuple = {"kylan", double(3.0), 4};
    auto& node = pager->getPage<BtreeNodePage<Vec<Vari>>>(root_id);
    node.cells().push_back({key, tuple});
    Vec<Vari> tuple2 = {"my other name", double(6.9), 14};
    btree->update(tuple2, key);
    ASSERT_EQ(btree->search(key), tuple2);
}

TEST_F(BtreeTest, StressTestInsertAndSearchOneHundredThousand) {
    Vec<int> keys(100000);
    std::iota(keys.begin(), keys.end(), 1); // Fill with 1..100000
    std::shuffle(keys.begin(), keys.end(), std::mt19937(SEED));

    std::shuffle(keys.begin(), keys.end(), std::mt19937(SEED + 1));
    Vec<Vec<Vari>> tuples;
    for (int key : keys) {
        Vec<Vari> tuple = {key, double(key * 1.5), int(key % 100)};
        btree->insert(tuple, key);
        tuples.push_back(tuple);
    }

    for (int i = 0; i < keys.size(); i++) {
        ASSERT_EQ(btree->search(keys[i]), tuples[i]);
    }
}
