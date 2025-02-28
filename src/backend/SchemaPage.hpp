//
// Created by Kylan Chen on 12/20/24.
//

#ifndef KNDB_SCHEMAPAGE_HPP
#define KNDB_SCHEMAPAGE_HPP

#include <vector>

#include "Page.hpp"
#include "unordered_map"

using variants = std::variant<int, char, bool, std::string>;
using std::byte;
using std::vector;
using ByteVec = std::vector<std::byte>;
using ByteVecPtr = std::unique_ptr<ByteVec>;
using std::string;
using std::unordered_map;

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
     * @brief Gets the list of types in a specific table.
     * @param table_name The name of the table (case insensitive).
     * @return A list of variants representing the types.
     */
    vector<variants> getTableTypes(string table_name);

    /**
     * @brief Gets the total number of tables in the schema.
     * @return The number of tables.
     */
    size_t getNumTables();

    /**
     * @brief Gets list of all tables in schema.
     * @return Map of table names to pageID of table metadata page.
     */
    unordered_map<string, size_t> getTables();

    /**
     * @brief Creates a new table in the schema.
     * @param name The name of the table.
     * @param types The types of the table columns.
     * @param pageID The page number of the table.
     */
    void addTable(string name, vector<variants> types, size_t pageID);

    /**
     * @brief Removes a table from the schema.
     * @param name The name of the table to remove.
     */
    void removeTable(string name);

    void to_bytes(ByteVec &vec) override;

private:
    struct table_descriptor {
        string name;
        vector<variants> types;
        size_t pageID;
    };

    vector<table_descriptor> m_tables;
};


#endif //KNDB_SCHEMAPAGE_HPP
