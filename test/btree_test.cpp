//
// Created by Kylan Chen on 3/8/25.
//

#include <gtest/gtest.h>
#include <fstream>
#include <random>

#include "Btree.hpp"
#include "BtreeNodePage.hpp"

class BtreeTest : public ::testing::Test {
protected:
    static constexpr int SEED = 152;
    static constexpr size_t DEGREE = 60;
    std::string filename = "testfile.db";
    size_t root_id;
    IOHandler* ioHandler;
    Pager* pager;
    Btree<vector<variants>>* btree;

    void SetUp() override {
        std::ofstream file(filename, std::ios::trunc);
        file.close();
        ioHandler = new IOHandler(filename);
        pager = new Pager(*ioHandler);
        root_id = pager->createNewPage<BtreeNodePage<vector<variants>>>(
                DEGREE, cts::SIZE_T_OUT_OF_BOUNDS, true, true
        ).getPageID();
        btree = new Btree<vector<variants>>(root_id, *pager, DEGREE);
    }

    void TearDown() override {
        delete btree;
        delete pager;
        delete ioHandler;
    }
};

TEST_F(BtreeTest, BasicInsertWorks) {
    variants key = "kylan";
    vector<variants> tuple = {"kylan", double(3.0), 4};
    btree->insert(tuple, key);
    ASSERT_EQ(btree->search(key), tuple);
}

TEST_F(BtreeTest, BasicSearchWorks) {
    variants key = "kylan";
    vector<variants> tuple = {"kylan", double(3.0), 4.0};
    auto& node = pager->getPage<BtreeNodePage<vector<variants>>>(root_id);
    node.getCells().push_back({key, tuple});
    node.getChildren().push_back(3);
    node.getChildren().push_back(4);
    ASSERT_EQ(btree->search(key), tuple);
}

TEST_F(BtreeTest, BasicUpdateWorks) {
    variants key = "kylan";
    vector<variants> tuple = {"kylan", double(3.0), 4};
    auto& node = pager->getPage<BtreeNodePage<vector<variants>>>(root_id);
    node.getCells().push_back({key, tuple});
    node.getChildren().push_back(3);
    node.getChildren().push_back(4);

    vector<variants> tuple2 = {"my other name", double(6.9), 14};
    btree->update(tuple2, key);
    ASSERT_EQ(btree->search(key), tuple2);
}

TEST_F(BtreeTest, StressTestInsertAndSearchFiveMillion) {
    std::vector<int> keys(5000000);
    std::iota(keys.begin(), keys.end(), 1); // Fill with 1..5000000
    std::shuffle(keys.begin(), keys.end(), std::mt19937(SEED));

    std::shuffle(keys.begin(), keys.end(), std::mt19937(SEED + 1));
    std::vector<vector<variants>> tuples;
    for (int key : keys) {
        vector<variants> tuple = {key, double(key * 1.5), int(key % 100)};
        btree->insert(tuple, key);
        tuples.push_back(tuple);
    }

    for (int i = 0; i < keys.size(); i++) {
        ASSERT_EQ(btree->search(keys[i]), tuples[i]);
    }
}
