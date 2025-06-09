//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_STORAGE_ENGINE_HPP
#define KNDB_STORAGE_ENGINE_HPP

#include "Pager.hpp"
#include "Table.hpp"
#include "kndb_types.hpp"

namespace backend {

/**
 * @class StorageEngine
 *
 * The Storage Engine is the core component responsible for managing all tables in the database.
 * It acts as the interface to the backend, handling table creation, deletion, and data operations:
 * - Creating and dropping tables.
 * - Inserting, updating, removing, and reading tuples from tables.
 * - Retrieving table metadata, including column types and tuple counts.
 */
class StorageEngine {
public:
    /**
     * Constructs the Storage Engine from a given SchemaPage.
     *
     * @param schemaPageID The page ID containing the metadata for the Storage Engine.
     * @param pgr The Pager responsible for managing disk I/O.
     */
    StorageEngine(Pager& pgr, pgid_t schemaPageID);

    /**
     * Creates a new table in the Storage Engine.
     *
     * @throws std::invalid_argument if the name is empty, too long, or already exists.
     * @param tableName The name of the table (case-sensitive).
     * @param types The list of column types for the table.
     */
    void createTable(const string &tableName, const Vec<Vari>& types);

    /**
     * Drops an existing table from the Storage Engine.
     *
     * @throws std::invalid_argument if the table does not exist.
     * @param tableName The name of the table to drop (case-sensitive).
     */
    void dropTable(const string& tableName);

    /**
     * Retrieves the names of all tables in the Storage Engine.
     *
     * @return A list containing the names of all tables.
     */
    Vec<string> getTableNames() const;

    /**
     * Retrieves the column types of a specified table.
     *
     * @throws std::invalid_argument if the table does not exist.
     * @param tableName The name of the table.
     * @return A list of column types.
     */
    Vec<Vari> getTableTypes(const string& tableName) const;

    /**
     * Retrieves the number of tuples stored in a table.
     *
     * @throws std::invalid_argument if the table does not exist.
     * @param tableName The name of the table.
     * @return The number of tuples in the table.
     */
    u64 getNumTuples(const string &tableName) const;

    /**
     * Removes a tuple from a table.
     *
     * @throws std::invalid_argument if the table does not exist.
     * @param tableName The name of the table.
     * @param key The primary key value of the tuple to remove.
     */
    void removeTuple(const string &tableName, const Vari& key) const;

    /**
     * Updates an existing tuple in a table.
     *
     * @throws std::invalid_argument if the table does not exist.
     * @param tableName The name of the table.
     * @param values The new values for the tuple.
     */
    void updateTuple(const string &tableName, const Vec<Vari>& values) const;

    /**
     * Inserts a new tuple into a table.
     *
     * @throws std::invalid_argument if the table does not exist.
     * @param tableName The name of the table.
     * @param values The values for the new tuple.
     */
    void insertTuple(const string &tableName, const Vec<Vari>& values) const;

    /**
     * Retrieves a tuple from a table based on the primary key.
     *
     * @throws std::invalid_argument if the table does not exist.
     * @param tableName The name of the table.
     * @param key The primary key value of the tuple to retrieve.
     * @return The tuple's values.
     */
    Vec<Vari> getTuple(const string &tableName, const Vari& key) const;

private:
    /**
     * Checks if two lists of column types match.
     *
     * @param vec1 The first list of types.
     * @param vec2 The second list of types.
     * @return True if both lists are identical, false otherwise.
     */
    bool sameTypes(Vec<Vari> vec1, Vec<Vari> vec2);

    Pager& m_pager;  ///< Reference to the Pager handling disk operations.
    Vec<std::unique_ptr<Table>> m_tables;  ///< List of tables managed by the Storage Engine.
    pgid_t m_schemaPageID;  ///< The hardcoded page ID where Storage Engine metadata is stored.
};

} // namespace backend

#endif //KNDB_STORAGE_ENGINE_HPP
