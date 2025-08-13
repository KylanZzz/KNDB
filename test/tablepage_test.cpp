//
// Created by Kylan Chen on 3/3/25.
//

#include <Btree.hpp>
#include <gtest/gtest.h>
#include "TablePage.hpp"
#include "utility.hpp"
#include "kndb_types.hpp"

using namespace backend;

struct TablePageTest : testing::Test {
    std::unique_ptr<TablePage> table;
    Vec<Vari> types = {string(), int(), double(), double(), float()};

    void SetUp() override {
        table = std::make_unique<TablePage>(types, 5, 3);
    }
};

TEST_F(TablePageTest, TableInitializationIsCorrect) {
    ASSERT_EQ(table->getNumTuples(), 0);
    ASSERT_EQ(table->getBtreePageID(), 5);
    ASSERT_EQ(table->getTypes(), types);
}

TEST_F(TablePageTest, SimpleAddAndSubtractTuplesWorks) {
    ASSERT_EQ(table->getNumTuples(), 0);
    table->addTuple();
    ASSERT_EQ(table->getNumTuples(), 1);
    table->removeTuple();
    ASSERT_EQ(table->getNumTuples(), 0);
}

TEST_F(TablePageTest, SimpleSerializationAndDeserializationWorks) {
    Vec<byte> buffer(cts::PG_SZ);
    table->toBytes(buffer);
    TablePage serialized(buffer, 1);
    ASSERT_EQ(serialized.getNumTuples(), 0);
    ASSERT_EQ(serialized.getBtreePageID(), 5);
    ASSERT_EQ(serialized.getTypes(), types);
}

TEST_F(TablePageTest, SerializationWorksWithLargeNumberOfAddAndRemove) {
    for (int i = 0; i < 50000; i++)
        table->addTuple();

    Vec<byte> buffer(cts::PG_SZ);
    table->toBytes(buffer);

    TablePage serialized(buffer, 1);
    ASSERT_EQ(serialized.getNumTuples(), 50000);
    for (int i = 0; i < 40000; i++)
        serialized.removeTuple();

    Vec<byte> buffer2(cts::PG_SZ);
    serialized.toBytes(buffer2);

    TablePage serialized2(buffer2, 1);

    ASSERT_EQ(serialized2.getNumTuples(), 10000);
}

TEST_F(TablePageTest, ManyTypesWorks) {
    Vec<Vari> types;
    Vec<Vari> valid_types = {int(), double(), string(), float(), char(), bool()};
    for (int i = 0; i < 200; i++) {
        types.push_back(valid_types[i % 6]);
    }
    TablePage manyTypesTable(types, 4, 10);
    ASSERT_EQ(manyTypesTable.getTypes(), types);
}

TEST_F(TablePageTest, ConstructorTerminatesOnEmptyTypes) {
    Vec<Vari> emptyTypes;
    ASSERT_DEATH(TablePage(emptyTypes, 5, 3), "");
}

TEST_F(TablePageTest, ConstructorTerminatesOnTooManyTypes) {
    Vec<Vari> tooManyTypes;
    // Create a types vector that would exceed reasonable page capacity
    for (int i = 0; i < 10000; i++) {
        tooManyTypes.push_back(int());
    }
    ASSERT_DEATH(TablePage(tooManyTypes, 5, 3), "");
}

TEST_F(TablePageTest, RemoveTupleOnEmptyTableCausesDeath) {
    ASSERT_DEATH(table->removeTuple(), "");
}

TEST_F(TablePageTest, RemoveTupleBelowZeroCausesDeath) {
    table->addTuple();
    table->removeTuple();
    ASSERT_DEATH(table->removeTuple(), "");
}

TEST_F(TablePageTest, SetBtreePageIDWorks) {
    ASSERT_EQ(table->getBtreePageID(), 5);
    table->setBtreePageID(42);
    ASSERT_EQ(table->getBtreePageID(), 42);
}

TEST_F(TablePageTest, SerializationWithNonZeroTupleCount) {
    table->addTuple();
    table->addTuple();
    
    Vec<byte> buffer(cts::PG_SZ);
    table->toBytes(buffer);
    
    TablePage serialized(buffer, 1);
    ASSERT_EQ(serialized.getNumTuples(), 2);
    ASSERT_EQ(serialized.getBtreePageID(), 5);
    ASSERT_EQ(serialized.getTypes(), types);
}

TEST_F(TablePageTest, SingleTupleEdgeCase) {
    table->addTuple();
    ASSERT_EQ(table->getNumTuples(), 1);
    
    Vec<byte> buffer(cts::PG_SZ);
    table->toBytes(buffer);
    
    TablePage serialized(buffer, 1);
    ASSERT_EQ(serialized.getNumTuples(), 1);
}

TEST_F(TablePageTest, ComplexAddRemoveCycles) {
    // Add 10, remove 5, add 3, remove 2, add 1
    for (int i = 0; i < 10; i++) table->addTuple();
    for (int i = 0; i < 5; i++) table->removeTuple();
    for (int i = 0; i < 3; i++) table->addTuple();
    for (int i = 0; i < 2; i++) table->removeTuple();
    table->addTuple();
    
    ASSERT_EQ(table->getNumTuples(), 7);
    
    // Verify serialization preserves this state
    Vec<byte> buffer(cts::PG_SZ);
    table->toBytes(buffer);
    
    TablePage serialized(buffer, 1);
    ASSERT_EQ(serialized.getNumTuples(), 7);
}

TEST_F(TablePageTest, MaximumTupleCount) {
    // Test with a very large number to check for integer overflow
    for (int i = 0; i < 1000000; i++) {
        table->addTuple();
    }
    
    ASSERT_EQ(table->getNumTuples(), 1000000);
    
    // Verify serialization works with large counts
    Vec<byte> buffer(cts::PG_SZ);
    ASSERT_NO_THROW(table->toBytes(buffer));
}

TEST_F(TablePageTest, SingleTypeTable) {
    Vec<Vari> singleType = {string()};
    TablePage singleTypeTable(singleType, 5, 4);
    
    ASSERT_EQ(singleTypeTable.getTypes().size(), 1);
    ASSERT_EQ(singleTypeTable.getTypes(), singleType);
}
