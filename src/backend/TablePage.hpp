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
     * Loads a TablePage from a byte vector.
     * @param bytes The byte vector containing table data.
     * @param pageID The on-disk page number of this table page.
     */
    TablePage(std::span<const Byte> bytes, size_t pageID);

    /**
     * Creates an empty TablePage.
     * @param types List of types that this table contains.
     * @param btreePageID The root node pageID of the btree that stores this table's data.
     * @param pageID The on-disk page number of this table page.
     * @throws std::invalid_argument if list of types is empty or too long.
     */
    TablePage(const Vec<Vari> &types, size_t btreePageID, size_t pageID);

    /**
     * Get the types that this table stores.
     * @throws std::runtime_error if the TablePage has not been initialized yet.
     * @return A list of all the types.
     */
    const Vec<Vari> &getTypes();

    /**
     * @throws std::runtime_error if the TablePage has not been initialized yet.
     * @return The pageID of the root node that contains the data for this table.
     */
    size_t getBtreePageID() const;

    /**
     * @throws std::runtime_error if the TablePage has not been initialized yet.
     * @return The number of tuples of the table that this page represents.
     */
    size_t getNumTuples() const;

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
     * Changes the page ID of the table's btree root.
     * @param btreePageID the new page ID.
     */
    void setBtreePageID(size_t btreePageID);

    void toBytes(std::span<Byte> buffer) override;

private:
    Vec<Vari> m_types;
    size_t m_btreePageID;
    size_t m_numTuples;
};


#endif //KNDB_TABLEPAGE_HPP
