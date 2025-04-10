//
// Created by Kylan Chen on 2/28/25.
//

#include <gtest/gtest.h>

#include "SchemaPage.hpp"
#include "kndb_types.hpp"
#include "constants.hpp"

using namespace backend;

struct SchemaPageTest : testing::Test {
    std::unique_ptr<Vec<byte>> vec;
    SchemaPageTest() {
        vec = std::make_unique<Vec<byte>>();
        vec->resize(cts::PG_SZ);
        u8 a = cts::pg_type_id::SCHEMA_PAGE;
        memcpy(vec->data(), &a, sizeof(u8));
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

    Vec<byte> serialized(cts::PG_SZ);
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

    Vec<byte> serialized(cts::PG_SZ);
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

    Vec<byte> serialized(cts::PG_SZ);
    original.toBytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 1);
    ASSERT_EQ(deserialized.getTables().at("TempTable"), 60);
}

TEST_F(SchemaPageTest, AddTableWithEmptyNameExits) {
    SchemaPage schema(1);
    ASSERT_DEATH(schema.addTable("", 5), "");
}

TEST_F(SchemaPageTest, AddTableWithMaxAndExceedingNameLength) {
    SchemaPage schema(1);
    std::string maxLengthName(cts::MAX_STR_SZ - 1, 'A'); // 31 characters, should work
    std::string tooLongName(cts::MAX_STR_SZ, 'B');   // 32 characters, should throw

    schema.addTable(maxLengthName, 5);
    ASSERT_EQ(schema.getTables().size(), 1);

    ASSERT_DEATH(schema.addTable(tooLongName, 6), "");

    Vec<byte> serialized(cts::PG_SZ);
    schema.toBytes(serialized);

    SchemaPage schema2(serialized, 1);
    ASSERT_EQ(schema2.getTables().at(maxLengthName), 5);
}

TEST_F(SchemaPageTest, AddingTableBeyondCapacityExits) {
    SchemaPage schema(1);
    u32 used_space = 2; // Initial schema space
    u32 max_space = cts::PG_SZ;

    u32 pageID = 5;
    while (used_space + cts::MAX_STR_SZ + sizeof(u32) <= max_space) {
        schema.addTable("Table" + std::to_string(pageID), pageID);
        pageID++;
        used_space += cts::MAX_STR_SZ + sizeof(u32); // 32 (name) + 4 (pageID)
    }

    ASSERT_DEATH(schema.addTable("OverflowTable", pageID), "");

    Vec<byte> bytes(cts::PG_SZ);
    schema.toBytes(bytes);

    SchemaPage s2(bytes, 1);
    ASSERT_EQ(s2.getTables(), schema.getTables());
    ASSERT_EQ(schema.getNumTables(), s2.getNumTables());
}

TEST_F(SchemaPageTest, SerializationPreservesEmptySchema) {
    SchemaPage original(1);
    Vec<byte> serialized(cts::PG_SZ);
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

    Vec<byte> serialized(cts::PG_SZ);
    original.toBytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 2);
    ASSERT_EQ(deserialized.getTables().size(), 2);
    ASSERT_EQ(deserialized.getTables().at("Products"), 15);
    ASSERT_EQ(deserialized.getTables().at("Customers"), 20);
    ASSERT_THROW(deserialized.getTables().at("Users"), std::out_of_range);
    ASSERT_THROW(deserialized.getTables().at("Orders"), std::out_of_range);
}