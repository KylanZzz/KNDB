//
// Created by Kylan Chen on 3/3/25.
//

#include <gtest/gtest.h>
#include "TablePage.hpp"
#include "utility.hpp"

struct TablePageTest : testing::Test {
    std::unique_ptr<std::vector<std::byte>> vec;
    std::unique_ptr<TablePage> table;
    vector<variants> types = {string(), int(), double(), double(), float()};

    TablePageTest() {
        vec = std::make_unique<std::vector<std::byte>>(cts::PG_SZ);
        size_t pageType = cts::pg_type_id::TABLE_PAGE;
        memcpy(vec->data(), &pageType, sizeof(size_t));
    }

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
    ByteVec buffer(cts::PG_SZ);
    table->toBytes(buffer);
    TablePage serialized(buffer, 1);
    ASSERT_EQ(serialized.getNumTuples(), 0);
    ASSERT_EQ(serialized.getBtreePageID(), 5);
    ASSERT_EQ(serialized.getTypes(), types);
}

TEST_F(TablePageTest, SerializationWorksWithLargeNumberOfAddAndRemove) {
    for (int i = 0; i < 50000; i++)
        table->addTuple();

    ByteVec buffer(cts::PG_SZ);
    table->toBytes(buffer);

    TablePage serialized(buffer, 1);
    ASSERT_EQ(serialized.getNumTuples(), 50000);
    for (int i = 0; i < 40000; i++)
        serialized.removeTuple();

    ByteVec buffer2(cts::PG_SZ);
    serialized.toBytes(buffer2);

    TablePage serialized2(buffer2, 1);

    ASSERT_EQ(serialized2.getNumTuples(), 10000);
}

TEST_F(TablePageTest, ManyTypesWorks) {
    vector<variants> types;
    vector<variants> valid_types = {int(), double(), string(), float(), char(), bool()};
    for (int i = 0; i < 200; i++) {
        types.push_back(valid_types[i % 6]);
    }
    TablePage manyTypesTable(types, 4, 10);
    ASSERT_EQ(manyTypesTable.getTypes(), types);
}

TEST_F(TablePageTest, EmptyTypesThrowsException) {
    ASSERT_THROW(TablePage testpg({}, 5, 2), std::invalid_argument);
}
