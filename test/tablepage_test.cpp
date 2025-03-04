//
// Created by Kylan Chen on 3/3/25.
//

#include <gtest/gtest.h>
#include "TablePage.hpp"
#include "utility.hpp"

struct TablePageTest : testing::Test {
    std::unique_ptr<std::vector<std::byte>> vec;
    std::unique_ptr<TablePage> table;

    TablePageTest() {
        vec = std::make_unique<std::vector<std::byte>>(cts::PG_SZ);
        size_t pageType = get_page_type_id<TablePage>();
        memcpy(vec->data(), &pageType, sizeof(size_t));
    }

    void SetUp() override {
        table = std::make_unique<TablePage>(*vec, 1);
    }
};

TEST_F(TablePageTest, TableInitializationIsCorrect) {
    vector<variants> types = {string(), int(), double(), double(), float()};
    table->init(types, 5);
    ASSERT_EQ(table->getNumTuples(), 0);
    ASSERT_EQ(table->getBtreePageID(), 5);
    ASSERT_EQ(table->getTypes(), types);
}

TEST_F(TablePageTest, SimpleAddAndSubtractTuplesWorks) {
    vector<variants> types = {string(), int(), double(), double(), float()};
    table->init(types, 5);
    ASSERT_EQ(table->getNumTuples(), 0);
    table->addTuple();
    ASSERT_EQ(table->getNumTuples(), 1);
    table->removeTuple();
    ASSERT_EQ(table->getNumTuples(), 0);
}

TEST_F(TablePageTest, SimpleSerializationAndDeserializationWorks) {
    vector<variants> types = {string(), int(), double(), double(), float()};
    table->init(types, 5);
    ByteVec buffer(cts::PG_SZ);
    table->toBytes(buffer);
    TablePage serialized(buffer, 1);
    ASSERT_THROW(serialized.init(types, 5), std::runtime_error);
    ASSERT_THROW(serialized.init({double(), float(), int()}, 3), std::runtime_error);
    ASSERT_EQ(serialized.getNumTuples(), 0);
    ASSERT_EQ(serialized.getBtreePageID(), 5);
    ASSERT_EQ(serialized.getTypes(), types);
}

TEST_F(TablePageTest, SerializationWorksWithLargeNumberOfAddAndRemove) {
    vector<variants> types = {string(), int(), double(), double(), float()};
    table->init(types, 5);
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

TEST_F(TablePageTest, GettersThrowIfNotInitialized) {
    ASSERT_THROW(table->addTuple(), std::runtime_error);
    ASSERT_THROW(table->removeTuple(), std::runtime_error);
    ASSERT_THROW(table->getNumTuples(), std::runtime_error);
    ASSERT_THROW(table->getBtreePageID(), std::runtime_error);
    ByteVec temp(cts::PG_SZ);
    ASSERT_THROW(table->toBytes(temp), std::runtime_error);
}

TEST_F(TablePageTest, ManyTypesWorks) {
    vector<variants> types;
    vector<variants> valid_types = {int(), double(), string(), float(), char(), bool()};
    for (int i = 0; i < 200; i++) {
        types.push_back(valid_types[i % 6]);
    }
    table->init(types, 5);
    ASSERT_EQ(table->getTypes(), types);
}

TEST_F(TablePageTest, InitThrowsIfAlreadyInitialized) {
    table->init({int(), double(), int()}, 42);
    ASSERT_THROW(table->init({int(), double(), int()}, 43), std::runtime_error);
}

TEST_F(TablePageTest, EmptyTypesThrowsException) {
    ASSERT_THROW(table->init({}, 42), std::invalid_argument);
}
