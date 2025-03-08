//
// Created by Kylan Chen on 3/8/25.
//

#include <gtest/gtest.h>
#include <fstream>

#include "Btree.hpp"
#include "BtreeNodePage.hpp"

TEST(BtreeTest, BasicSearchWorks) {
    std::ofstream file("testfile.db", std::ios::trunc);
    file.close();
    IOHandler ioHandler("testfile.db");
    Pager pager(ioHandler);

    size_t root_id = pager.createNewPage<BtreeNodePage<vector<variants>>>(
            100, cts::SIZE_T_OUT_OF_BOUNDS, true, true
    ).getPageID();

    variants key = "kylan";
    vector<variants> tuple = {"kylan", double(3.0), 4.0};

    auto& node = pager.getPage<BtreeNodePage<vector<variants>>>(root_id);
    node.getCells().push_back({key, tuple});
    node.getChildren().push_back(3);
    node.getChildren().push_back(4);

    Btree<vector<variants>> btree(root_id, pager);
    ASSERT_EQ(btree.search(key), tuple);
}


TEST(BtreeTest, BasicUpdateWorks) {
    std::ofstream file("testfile.db", std::ios::trunc);
    file.close();
    IOHandler ioHandler("testfile.db");
    Pager pager(ioHandler);

    size_t root_id = pager.createNewPage<BtreeNodePage<vector<variants>>>(
            100, cts::SIZE_T_OUT_OF_BOUNDS, true, true
    ).getPageID();

    variants key = "kylan";
    vector<variants> tuple = {"kylan", double(3.0), 4};

    auto& node = pager.getPage<BtreeNodePage<vector<variants>>>(root_id);
    node.getCells().push_back({key, tuple});
    node.getChildren().push_back(3);
    node.getChildren().push_back(4);

    Btree<vector<variants>> btree(root_id, pager);
    vector<variants> tuple2 = {"my other name", double(6.9), 14};
    btree.update(tuple2, key);
    ASSERT_EQ(btree.search(key), tuple2);
}