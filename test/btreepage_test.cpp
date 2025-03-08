//
// Created by Kylan Chen on 3/6/25.
//

#include <gtest/gtest.h>

#include "BtreeNodePage.hpp"
#include "constants.hpp"

TEST(BtreeNodePageTest, PageInitilizationIsCorrect) {
//    vector<variants> types = {string(), string(), double(), int(), float(), int()};
    BtreeNodePage<vector<variants>> node(6, 5, true, true, 3);
    
    ASSERT_TRUE(node.getChildren().empty());
    ASSERT_TRUE(node.getCells().empty());
    ASSERT_TRUE(node.isLeaf());
    ASSERT_TRUE(node.isRoot());
    ASSERT_EQ(5, node.getParentID());
}

TEST(BtreeNodePageTest, PageInitializationAndSerialization) {
//    vector<variants> types = {string(), string(), double(), int(), float(), int()};
    BtreeNodePage<vector<variants>> node(6, 5, true, true, 3);

    ByteVec bytes(cts::PG_SZ);
    node.toBytes(bytes);

    BtreeNodePage<vector<variants>> node2(bytes, 3);
    ASSERT_TRUE(node2.getChildren().empty());
    ASSERT_TRUE(node2.getCells().empty());
    ASSERT_TRUE(node2.isLeaf());
    ASSERT_TRUE(node2.isRoot());
    ASSERT_EQ(6 - 1, node2.getMinKeys());
    ASSERT_EQ(5, node2.getParentID());
    ASSERT_EQ(2 * 6 - 1, node2.getMaxKeys());
}

TEST(BtreeNodePageTest, AddingCellWorks) {
//    vector<variants> types = {string(), double(), int()};
    variants key = int();
    BtreeNodePage<vector<variants>> node(6, 5, true, true, 3);

    node.getCells().push_back({int(33), {string("Kylan"), double(3.1144), int(10)}});
    node.getChildren().push_back(100);
    node.getChildren().push_back(101);

    ByteVec bytes(cts::PG_SZ);
    node.toBytes(bytes);

    BtreeNodePage<vector<variants>> node2(bytes, 3);
    ASSERT_EQ(2, node2.getChildren().size());
    ASSERT_EQ(1, node2.getCells().size());

    variants expected = 33;
    ASSERT_EQ(expected, node2.getCells()[0].key);
    expected = "Kylan";
    ASSERT_EQ(expected, node2.getCells()[0].value[0]);
    expected = double(3.1144);
    ASSERT_EQ(expected, node2.getCells()[0].value[1]);
    expected = int(10);
    ASSERT_EQ(expected, node2.getCells()[0].value[2]);
    ASSERT_EQ(100, node2.getChildren()[0]);
    ASSERT_EQ(101, node2.getChildren()[1]);
}

TEST(BtreeNodePageTest, AddingMultipleCellsWorks) {
//    vector<variants> types = {string(), string(), int()};
    variants key = string();
    size_t MX_SZ = (cts::PG_SZ - 500) / (cts::STR_SZ + cts::STR_SZ + sizeof(int) + cts::STR_SZ);
    size_t deg = (MX_SZ + 1) / 2;
    BtreeNodePage<vector<variants>> node(6, 5, true, true, 3);

    vector<variants> exp_key;
    vector<vector<variants>> exp_tuple;
    vector<size_t> exp_children;

    int count = 0;
    node.getChildren().push_back(499);
    exp_children.push_back(499);
    while (node.getCells().size() < node.getMaxKeys()) {
        variants cell_key = "Cell #" + std::to_string(count);
        vector<variants> tuple = {"val1 #" + std::to_string(count), "val2 #" + std::to_string
                                   (count + 100), count + 200};
        node.getCells().push_back({cell_key, tuple});
        exp_key.push_back(cell_key);
        exp_tuple.push_back(tuple);
        node.getChildren().push_back(count + 500);
        exp_children.push_back(count + 500);
    }

    for (int i = 0; i < node.getCells().size(); i++) {
        ASSERT_EQ(node.getCells()[i].key, exp_key[i]);
        ASSERT_EQ(node.getCells()[i].value, exp_tuple[i]);
        ASSERT_EQ(node.getChildren()[i], exp_children[i]);
    }

    ByteVec bytes(cts::PG_SZ);
    node.toBytes(bytes);
    BtreeNodePage<vector<variants>> node2(bytes, 3);

    for (int i = 0; i < node.getCells().size(); i++) {
        ASSERT_EQ(node2.getCells()[i].key, exp_key[i]);
        ASSERT_EQ(node2.getCells()[i].value, exp_tuple[i]);
        ASSERT_EQ(node2.getChildren()[i], exp_children[i]);
    }
}


TEST(BtreeNodePageTest, RowPtrBtreeNodeWorks) {
    variants key = int();
    BtreeNodePage<kndb_types::RowPtr> node(6, 5, true, true, 3);

    node.getCells().push_back({3, {4, 10}});
    node.getChildren().push_back(100);
    node.getChildren().push_back(101);

    ByteVec bytes(cts::PG_SZ);
    node.toBytes(bytes);

    BtreeNodePage<kndb_types::RowPtr> node2(bytes, 3);
    ASSERT_EQ(2, node2.getChildren().size());
    ASSERT_EQ(1, node2.getCells().size());

    variants expected = 3;
    ASSERT_EQ(expected, node2.getCells()[0].key);
    kndb_types::RowPtr row = {4,10};
    ASSERT_EQ(row.pageID, node2.getCells()[0].value.pageID);
    ASSERT_EQ(row.cellID, node2.getCells()[0].value.cellID);
    ASSERT_EQ(100, node2.getChildren()[0]);
    ASSERT_EQ(101, node2.getChildren()[1]);
}

TEST(BtreeNodePageTest, InsertingManyRowPtrWorks) {
    variants key = int();
    size_t MX_SZ = cts::PG_SZ / 30;
    size_t deg = (MX_SZ + 1) / 2;
    BtreeNodePage<kndb_types::RowPtr> node(deg, 5, false, false, 3);

    node.getChildren().push_back(1111);
    for (int i = 0; i < MX_SZ; i++) {
        RowPtr ptr{}; ptr.cellID = i * 2; ptr.pageID = i * 3;
        node.getCells().push_back({i, ptr});
        node.getChildren().push_back(i * 4);
    }

    auto exp_cell = node.getCells();
    auto exp_child = node.getChildren();

    ByteVec bytes(cts::PG_SZ);
    node.toBytes(bytes);
    BtreeNodePage<kndb_types::RowPtr> node2(bytes, 3);

    for (int i = 0; i < node.getCells().size(); i++) {

        ASSERT_EQ(node2.getCells()[i].value.cellID, exp_cell[i].value.cellID);
        ASSERT_EQ(node2.getCells()[i].value.pageID, exp_cell[i].value.pageID);
        ASSERT_EQ(node2.getCells()[i].key, exp_cell[i].key);
        ASSERT_EQ(node2.getChildren(), exp_child);
    }
}