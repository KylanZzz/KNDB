//
// Created by Kylan Chen on 2/28/25.
//

#include <gtest/gtest.h>

#include "SchemaPage.hpp"
#include "kndb_types.hpp"
#include "constants.hpp"
#include "utility.hpp"

using namespace kndb_types;

struct SchemaPageTest : testing::Test {
    std::unique_ptr<std::vector<std::byte>> vec;
    SchemaPageTest() {
        vec = std::make_unique<std::vector<std::byte>>();
        vec->resize(cts::PG_SZ);
        size_t a = cts::pg_type_id::SCHEMA_PAGE;
        memcpy(vec->data(), &a, sizeof(size_t));
    }
};

TEST_F(SchemaPageTest, NewlyCreatedSchemaHasNoTables) {
    SchemaPage schema(1);
    ASSERT_EQ(schema.getNumTables(), 0);
    ASSERT_TRUE(schema.getTables().empty());
}

TEST_F(SchemaPageTest, AddTableIncreasesTableCount) {
    SchemaPage schema(1);
    schema.addTable("Users", 5);
    ASSERT_EQ(schema.getNumTables(), 1);
    ASSERT_EQ(schema.getTables().at("Users"), 5);
}

TEST_F(SchemaPageTest, RemoveTableDecreasesTableCount) {
    SchemaPage schema(1);
    schema.addTable("Users", 5);
    schema.removeTable("Users");
    ASSERT_EQ(schema.getNumTables(), 0);
    ASSERT_TRUE(schema.getTables().empty());
}

TEST_F(SchemaPageTest, SerializationPreservesTables) {
    SchemaPage original(1);
    original.addTable("Users", 5);
    original.addTable("Orders", 10);

    ByteVec serialized(cts::PG_SZ);
    original.toBytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 2);
    ASSERT_EQ(deserialized.getTables().at("Users"), 5);
    ASSERT_EQ(deserialized.getTables().at("Orders"), 10);
}

TEST_F(SchemaPageTest, AddRemoveTablesWithSerialization) {
    SchemaPage original(1);
    original.addTable("Users", 5);
    original.addTable("Orders", 10);
    original.removeTable("Users");

    ByteVec serialized(cts::PG_SZ);
    original.toBytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 1);
    ASSERT_EQ(deserialized.getTables().size(), 1);
    ASSERT_EQ(deserialized.getTables().at("Orders"), 10);
    ASSERT_THROW(deserialized.getTables().at("Users"), std::out_of_range);
}

TEST_F(SchemaPageTest, SerializeAfterAddingAndRemovingSameTable) {
    SchemaPage original(1);
    original.addTable("TempTable", 50);
    original.removeTable("TempTable");
    original.addTable("TempTable", 60);

    ByteVec serialized(cts::PG_SZ);
    original.toBytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 1);
    ASSERT_EQ(deserialized.getTables().at("TempTable"), 60);
}

TEST_F(SchemaPageTest, AddTableThrowsIfNameExists) {
    SchemaPage schema(1);
    schema.addTable("Products", 15);
    ASSERT_THROW(schema.addTable("Products", 20), std::invalid_argument);
}

TEST_F(SchemaPageTest, AddTableWithEmptyNameThrows) {
    SchemaPage schema(1);
    ASSERT_THROW(schema.addTable("", 5), std::invalid_argument);
}

TEST_F(SchemaPageTest, AddTableWithMaxAndExceedingNameLength) {
    SchemaPage schema(1);
    std::string maxLengthName(31, 'A'); // 31 characters, should work
    std::string tooLongName(32, 'B');   // 32 characters, should throw

    schema.addTable(maxLengthName, 5);
    ASSERT_EQ(schema.getTables().size(), 1);

    ASSERT_THROW(schema.addTable(tooLongName, 6), std::invalid_argument);

    ByteVec serialized(cts::PG_SZ);
    schema.toBytes(serialized);

    SchemaPage schema2(serialized, 1);
    ASSERT_EQ(schema2.getTables().at(maxLengthName), 5);
}

TEST_F(SchemaPageTest, AddingTableBeyondCapacityThrows) {
    SchemaPage schema(1);
    size_t used_space = 16; // Initial schema space
    size_t max_space = cts::PG_SZ;

    size_t pageID = 5;
    while (used_space + 32 + 8 <= max_space) {
        schema.addTable("Table" + std::to_string(pageID), pageID);
        pageID++;
        used_space += 32 + 8; // 32 (name) + 8 (pageID)
    }

    ASSERT_THROW(schema.addTable("OverflowTable", pageID), std::runtime_error);

    ByteVec bytes(cts::PG_SZ);
    schema.toBytes(bytes);

    SchemaPage s2(bytes, 1);
    ASSERT_EQ(s2.getTables(), schema.getTables());
    ASSERT_EQ(schema.getNumTables(), s2.getNumTables());
}

TEST_F(SchemaPageTest, SerializationPreservesEmptySchema) {
    SchemaPage original(1);
    ByteVec serialized(cts::PG_SZ);
    original.toBytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 0);
    ASSERT_TRUE(deserialized.getTables().empty());
}

TEST_F(SchemaPageTest, SerializationWithMultipleOperations) {
    SchemaPage original(1);
    original.addTable("Users", 5);
    original.addTable("Orders", 10);
    original.removeTable("Users");
    original.addTable("Products", 15);
    original.removeTable("Orders");
    original.addTable("Customers", 20);

    ByteVec serialized(cts::PG_SZ);
    original.toBytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 2);
    ASSERT_EQ(deserialized.getTables().size(), 2);
    ASSERT_EQ(deserialized.getTables().at("Products"), 15);
    ASSERT_EQ(deserialized.getTables().at("Customers"), 20);
    ASSERT_THROW(deserialized.getTables().at("Users"), std::out_of_range);
    ASSERT_THROW(deserialized.getTables().at("Orders"), std::out_of_range);
}