//
// Created by Kylan Chen on 3/6/25.
//

#include <gtest/gtest.h>

#include "BtreeNodePage.hpp"
#include "constants.hpp"

TEST(BtreeNodePageTest, PageInitilizationIsCorrect) {
//    vector<variants> types = {string(), string(), double(), int(), float(), int()};
    BtreeNodePage node(6, 5, true, true, 3);
    
    ASSERT_TRUE(node.getChildren().empty());
    ASSERT_TRUE(node.getCells().empty());
    ASSERT_TRUE(node.isLeaf());
    ASSERT_TRUE(node.isRoot());
    ASSERT_FALSE(node.isFull());
    ASSERT_EQ(5, node.getParentID());
    ASSERT_EQ(6, node.getDegree());
}

TEST(BtreeNodePageTest, PageInitializationAndSerialization) {
//    vector<variants> types = {string(), string(), double(), int(), float(), int()};
    BtreeNodePage node(6, 5, true, true, 3);

    ByteVec bytes(cts::PG_SZ);
    node.toBytes(bytes);

    BtreeNodePage node2(bytes, 3);
    ASSERT_TRUE(node2.getChildren().empty());
    ASSERT_TRUE(node2.getCells().empty());
    ASSERT_TRUE(node2.isLeaf());
    ASSERT_TRUE(node2.isRoot());
    ASSERT_FALSE(node2.isFull());
    ASSERT_EQ(5, node2.getParentID());
    ASSERT_EQ(6, node2.getDegree());
}

TEST(BtreeNodePageTest, AddingCellWorks) {
//    vector<variants> types = {string(), double(), int()};
    variants key = int();
    BtreeNodePage node(6, 5, true, true, 3);

    node.getCells().push_back({int(33), {string("Kylan"), double(3.1144), int(10)}});
    node.getChildren().push_back(100);
    node.getChildren().push_back(101);

    ByteVec bytes(cts::PG_SZ);
    node.toBytes(bytes);

    BtreeNodePage node2(bytes, 3);
    ASSERT_EQ(2, node2.getChildren().size());
    ASSERT_EQ(1, node2.getCells().size());

    variants expected = 33;
    ASSERT_EQ(expected, node2.getCells()[0].key);
    expected = "Kylan";
    ASSERT_EQ(expected, node2.getCells()[0].tuple[0]);
    expected = double(3.1144);
    ASSERT_EQ(expected, node2.getCells()[0].tuple[1]);
    expected = int(10);
    ASSERT_EQ(expected, node2.getCells()[0].tuple[2]);
    ASSERT_EQ(100, node2.getChildren()[0]);
    ASSERT_EQ(101, node2.getChildren()[1]);
}

TEST(BtreeNodePageTest, AddingMultipleCellsWorks) {
//    vector<variants> types = {string(), string(), int()};
    variants key = string();
    size_t MX_SZ = (cts::PG_SZ - 500) / (cts::STR_SZ + cts::STR_SZ + sizeof(int) + cts::STR_SZ);
    size_t deg = (MX_SZ + 1) / 2;
    BtreeNodePage node(6, 5, true, true, 3);

    vector<variants> exp_key;
    vector<vector<variants>> exp_tuple;
    vector<size_t> exp_children;

    int count = 0;
    node.getChildren().push_back(499);
    exp_children.push_back(499);
    while (!node.isFull()) {
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
        ASSERT_EQ(node.getCells()[i].tuple, exp_tuple[i]);
        ASSERT_EQ(node.getChildren()[i], exp_children[i]);
    }

    ByteVec bytes(cts::PG_SZ);
    node.toBytes(bytes);
    BtreeNodePage node2(bytes, 3);

    for (int i = 0; i < node.getCells().size(); i++) {
        ASSERT_EQ(node2.getCells()[i].key, exp_key[i]);
        ASSERT_EQ(node2.getCells()[i].tuple, exp_tuple[i]);
        ASSERT_EQ(node2.getChildren()[i], exp_children[i]);
    }
}

