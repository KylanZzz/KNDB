//
// Created by Kylan Chen on 3/3/25.
//

#include <cassert>

#include "TablePage.hpp"
#include "utility.hpp"

namespace backend {

TablePage::TablePage(std::span<const byte> bytes, u32 pageID) : Page(pageID) {
    u16 offset = 0;

    u8 page_type_id, type_id;
    u16 num_types;
    db_deserialize(page_type_id, bytes, offset);

    // check if page type is correct
    if (page_type_id != cts::pg_type_id::TABLE_PAGE)
        throw std::runtime_error("page type id is incorrect");

    // deserialize # of types
    db_deserialize(num_types, bytes, offset);

    // page has not been initialized yet, since tuples cannot be empty
    if (num_types == 0)
        throw std::runtime_error("Page has not been initialized yet, has no types");

    // deserialize types
    for (int j = 0; j < num_types; ++j) {
        db_deserialize(type_id, bytes, offset);
        m_types.push_back(type_id_to_variant(type_id));
    }

    // deserialize btree page id
    db_deserialize(m_btreePageID, bytes, offset);

    // deserialize numTuples
    db_deserialize(m_numTuples, bytes, offset);

    assert(offset <= cts::PG_SZ);
}

TablePage::TablePage(const Vec<Vari> &types, u32 btreePageID, u32 pageID)
        : Page(pageID), m_types(types), m_btreePageID(btreePageID), m_numTuples(0) {
    if (types.empty())
        throw std::invalid_argument("There cannot be 0 types in TablePage.");

    if (types.size() > (cts::PG_SZ - 100) / db_sizeof<u8>()) {
        throw std::invalid_argument("TablePage cannot support that many types.");
    }
}

u32 TablePage::getBtreePageID() const {
    return m_btreePageID;
}

void TablePage::setBtreePageID(u32 btreePageID) {
    m_btreePageID = btreePageID;
}

const Vec<Vari> &TablePage::getTypes() {
    return m_types;
}

u64 TablePage::getNumTuples() const {
    return m_numTuples;
}

void TablePage::addTuple() {
    m_numTuples++;
}

void TablePage::removeTuple() {
    assert(m_numTuples > 0);

    m_numTuples--;
}

void TablePage::toBytes(std::span<byte> buf) {
    u16 offset = 0;

    // serialize page_type_id
    u8 page_type_id = cts::pg_type_id::TABLE_PAGE;
    db_serialize(page_type_id, buf, offset);

    // serialize # of types
    u16 numTypes = m_types.size();
    db_serialize(numTypes, buf, offset);

    // serialize list of types in each table
    for (int j = 0; j < numTypes; ++j) {
        u8 type_id = variant_to_type_id(m_types[j]);
        db_serialize(type_id, buf, offset);
    }

    // serialize btree page id
    db_serialize(m_btreePageID, buf, offset);

    // serialize num tuples
    db_serialize(m_numTuples, buf, offset);

    assert (offset < cts::PG_SZ);
}

} // namespace backend