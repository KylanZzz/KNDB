#include <gtest/gtest.h>
#include <stdexcept>

#include "Btree.hpp"
#include "BtreeNodePage.hpp"
#include "constants.hpp"

using namespace backend;

class BtreeNodePageTestFixture : public ::testing::Test {
protected:
    uint16_t defaultPageID = 3;

    // Helper: Serializes a node and returns a new node constructed from the bytes.
    template <typename T>
    BtreeNodePage<T> roundTrip(BtreeNodePage<T>& node) {
        Vec<byte> bytes(cts::PG_SZ);
        node.toBytes(bytes);
        return BtreeNodePage<T>(bytes, defaultPageID);
    }
};

TEST_F(BtreeNodePageTestFixture, PageInitializationIsCorrect) {
    BtreeNodePage<Vec<Vari>> node(6, 5, true, false, defaultPageID);

    ASSERT_TRUE(node.children().empty());
    ASSERT_TRUE(node.cells().empty());
    ASSERT_FALSE(node.leaf());
    ASSERT_TRUE(node.root());
    ASSERT_EQ(5, node.parent());
}

TEST_F(BtreeNodePageTestFixture, PageInitializationAndSerialization) {
    BtreeNodePage<Vec<Vari>> node(6, 5, true, false, defaultPageID);
    BtreeNodePage<Vec<Vari>> node2 = roundTrip(node);

    ASSERT_TRUE(node2.children().empty());
    ASSERT_TRUE(node2.cells().empty());
    ASSERT_FALSE(node2.leaf());
    ASSERT_TRUE(node2.root());
    ASSERT_EQ(6 - 1, node2.minKeys());
    ASSERT_EQ(5, node2.parent());
    ASSERT_EQ(2 * 6 - 1, node2.maxKeys());
}

TEST_F(BtreeNodePageTestFixture, AddingSingleCellWorks) {
    BtreeNodePage<Vec<Vari>> node(6, 5, true, false, defaultPageID);
    node.cells().push_back({33, {string("Kylan"), 3.1144, 10}});
    node.children().push_back(100);
    node.children().push_back(101);

    BtreeNodePage<Vec<Vari>> node2 = roundTrip(node);
    ASSERT_EQ(2, node2.children().size());
    ASSERT_EQ(1, node2.cells().size());

    ASSERT_EQ(Vari(33), node2.cells()[0].key);
    ASSERT_EQ(Vari("Kylan"), node2.cells()[0].value[0]);
    ASSERT_EQ(Vari(3.1144), node2.cells()[0].value[1]);
    ASSERT_EQ(Vari(10), node2.cells()[0].value[2]);
    ASSERT_EQ(100, node2.children()[0]);
    ASSERT_EQ(101, node2.children()[1]);
}

TEST_F(BtreeNodePageTestFixture, AddingMultipleCellsWorks) {
    uint16_t MX_SZ = (cts::PG_SZ - 500) / (cts::STR_SZ + cts::STR_SZ + sizeof(int) + cts::STR_SZ);
    uint16_t degree = (MX_SZ + 1) / 2;
    BtreeNodePage<Vec<Vari>> node(degree, 5, true, false, defaultPageID);

    Vec<Vari> expectedKeys;
    Vec<Vec<Vari>> expectedValues;
    Vec<uint32_t> expectedChildren;

    node.children().push_back(499);
    expectedChildren.push_back(499);

    for (int i = 0; i < node.maxKeys(); i++) {
        Vari cellKey = "Cell #" + std::to_string(i);
        Vec<Vari> tuple = {"val1 #" + std::to_string(i), "val2 #" + std::to_string(i + 100), i + 200};
        node.cells().push_back({cellKey, tuple});
        expectedKeys.push_back(cellKey);
        expectedValues.push_back(tuple);
        node.children().push_back(i + 500);
        expectedChildren.push_back(i + 500);
    }

    BtreeNodePage<Vec<Vari>> node2 = roundTrip(node);

    for (int i = 0; i < node2.cells().size(); i++) {
        ASSERT_EQ(node2.cells()[i].key, expectedKeys[i]);
        ASSERT_EQ(node2.cells()[i].value, expectedValues[i]);
        ASSERT_EQ(node2.children()[i], expectedChildren[i]);
    }
}

TEST_F(BtreeNodePageTestFixture, RowPtrBtreeNodeWorks) {
    BtreeNodePage<RowPos> node(6, 5, true, false, defaultPageID);
    node.cells().push_back({3, {4, 10}});
    node.children().push_back(100);
    node.children().push_back(101);

    BtreeNodePage<RowPos> node2 = roundTrip(node);
    ASSERT_EQ(2, node2.children().size());
    ASSERT_EQ(1, node2.cells().size());

    ASSERT_EQ(Vari(3), node2.cells()[0].key);
    ASSERT_EQ(4, node2.cells()[0].value.pageID);
    ASSERT_EQ(10, node2.cells()[0].value.cellID);
    ASSERT_EQ(100, node2.children()[0]);
    ASSERT_EQ(101, node2.children()[1]);
}

TEST_F(BtreeNodePageTestFixture, InsertingManyRowPtrWorks) {
    uint16_t MX_SZ = cts::PG_SZ / 30;
    uint16_t degree = (MX_SZ + 1) / 2;
    BtreeNodePage<RowPos> node(degree, 5, false, false, defaultPageID);
    node.children().push_back(1111);

    Vec<BtreeNodePage<RowPos>::cell> expectedCells;
    Vec<uint32_t> expectedChildren = {1111};

    for (int i = 0; i < static_cast<int>(MX_SZ); i++) {
        RowPos ptr{};
        ptr.cellID = i * 2;
        ptr.pageID = i * 3;
        node.cells().push_back({i, ptr});
        expectedCells.push_back({i, ptr});
        node.children().push_back(i * 4);
        expectedChildren.push_back(i * 4);
    }

    BtreeNodePage<RowPos> node2 = roundTrip(node);

    ASSERT_EQ(node2.cells().size(), expectedCells.size());
    ASSERT_EQ(node2.children(), expectedChildren);

    for (int i = 0; i < expectedCells.size(); i++) {
        ASSERT_EQ(node2.cells()[i].key, expectedCells[i].key);
        ASSERT_EQ(node2.cells()[i].value.cellID, expectedCells[i].value.cellID);
        ASSERT_EQ(node2.cells()[i].value.pageID, expectedCells[i].value.pageID);
    }
}

TEST_F(BtreeNodePageTestFixture, InvalidDeserializationThrows) {
    Vec<byte> invalidBytes(10, static_cast<byte>(0)); // Intentionally too short
    ASSERT_THROW(BtreeNodePage<Vec<Vari>> invalidNode(invalidBytes, defaultPageID), std::runtime_error);
}
