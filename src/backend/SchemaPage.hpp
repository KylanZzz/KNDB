//
// Created by Kylan Chen on 12/20/24.
//

#ifndef KNDB_SCHEMAPAGE_HPP
#define KNDB_SCHEMAPAGE_HPP

#include <vector>

#include "Page.hpp"
#include "unordered_map"
#include "kndb_types.hpp"

using namespace kndb_types;

/**
 * @class SchemaPage
 * @brief Represents a page that contains metadata about tables in a schema.
 *
 * This class contains table information such as names, types, and page numbers.
 */
class SchemaPage : public Page {
public:

    /**
     * @brief Constructs a SchemaPage from a byte vector.
     * @param bytes The byte vector containing schema data.
     * @param pageID The on-disk page number of this schema page.
     */
    SchemaPage(ByteVec &bytes, size_t pageID);

    /**
     * @brief Gets the total number of tables in the schema.
     * @return The number of tables.
     */
    size_t getNumTables();

    /**
     * @brief Gets list of all tables in schema.
     * @return Map of table names to pageID of table metadata page.
     */
    std::unordered_map<string, size_t> getTables();

    /**
     * @brief Creates a new table in the schema.
     * @param name The name of the table.
     * @param types The types of the table columns.
     * @param pageID The page number of the table.
     */
    void addTable(string name, size_t pageID);

    /**
     * @brief Removes a table from the schema.
     * @param name The name of the table to remove.
     */
    void removeTable(string name);

    void toBytes(ByteVec &vec) override;

private:
    struct table_descriptor {
        string name;
        size_t pageID;
    };

    // calculates the amount of free space (in bytes) that the page has left
    size_t freeSpace();

    vector<table_descriptor> m_tables;
};


#endif //KNDB_SCHEMAPAGE_HPP
