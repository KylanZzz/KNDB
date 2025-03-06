//
// Created by Kylan Chen on 3/3/25.
//

#include <cassert>

#include "BtreeNodePage.hpp"
#include "utility.hpp"

    //    size_t pagetypeid
    //    size_t m_degree;
    //    size_t m_parentID;
    //    bool m_isLeaf;
    //    bool m_isRoot;
    //
    //    vector<variants> m_types;
    //    vector<size_t> m_children;
    //    vector<cell> m_cells;

    //    cell {
    //    variant key
    //    vec<variants> values
    //    }
BtreeNodePage::BtreeNodePage(ByteVec &bytes, size_t pageID) : Page(pageID) {
    size_t offset = 0;

    size_t page_type_id;
    serialize(page_type_id, bytes, offset);

    // check if page type is correct
    if (page_type_id != get_page_type_id<BtreeNodePage>())
        throw std::runtime_error("page_type_id does not match any valid page type");

    // deserialize degree
    serialize(m_degree, bytes, offset);

    // deserialize parentID
    serialize(m_parentID, bytes, offset);

    // deserialize isLeaf
    serialize(m_isLeaf, bytes, offset);

    // deserialize isLeaf
    serialize(m_isRoot, bytes, offset);

    size_t numCells, numTypes, key_type_id;

    // deserialize number of cells
    serialize(numCells, bytes, offset);
    m_cells.resize(numCells);

    // deserialize number of types
    serialize(numTypes, bytes, offset);

    // serialize key type
    serialize(key_type_id, bytes, offset);
    m_key = type_id_to_variant(key_type_id);

    // serialize tuple types
    size_t type_id;
    for (int i = 0; i < numTypes; ++i) {
        serialize(type_id, bytes, offset);
        m_types.push_back(type_id_to_variant(type_id));
    }

    // serialize cells
    for (int i = 0; i < numCells; i++) {
        auto& cell = m_cells[i];
        // serialize key
        serialize(cell.key, bytes, offset, m_key);

        // serialize tuple
        for (int j = 0; j < numTypes; ++j) {
            serialize(cell.tuple[j], bytes, offset, m_types[j]);
        }
    }

    // serialize children
    if (numCells != 0)
        for (int i = 0; i < numCells + 1; i++) {
            size_t child_id;
            serialize(child_id, bytes, offset);
            m_children.push_back(child_id);
        }


    assert (offset <= cts::PG_SZ);
}

void BtreeNodePage::toBytes(kndb_types::ByteVec &vec) {
    size_t offset = 0;
}

BtreeNodePage::BtreeNodePage(size_t degree, size_t parentID, bool is_root, bool is_leaf,
                             const vector<variants>& types, size_t pageID)
        : Page(pageID), m_isLeaf(is_leaf), m_isRoot(is_root), m_degree (degree),
        m_parentID(parentID), m_types(types), m_children(0), m_cells(0) {};