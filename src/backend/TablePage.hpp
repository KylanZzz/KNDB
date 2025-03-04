//
// Created by Kylan Chen on 3/3/25.
//

#ifndef KNDB_TABLEPAGE_HPP
#define KNDB_TABLEPAGE_HPP

#include "Page.hpp"

/**
 * @class TablePage
 * @brief Representation of an on-disk page that contains metadata for a table such as the types
 * and number of tuples in the table.
 */
class TablePage : public Page {
public:
    /**
     * Creates a TablePage from a byte vector.
     * @param bytes The byte vector containing table data.
     * @param pageID The on-disk page number of this table page.
     */
    TablePage(ByteVec &bytes, size_t pageID);

    /**
     * Get the types that this table stores.
     * @throws std::runtime_error if the TablePage has not been initialized yet.
     * @return A list of all the types.
     */
    vector<variants> getTypes();

    /**
     * @throws std::runtime_error if the TablePage has not been initialized yet.
     * @return The pageID of the root node that contains the data for this table.
     */
    size_t getBtreePageID();

    /**
     * @throws std::runtime_error if the TablePage has not been initialized yet.
     * @return The number of tuples of the table that this page represents.
     */
    size_t getNumTuples();

    /**
     * Adds a tuple to the count of tuples in this table.
     * @throws std::runtime_error if the TablePage has not been initialized yet.
     */
    void addTuple();

    /**
     * Removes a tuple to the count of tuples in this table.
     * @throws std::runtime_error if the TablePage has not been initialized yet.
     */
    void removeTuple();

    /**
     * Initializes this table page to contain a given list of types and btree root node pageID.
     * @param types the list of types that this table page represents.
     * @throws std::runtime_error if the TablePage has already been initialized.
     * @throws std::invalid_argument if list of types is empty or too long.
     * @param btreePageID the pageID of the btree root node which contains this table's data.
     */
    void init(const vector<variants> &types, size_t btreePageID);

    /**
     * Changes the page ID of the table's btree root.
     * @param btreePageID the new page ID.
     */
    void setBtreePageID(size_t btreePageID);

    void toBytes(ByteVec &vec) override;

private:
    vector<variants> m_types;
    size_t m_btreePageID;
    size_t m_numTuples;
    bool m_isInit{false};
};


#endif //KNDB_TABLEPAGE_HPP
