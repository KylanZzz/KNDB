//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_TABLE_HPP
#define KNDB_TABLE_HPP

#include "Pager.hpp"
#include "Btree.hpp"
#include "kndb_types.hpp"

using namespace kndb;

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
     * @param pgr Reference to the Pager.
     * @param tablePageId Page ID of the table's metadata page.
     */
    Table(string name, Pager& pgr, u32 tablePageId);

    /**
     * @brief Constructs a new Table.
     * @param name The name of the table.
     * @param pgr Reference to the Pager.
     * @param types A list of types that the table tuples will contain.
     */
    Table(string name, Pager& pgr, const Vec<Vari>& types);

    /**
     * @brief Deletes the table and all associated data, including any Btree Nodes used to store
     * the data.
     */
    void drop();

    /**
     * @brief Retrieves the table name.
     * @return The name of the table.
     */
    string getName();

    /**
     * @brief Retrieves the number of tuples in the table.
     * @return The number of tuples stored in the table.
     */
    u64 getNumTuples() const;

    /**
     * @brief Inserts a new tuple into the table.
     * @param values The values to insert as a tuple.
     *
     * The first index of the tuple will ALWAYS be the primary key, meaning it must be unique.
     */
    void insertTuple(Vec<Vari> values) const;

    /**
     * @brief Reads a tuple from the table using the key.
     * @param key The key to look up.
     * @return The tuple associated with the key.
     */
    Vec<Vari> readTuple(const Vari& key) const;

    /**
     * @brief Updates an existing tuple in the table.
     * @param values The updated tuple values.
     *
     * The first index of the tuple will ALWAYS be the primary key, meaning it must be unique.
     */
    void updateTuple(const Vec<Vari> &values) const;

    /**
     * @brief Deletes a tuple from the table using the key.
     * @param key The key of the tuple to delete.
     */
    void deleteTuple(const Vari &key) const;

    /**
     * @brief Retrieves the column types of the table.
     * @return A list containing the types of each column.
     */
    Vec<Vari> getTypes() const;

    /**
     * Get the Table PageID.
     * @return The ID of the page that stores metadata about the table.
     */
    u32 getTablePageID() const;

private:
    Pager& m_pager;
    std::unique_ptr<Btree<Vec<Vari>>> m_btree;
    u32 m_tablePageID;
    string m_name;
};


#endif //KNDB_TABLE_HPP
