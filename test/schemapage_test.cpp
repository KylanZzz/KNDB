//
// Created by Kylan Chen on 2/28/25.
//

#include <gtest/gtest.h>

#include "SchemaPage.hpp"
#include "utility.hpp"

TEST(SchemaPageTest, NewlyCreatedSchemaHasNoTables) {
    SchemaPage schema(1);
    ASSERT_EQ(schema.getNumTables(), 0);
    ASSERT_TRUE(schema.getTables().empty());
}

TEST(SchemaPageTest, AddTableIncreasesTableCount) {
    SchemaPage schema(1);
    schema.addTable("Users", {int(), std::string()}, 5);
    ASSERT_EQ(schema.getNumTables(), 1);
    ASSERT_EQ(schema.getTables().at("Users"), 5);
}

TEST(SchemaPageTest, RemoveTableDecreasesTableCount) {
    SchemaPage schema(1);
    schema.addTable("Users", {int(), std::string()}, 5);
    schema.removeTable("Users");
    ASSERT_EQ(schema.getNumTables(), 0);
    ASSERT_TRUE(schema.getTables().empty());
}

TEST(SchemaPageTest, SerializationPreservesTables) {
    SchemaPage original(1);
    original.addTable("Users", {int(), std::string()}, 5);
    original.addTable("Orders", {int(), bool()}, 10);

    ByteVec serialized(cts::PG_SZ);
    original.to_bytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 2);
    ASSERT_EQ(deserialized.getTables().at("Users"), 5);
    ASSERT_EQ(deserialized.getTables().at("Orders"), 10);
}

TEST(SchemaPageTest, AddRemoveTablesWithSerialization) {
    SchemaPage original(1);
    original.addTable("Users", {int(), std::string()}, 5);
    original.addTable("Orders", {int(), bool()}, 10);
    original.removeTable("Users");

    ByteVec serialized(cts::PG_SZ);
    original.to_bytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 1);
    ASSERT_EQ(deserialized.getTables().size(), 1);
    ASSERT_EQ(deserialized.getTables().at("Orders"), 10);
    ASSERT_THROW(deserialized.getTables().at("Users"), std::out_of_range);
}


TEST(SchemaPageTest, SerializeAfterAddingAndRemovingSameTable) {
    SchemaPage original(1);
    original.addTable("TempTable", {int(), bool()}, 50);
    original.removeTable("TempTable");
    original.addTable("TempTable", {std::string(), char()}, 60);

    ByteVec serialized(cts::PG_SZ);
    original.to_bytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 1);
    ASSERT_EQ(deserialized.getTables().at("TempTable"), 60);
    ASSERT_EQ(deserialized.getTableTypes("TempTable").size(), 2);
    ASSERT_TRUE(std::holds_alternative<std::string>(deserialized.getTableTypes("TempTable")[0]));
    ASSERT_TRUE(std::holds_alternative<char>(deserialized.getTableTypes("TempTable")[1]));
}

TEST(SchemaPageTest, AddTableThrowsIfNameExists) {
    SchemaPage schema(1);
    schema.addTable("Products", {std::string(), char()}, 15);
    ASSERT_THROW(schema.addTable("Products", {int(), bool()}, 20), std::invalid_argument);
}

TEST(SchemaPageTest, AddTableWithEmptyNameThrows) {
    SchemaPage schema(1);
    ASSERT_THROW(schema.addTable("", {int(), std::string()}, 5), std::invalid_argument);
}

TEST(SchemaPageTest, AddTableWithMaxAndExceedingNameLength) {
    SchemaPage schema(1);
    std::string maxLengthName(31, 'A'); // 31 characters, should work
    std::string tooLongName(32, 'B');   // 32 characters, should throw

    schema.addTable(maxLengthName, {int(), std::string()}, 5);
    ASSERT_EQ(schema.getTables().size(), 1);

    ASSERT_THROW(schema.addTable(tooLongName, {int(), std::string()}, 6), std::invalid_argument);
}

TEST(SchemaPageTest, AddingTableBeyondCapacityThrows) {
    SchemaPage schema(1);
    size_t used_space = 16; // Initial schema space
    size_t max_space = cts::PG_SZ;

    size_t pageID = 5;
    while (used_space + 32 + 8 + (3 * 8) <= max_space) {
        schema.addTable("Table" + std::to_string(pageID), {int(), bool(), std::string()}, pageID);
        used_space += 32 + 8 + (3 * 8); // 32 (name) + 8 (pageID) + 8 bytes per type
        ++pageID;
    }

    ASSERT_THROW(schema.addTable("OverflowTable", {int(), char()}, pageID), std::runtime_error);
}

TEST(SchemaPageTest, SerializationPreservesEmptySchema) {
    SchemaPage original(1);
    ByteVec serialized(cts::PG_SZ);
    original.to_bytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 0);
    ASSERT_TRUE(deserialized.getTables().empty());
}

TEST(SchemaPageTest, SerializationWithMultipleOperations) {
    SchemaPage original(1);
    original.addTable("Users", {int(), std::string()}, 5);
    original.addTable("Orders", {int(), bool()}, 10);
    original.removeTable("Users");
    original.addTable("Products", {char(), bool()}, 15);
    original.removeTable("Orders");
    original.addTable("Customers", {int(), std::string()}, 20);

    ByteVec serialized(cts::PG_SZ);
    original.to_bytes(serialized);

    SchemaPage deserialized(serialized, 1);
    ASSERT_EQ(deserialized.getNumTables(), 2);
    ASSERT_EQ(deserialized.getTables().size(), 2);
    ASSERT_EQ(deserialized.getTables().at("Products"), 15);
    ASSERT_EQ(deserialized.getTables().at("Customers"), 20);
    ASSERT_THROW(deserialized.getTables().at("Users"), std::out_of_range);
    ASSERT_THROW(deserialized.getTables().at("Orders"), std::out_of_range);
}