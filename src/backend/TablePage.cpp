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
    assert(types.empty());

    m_types = types;
    m_btreePageID = btreePageID;
    m_numTuples = 0;
}

size_t TablePage::getBtreePageNo() { return m_btreePageID; }

vector<variants> TablePage::getTypes() { return m_types; }

void TablePage::addTuple() { m_numTuples++; }

void TablePage::removeTuple() {
    m_numTuples--;
    assert(m_numTuples >= 0);
}
