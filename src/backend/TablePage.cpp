//
// Created by Kylan Chen on 3/3/25.
//

#include "TablePage.hpp"
#include "utility.hpp"
#include "assume.hpp"

namespace backend {

TablePage::TablePage(std::span<const byte> bytes, pgid_t pageID) : Page(pageID) {
    ASSUME_S(bytes.size() == cts::PG_SZ, "Buffer is incorrectly sized");
    offset_t offset = 0;

    pgtypeid_t page_type_id;
    typeid_t type_id;
    u16 num_types;
    db_deserialize(page_type_id, bytes, offset);

    // check if page type is correct
    ASSUME_S(page_type_id == cts::pg_type_id::TABLE_PAGE, "Invalid page_type_id");

    // deserialize # of types
    db_deserialize(num_types, bytes, offset);

    ASSUME_S(num_types > 0, "Page contains zero types (likely has not been initialized yet)");

    // deserialize types
    for (int j = 0; j < num_types; ++j) {
        db_deserialize(type_id, bytes, offset);

        m_types.push_back(type_id_to_variant(type_id));
    }

    // deserialize btree page id
    db_deserialize(m_btreePageID, bytes, offset);

    // deserialize numTuples
    db_deserialize(m_numTuples, bytes, offset);

    ASSUME_S(offset <= cts::PG_SZ, "Offset is out of bounds");
}

TablePage::TablePage(const Vec<Vari> &types, pgid_t btreePageID, pgid_t pageID)
        : Page(pageID), m_types(types), m_btreePageID(btreePageID), m_numTuples(0) {
    ASSUME_S(!types.empty(), "There cannot be 0 types in a TablePage");
    ASSUME_S(types.size() <= (cts::PG_SZ - 100) / db_sizeof<typeid_t>(), "TablePage cannot support that many types");
}

pgid_t TablePage::getBtreePageID() const {
    return m_btreePageID;
}

void TablePage::setBtreePageID(pgid_t btreePageID) {
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
    ASSUME_S(m_numTuples > 0, "Table has no tuples left to remove");

    m_numTuples--;
}

void TablePage::toBytes(std::span<byte> buf) {
    ASSUME_S(buf.size() == cts::PG_SZ, "Buffer is incorrectly sized");
    offset_t offset = 0;

    // serialize page_type_id
    pgtypeid_t page_type_id = cts::pg_type_id::TABLE_PAGE;
    db_serialize(page_type_id, buf, offset);

    // serialize # of types
    u16 numTypes = m_types.size();
    db_serialize(numTypes, buf, offset);

    // serialize list of types in each table
    for (int j = 0; j < numTypes; ++j) {
        typeid_t type_id = variant_to_type_id(m_types[j]);
        db_serialize(type_id, buf, offset);
    }

    // serialize btree page id
    db_serialize(m_btreePageID, buf, offset);

    // serialize num tuples
    db_serialize(m_numTuples, buf, offset);

    ASSUME_S(offset <= cts::PG_SZ, "Offset is out of bounds");
}

} // namespace backend