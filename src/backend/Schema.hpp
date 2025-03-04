//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_SCHEMA_HPP
#define KNDB_SCHEMA_HPP

#include "Pager.hpp"
#include "Table.hpp"
#include "kndb_types.hpp"

using namespace kndb_types;

/**
 * @class Schema
 *
 * Represents a schema in the database, which can contain multiple tables. This class serves as
 * an interface to the rest of the backend in KNDB. It is responsible for creating, dropping,
 * reading, and updating tables.
 *
 * So far, we will assume that the database can only contain one schema. As such, the page ID for
 * the schema page will also be hardcoded.
 */
class Schema {
public:
    /**
     * Constructs a schema from an SchemaPage.
     *
     * @param schemaPageID the pageID that contains the information for the Schema.
     * @param pgr The pager.
     */
    Schema(Pager& pgr, size_t schemaPageID);

    /**
     * Creates a table in the Schema.
     *
     * @throws std::invalid_argument if the name is empty, is too long, or already exists.
     * @param name the name of the table (case sensitive).
     * @param types the
     */
    void createTable(string name, vector<variants> types);

    /**
     * Drops a table from the Schema.
     *
     * @throws std::invalid_argument if the name does not exist.
     * @param name the name of the table to drop (case sensitive).
     */
    void dropTable(string name);

    /**
     * Get all the tables' names in the Schema.
     *
     * @return a list of all the table names.
     */
    vector<string> getTableNames();

    /**
     * Get the types that a table in the Schema contains.
     *
     * @throws std::invalid_argument if the 'table' does not exist in the Schema.
     * @param table the name of the table.
     * @return a list of the types in the table.
     */
    vector<variants> getTableTypes(string table);

    /**
     * Get the number of tuples a table contains.
     *
     * @throws std::invalid_argument if the 'table' does not exist in the Schema.
     * @param table the name of the table.
     * @return the number of tuples.
     */
    size_t getNumTuples(string table);

    /**
     * Removes a tuple from the database.
     *
     * @throws std::invalid_argument if the 'table' does not exist in the Schema.
     * @param table the name of the table.
     * @param key the value of the key of the desired tuple to remove.
     */
    void removeTuple(string table, variants key);

    /**
     * Removes a tuple from the database.
     *
     * @throws std::invalid_argument if the 'table' does not exist in the Schema.
     * @param table the name of the table.
     * @param values the updated tuple.
     */
    void updateTuple(string table, vector<variants> values);

    /**
     * Removes a tuple from the database.
     *
     * @throws std::invalid_argument if the 'table' does not exist in the Schema.
     * @param table the name of the table.
     * @param values the updated tuple.
     */
    void insertTuple(string table, vector<variants> values);

    /**
     * Retrieves a tuple from a given table in the Schema.
     *
     * @throws std::invalid_argument if the 'table' does not exist in the Schema.
     * @param table the name of the table.
     * @param key the value of the key of the desired tuple to retrieve.
     * @return
     */
    vector<variants> getTuple(string table, variants key);

private:
    bool sameTypes(vector<variants> vec1, vector<variants> vec2);

    Pager& m_pager;
    vector<std::unique_ptr<Table>> m_tables;
    size_t m_schemaPageID;
};


#endif //KNDB_SCHEMA_HPP
