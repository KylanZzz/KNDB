//
// Created by Kylan Chen on 12/20/24.
//

#ifndef KNDB_SCHEMAPAGE_HPP
#define KNDB_SCHEMAPAGE_HPP

#include "Page.hpp"
#include "unordered_map"
#include "kndb_types.hpp"

using namespace kndb;

/**
 * @class SchemaPage
 * @brief Represents a page that contains metadata about tables in a schema.
 *
 * This class contains table information such as names, types, and page numbers.
 */
class SchemaPage : public Page {
public:

    /**
     * @brief Loads a SchemaPage from a byte vector.
     * @param bytes The byte vector containing schema data.
     * @param pageID The on-disk page number of this schema page.
     */
    SchemaPage(std::span<const byte> bytes, u32 pageID);

    /**
     * @brief Creates a new empty SchemaPage.
     * @param pageID
     */
    SchemaPage(u32 pageID);

    /**
     * @brief Gets the total number of tables in the schema.
     * @return The number of tables.
     */
    u8 getNumTables() const;

    /**
     * @brief Gets list of all tables in schema.
     * @return Map of table names to pageID of table metadata page.
     */
    std::unordered_map<string, u32> getTables();

    /**
     * @brief Creates a new table in the schema.
     * @param name The name of the table.
     * @param pageID The page number of the table.
     */
    void addTable(string name, u32 pageID);

    /**
     * @brief Removes a table from the schema.
     * @param name The name of the table to remove.
     */
    void removeTable(const string& name);

    void toBytes(std::span<byte> buffer) override;

private:
    struct table_descriptor {
        string name;
        u32 pageID;
    };

    // calculates the amount of free space (in bytes) that the page has left
    u16 freeSpace();

    Vec<table_descriptor> m_tables;
};


#endif //KNDB_SCHEMAPAGE_HPP
