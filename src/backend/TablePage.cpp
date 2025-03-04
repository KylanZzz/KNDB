//
// Created by Kylan Chen on 3/3/25.
//

#include <cassert>

#include "TablePage.hpp"
#include "utility.hpp"

TablePage::TablePage(ByteVec &bytes, size_t pageID) : Page(pageID) {
    size_t offset = 0;

    // check if page type is correct
    size_t page_type_id;
    memcpy(&page_type_id, bytes.data() + offset, db_sizeof<size_t>());
    if (page_type_id != get_page_type_id<TablePage>())
        throw std::runtime_error("page_type_id does not match any valid page type");
    offset += db_sizeof<size_t>();

    // deserialize # of types
    size_t num_types;
    memcpy(&num_types, bytes.data() + offset, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // page has not been initialized yet, so we return
    if (num_types == 0)
        return;

    m_isInit = true;

    // serialize types
    size_t type_id;
    for (int j = 0; j < num_types; ++j) {
        memcpy(&type_id, bytes.data() + offset, db_sizeof<size_t>());
        offset += db_sizeof<size_t>();
        m_types.push_back(type_id_to_variant(type_id));
    }

    // deserialize btree page id
    memcpy(&m_btreePageID, bytes.data() + offset, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // deserialize numTuples
    memcpy(&m_numTuples, bytes.data() + offset, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    assert(offset <= cts::PG_SZ);
}

void TablePage::init(const vector<variants> &types, size_t btreePageID) {
    if (m_isInit)
        throw std::runtime_error("TablePage has already been initialized.");

    assert(m_types.empty());

    if (types.empty())
        throw std::invalid_argument("There cannot be 0 types in TablePage.");

    if (types.size() > (cts::PG_SZ - db_sizeof<size_t>() * 10) / 8)
        throw std::invalid_argument("TablePage cannot support that many types.");

    m_isInit = true;
    m_types = types;
    m_btreePageID = btreePageID;
    m_numTuples = 0;
}

size_t TablePage::getBtreePageID() {
    if (!m_isInit)
        throw std::runtime_error("TablePage has not been initialized yet");

    return m_btreePageID;
}

void TablePage::setBtreePageID(size_t btreePageID) {
    if (!m_isInit)
        throw std::runtime_error("TablePage has not been initialized yet");

    m_btreePageID = btreePageID;
}

vector<variants> TablePage::getTypes() {
    if (!m_isInit)
        throw std::runtime_error("TablePage has not been initialized yet");

    return m_types;
}

size_t TablePage::getNumTuples() {
    if (!m_isInit)
        throw std::runtime_error("TablePage has not been initialized yet");

    return m_numTuples;
}

void TablePage::addTuple() {
    if (!m_isInit)
        throw std::runtime_error("TablePage has not been initialized yet");

    m_numTuples++;
}

void TablePage::removeTuple() {
    if (!m_isInit)
        throw std::runtime_error("TablePage has not been initialized yet");

    if (m_numTuples < 1)
        throw std::runtime_error("The tuple count is already at 0");

    m_numTuples--;
}

void TablePage::toBytes(ByteVec &vec) {
    if (!m_isInit)
        throw std::runtime_error("TablePage has not been initialized yet");

    size_t offset = 0;

    // serialize page_type_id
    size_t page_type_id = get_page_type_id<TablePage>();
    memcpy(vec.data() + offset, &page_type_id, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // serialize # of types
    size_t numTypes = m_types.size();
    memcpy(vec.data() + offset, &numTypes, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // serialize list of types in each table
    for (int j = 0; j < numTypes; ++j) {
        size_t type_id = variant_to_type_id(m_types[j]);
        memcpy(vec.data() + offset, &type_id, db_sizeof<size_t>());
        offset += db_sizeof<size_t>();
    }

    // serialize btree page id
    memcpy(vec.data() + offset, &m_btreePageID, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // serialize num tuples
    memcpy(vec.data() + offset, &m_numTuples, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    assert (offset <= cts::PG_SZ);
}