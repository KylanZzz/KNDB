#include <gtest/gtest.h>
#include "BtreeNodePage.hpp"
#include "constants.hpp"
#include <stdexcept>

class BtreeNodePageTestFixture : public ::testing::Test {
protected:
    int defaultPageID = 3;

    // Helper: Serializes a node and returns a new node constructed from the bytes.
    template <typename T>
    BtreeNodePage<T> roundTrip(BtreeNodePage<T>& node) {
        ByteVec bytes(cts::PG_SZ);
        node.toBytes(bytes);
        return BtreeNodePage<T>(bytes, defaultPageID);
    }
};

TEST_F(BtreeNodePageTestFixture, PageInitializationIsCorrect) {
    BtreeNodePage<vector<variants>> node(6, 5, true, false, defaultPageID);

    ASSERT_TRUE(node.getChildren().empty());
    ASSERT_TRUE(node.getCells().empty());
    ASSERT_FALSE(node.isLeaf());
    ASSERT_TRUE(node.isRoot());
    ASSERT_EQ(5, node.getParentID());
}

TEST_F(BtreeNodePageTestFixture, PageInitializationAndSerialization) {
    BtreeNodePage<vector<variants>> node(6, 5, true, false, defaultPageID);
    BtreeNodePage<vector<variants>> node2 = roundTrip(node);

    ASSERT_TRUE(node2.getChildren().empty());
    ASSERT_TRUE(node2.getCells().empty());
    ASSERT_FALSE(node2.isLeaf());
    ASSERT_TRUE(node2.isRoot());
    ASSERT_EQ(6 - 1, node2.getMinKeys());
    ASSERT_EQ(5, node2.getParentID());
    ASSERT_EQ(2 * 6 - 1, node2.getMaxKeys());
}

TEST_F(BtreeNodePageTestFixture, AddingSingleCellWorks) {
    BtreeNodePage<vector<variants>> node(6, 5, true, false, defaultPageID);
    node.getCells().push_back({33, {string("Kylan"), 3.1144, 10}});
    node.getChildren().push_back(100);
    node.getChildren().push_back(101);

    BtreeNodePage<vector<variants>> node2 = roundTrip(node);
    ASSERT_EQ(2, node2.getChildren().size());
    ASSERT_EQ(1, node2.getCells().size());

    ASSERT_EQ(variants(33), node2.getCells()[0].key);
    ASSERT_EQ(variants("Kylan"), node2.getCells()[0].value[0]);
    ASSERT_EQ(variants(3.1144), node2.getCells()[0].value[1]);
    ASSERT_EQ(variants(10), node2.getCells()[0].value[2]);
    ASSERT_EQ(100, node2.getChildren()[0]);
    ASSERT_EQ(101, node2.getChildren()[1]);
}

TEST_F(BtreeNodePageTestFixture, AddingMultipleCellsWorks) {
    size_t MX_SZ = (cts::PG_SZ - 500) / (cts::STR_SZ + cts::STR_SZ + sizeof(int) + cts::STR_SZ);
    size_t degree = (MX_SZ + 1) / 2;
    BtreeNodePage<vector<variants>> node(degree, 5, true, false, defaultPageID);

    vector<variants> expectedKeys;
    vector<vector<variants>> expectedValues;
    vector<size_t> expectedChildren;

    node.getChildren().push_back(499);
    expectedChildren.push_back(499);

    for (int i = 0; i < node.getMaxKeys(); i++) {
        variants cellKey = "Cell #" + std::to_string(i);
        vector<variants> tuple = {"val1 #" + std::to_string(i), "val2 #" + std::to_string(i + 100), i + 200};
        node.getCells().push_back({cellKey, tuple});
        expectedKeys.push_back(cellKey);
        expectedValues.push_back(tuple);
        node.getChildren().push_back(i + 500);
        expectedChildren.push_back(i + 500);
    }

    BtreeNodePage<vector<variants>> node2 = roundTrip(node);

    for (size_t i = 0; i < node2.getCells().size(); i++) {
        ASSERT_EQ(node2.getCells()[i].key, expectedKeys[i]);
        ASSERT_EQ(node2.getCells()[i].value, expectedValues[i]);
        ASSERT_EQ(node2.getChildren()[i], expectedChildren[i]);
    }
}

TEST_F(BtreeNodePageTestFixture, RowPtrBtreeNodeWorks) {
    BtreeNodePage<kndb_types::RowPtr> node(6, 5, true, false, defaultPageID);
    node.getCells().push_back({3, {4, 10}});
    node.getChildren().push_back(100);
    node.getChildren().push_back(101);

    BtreeNodePage<kndb_types::RowPtr> node2 = roundTrip(node);
    ASSERT_EQ(2, node2.getChildren().size());
    ASSERT_EQ(1, node2.getCells().size());

    ASSERT_EQ(variants(3), node2.getCells()[0].key);
    ASSERT_EQ(4, node2.getCells()[0].value.pageID);
    ASSERT_EQ(10, node2.getCells()[0].value.cellID);
    ASSERT_EQ(100, node2.getChildren()[0]);
    ASSERT_EQ(101, node2.getChildren()[1]);
}

TEST_F(BtreeNodePageTestFixture, InsertingManyRowPtrWorks) {
    size_t MX_SZ = cts::PG_SZ / 30;
    size_t degree = (MX_SZ + 1) / 2;
    BtreeNodePage<kndb_types::RowPtr> node(degree, 5, false, false, defaultPageID);
    node.getChildren().push_back(1111);

    vector<BtreeNodePage<kndb_types::RowPtr>::cell> expectedCells;
    vector<size_t> expectedChildren = {1111};

    for (int i = 0; i < static_cast<int>(MX_SZ); i++) {
        kndb_types::RowPtr ptr{};
        ptr.cellID = i * 2;
        ptr.pageID = i * 3;
        node.getCells().push_back({i, ptr});
        expectedCells.push_back({i, ptr});
        node.getChildren().push_back(i * 4);
        expectedChildren.push_back(i * 4);
    }

    BtreeNodePage<kndb_types::RowPtr> node2 = roundTrip(node);

    ASSERT_EQ(node2.getCells().size(), expectedCells.size());
    ASSERT_EQ(node2.getChildren(), expectedChildren);

    for (size_t i = 0; i < expectedCells.size(); i++) {
        ASSERT_EQ(node2.getCells()[i].key, expectedCells[i].key);
        ASSERT_EQ(node2.getCells()[i].value.cellID, expectedCells[i].value.cellID);
        ASSERT_EQ(node2.getCells()[i].value.pageID, expectedCells[i].value.pageID);
    }
}

TEST_F(BtreeNodePageTestFixture, InvalidDeserializationThrows) {
    ByteVec invalidBytes(10, static_cast<std::byte>(0)); // Intentionally too short
    ASSERT_THROW(BtreeNodePage<vector<variants>> invalidNode(invalidBytes, defaultPageID), std::runtime_error);
}
