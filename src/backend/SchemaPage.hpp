//
// Created by Kylan Chen on 12/20/24.
//

#ifndef KNDB_SCHEMAPAGE_HPP
#define KNDB_SCHEMAPAGE_HPP

#include <unordered_map>

#include "Page.hpp"
#include "kndb_types.hpp"

namespace backend {

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
    SchemaPage(std::span<const byte> bytes, pgid_t pageID);

    /**
     * @brief Creates a new empty SchemaPage.
     * @param pageID
     */
    SchemaPage(pgid_t pageID);

    /**
     * @brief Gets the total number of tables in the schema.
     * @return The number of tables.
     */
    u8 getNumTables() const;

    /**
     * @brief Gets list of all tables in schema.
     * @return Map of table names to pageID of table metadata page.
     */
    const std::unordered_map<string, pgid_t>& getTables();

    /**
     * @brief Creates a new table in the schema.
     * @param name The name of the table.
     * @param pageID The page number of the table.
     */
    void addTable(const string& name, pgid_t pageID);

    /**
     * @brief Removes a table from the schema.
     * @param name The name of the table to remove.
     */
    void removeTable(const string& name);

    void toBytes(std::span<byte> buffer) override;

private:
    // calculates the amount of free space (in bytes) that the page has left
    offset_t freeSpace() const;

    std::unordered_map<string, pgid_t> m_tables;
};

} // namespace backend

#endif //KNDB_SCHEMAPAGE_HPP
