//
// Created by Kylan Chen on 3/3/25.
//

#include <cassert>

#include "TablePage.hpp"
#include "utility.hpp"

TablePage::TablePage(std::span<const Byte> bytes, size_t pageID) : Page(pageID) {
    size_t offset = 0;

    size_t page_type_id, num_types, type_id;
    deserialize(page_type_id, bytes, offset);

    // check if page type is correct
    if (page_type_id != cts::pg_type_id::TABLE_PAGE)
        throw std::runtime_error("page type id is incorrect");

    // deserialize # of types
    deserialize(num_types, bytes, offset);

    // page has not been initialized yet, since tuples cannot be empty
    if (num_types == 0)
        throw std::runtime_error("Page has not been initialized yet, has no types");

    // deserialize types
    for (int j = 0; j < num_types; ++j) {
        deserialize(type_id, bytes, offset);
        m_types.push_back(type_id_to_variant(type_id));
    }

    // deserialize btree page id
    deserialize(m_btreePageID, bytes, offset);

    // deserialize numTuples
    deserialize(m_numTuples, bytes, offset);

    assert(offset <= cts::PG_SZ);
}

TablePage::TablePage(const Vec<Vari>& types, size_t btreePageID, size_t pageID)
: Page(pageID), m_types(types), m_btreePageID(btreePageID), m_numTuples(0) {
    if (types.empty())
        throw std::invalid_argument("There cannot be 0 types in TablePage.");

    if (types.size() > (cts::PG_SZ - db_sizeof<size_t>() * 10) / 8) {
        throw std::invalid_argument("TablePage cannot support that many types.");
    }
}

size_t TablePage::getBtreePageID() const {
    return m_btreePageID;
}

void TablePage::setBtreePageID(size_t btreePageID) {
    m_btreePageID = btreePageID;
}

const Vec<Vari>& TablePage::getTypes() {
    return m_types;
}

size_t TablePage::getNumTuples() const {
    return m_numTuples;
}

void TablePage::addTuple() {
    m_numTuples++;
}

void TablePage::removeTuple() {
    if (m_numTuples < 1)
        throw std::runtime_error("The tuple count is already at 0");

    m_numTuples--;
}

void TablePage::toBytes(std::span<Byte> buf) {
    size_t offset = 0;

    // serialize page_type_id
    size_t page_type_id = cts::pg_type_id::TABLE_PAGE;
    serialize(page_type_id, buf, offset);

    // serialize # of types
    size_t numTypes = m_types.size();
    serialize(numTypes, buf, offset);

    // serialize list of types in each table
    for (int j = 0; j < numTypes; ++j) {
        size_t type_id = variant_to_type_id(m_types[j]);
        serialize(type_id, buf, offset);
    }

    // serialize btree page id
    serialize(m_btreePageID, buf, offset);

    // serialize num tuples
    serialize(m_numTuples, buf, offset);

    assert (offset <= cts::PG_SZ);
}