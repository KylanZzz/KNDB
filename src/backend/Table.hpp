//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_TABLE_HPP
#define KNDB_TABLE_HPP

#include <vector>
#include "Pager.hpp"
#include "Btree.hpp"
#include "kndb_types.hpp"

using namespace kndb_types;

/**
 * @class Table
 * @brief Represents a table in the database.
 *
 * Manages tuple storage and retrieval using a B-tree for indexing.
 * Provides CRUD (Create, Read, Update, Delete) operations on tuples.
 */
class Table {
public:
    /**
     * @brief Constructs a Table from an existing TablePage.
     * @param name The name of the table.
     * @param pgr Reference to the Pager for managing pages.
     * @param tablePageId Page ID of the table's metadata page.
     *
     * If no corresponding B-tree node exists, one is initialized.
     */
    Table(string name, Pager& pgr, size_t tablePageId);

    /**
     * @brief Deletes the table and all associated data, including any Btree Nodes used to store
     * the data.
     */
    void dropTable();

    /**
     * @brief Retrieves the table name.
     * @return The name of the table.
     */
    string getName();

    /**
     * @brief Retrieves the number of tuples in the table.
     * @return The number of tuples stored in the table.
     */
    size_t getNumTuples();

    /**
     * @brief Inserts a new tuple into the table.
     * @param values The values to insert as a tuple.
     *
     * The first index of the tuple will ALWAYS be the primary key, meaning it must be unique.
     */
    void createTuple(vector<variants> values);

    /**
     * @brief Reads a tuple from the table using the key.
     * @param key The key to look up.
     * @return The tuple associated with the key.
     */
    vector<variants> readTuple(variants key);

    /**
     * @brief Updates an existing tuple in the table.
     * @param values The updated tuple values.
     *
     * The first index of the tuple will ALWAYS be the primary key, meaning it must be unique.
     */
    void updateTuple(vector<variants> values);

    /**
     * @brief Deletes a tuple from the table using the key.
     * @param key The key of the tuple to delete.
     */
    void deleteTuple(variants key);

    /**
     * @brief Retrieves the column types of the table.
     * @return A list containing the types of each column.
     */
    vector<variants> getTypes();

private:
    Pager& m_pager;
    std::unique_ptr<Btree> m_btree;
    size_t m_tablePageID;
    string m_name;
};


#endif //KNDB_TABLE_HPP
