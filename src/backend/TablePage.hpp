//
// Created by Kylan Chen on 3/3/25.
//

#ifndef KNDB_TABLEPAGE_HPP
#define KNDB_TABLEPAGE_HPP

#include "Page.hpp"
#include "kndb_types.hpp"

namespace backend {

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
    TablePage(std::span<const byte> bytes, pgid_t pageID);

    /**
     * Creates an empty TablePage.
     * @param types List of types that this table contains.
     * @param btreePageID The root node pageID of the btree that stores this table's data.
     * @param pageID The on-disk page number of this table page.
     * @throws std::invalid_argument if list of types is empty or too long.
     */
    TablePage(const Vec<Vari> &types, pgid_t btreePageID, pgid_t pageID);

    /**
     * Get the types that this table stores.
     * @return A list of all the types.
     */
    const Vec<Vari> &getTypes();

    /**
     * @return The pageID of the root node that contains the data for this table.
     */
    pgid_t getBtreePageID() const;

    /**
     * @return The number of tuples of the table that this page represents.
     */
    u64 getNumTuples() const;

    /**
     * Adds a tuple to the count of tuples in this table.
     */
    void addTuple();

    /**
     * Removes a tuple to the count of tuples in this table.
     */
    void removeTuple();

    /**
     * Changes the page ID of the table's btree root.
     * @param btreePageID the new page ID.
     */
    void setBtreePageID(pgid_t btreePageID);

    void toBytes(std::span<byte> buffer) override;

private:
    Vec<Vari> m_types;
    pgid_t m_btreePageID;
    u64 m_numTuples;
};

} //namespace backend

#endif //KNDB_TABLEPAGE_HPP
