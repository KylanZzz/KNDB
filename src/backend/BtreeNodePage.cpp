//
// Created by Kylan Chen on 3/3/25.
//

#include <cassert>
#include <utility>

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
    deserialize(page_type_id, bytes, offset);

    // check if page type is correct
    if (page_type_id != get_page_type_id<BtreeNodePage>())
        throw std::runtime_error("page_type_id does not match any valid page type");

    // deserialize degree
    deserialize(m_degree, bytes, offset);

    // deserialize parentID
    deserialize(m_parentID, bytes, offset);

    // deserialize isLeaf
    deserialize(m_leaf, bytes, offset);

    // deserialize isLeaf
    deserialize(m_root, bytes, offset);

    size_t numCells, numTypes, key_type_id;

    // deserialize number of cells
    deserialize(numCells, bytes, offset);
    m_cells.resize(numCells);

    if (numCells == 0) return;

    // deserialize number of types
    deserialize(numTypes, bytes, offset);

    // deserialize key type
    variants key;
    deserialize(key_type_id, bytes, offset);
    key = type_id_to_variant(key_type_id);

    // deserialize tuple types
    vector<variants> types;
    size_t type_id;
    for (int i = 0; i < numTypes; ++i) {
        deserialize(type_id, bytes, offset);
        types.push_back(type_id_to_variant(type_id));
    }

    // deserialize cells
    for (int i = 0; i < numCells; i++) {
        auto &cell = m_cells[i];
        cell.tuple.resize(numTypes);
        // deserialize key
        deserialize(cell.key, bytes, offset, key);

        // deserialize tuple
        for (int j = 0; j < numTypes; ++j)
            deserialize(cell.tuple[j], bytes, offset, types[j]);
    }

    // serialize children
    if (numCells != 0)
        for (int i = 0; i < numCells + 1; i++) {
            size_t child_id;
            deserialize(child_id, bytes, offset);
            m_children.push_back(child_id);
        }

    assert (offset <= cts::PG_SZ);
}

void BtreeNodePage::toBytes(ByteVec &vec) {
    size_t offset = 0;

    assert(m_cells.empty() || m_children.size() == m_cells.size() + 1);

    // serialize page type id
    size_t page_type_id = get_page_type_id<BtreeNodePage>();
    serialize(page_type_id, vec, offset);

    // serialize degree
    serialize(m_degree, vec, offset);

    // serialize parentID
    serialize(m_parentID, vec, offset);

    // serialize isLeaf
    serialize(m_leaf, vec, offset);

    // serialize isRoot
    serialize(m_root, vec, offset);

    //  serialize num cells
    size_t numCells = m_cells.size();
    serialize(numCells, vec, offset);

    if (numCells == 0) return;

    // serialize num types
    size_t numTypes = m_cells[0].tuple.size();
    serialize(numTypes, vec, offset);

    // serialize key type id
    size_t key_type_id = variant_to_type_id(m_cells[0].key);
    serialize(key_type_id, vec, offset);

    // serialize tuple types
    size_t type_id;
    for (int i = 0; i < numTypes; i++) {
        type_id = variant_to_type_id(m_cells[0].tuple[i]);
        serialize(type_id, vec, offset);
    }

    // serialize cells
    for (int i = 0; i < numCells; i++) {
        const auto &cell = m_cells[i];

        serialize(cell.key, vec, offset);

        for (int j = 0; j < numTypes; ++j)
            serialize(cell.tuple[j], vec, offset);
    }

    for (int i = 0; i < numCells + 1; i++)
        serialize(m_children[i], vec, offset);
}

BtreeNodePage::BtreeNodePage(size_t degree, size_t parentID, bool is_root, bool is_leaf, size_t pageID)
        : Page(pageID), m_leaf(is_leaf), m_root(is_root), m_degree(degree), m_parentID(parentID),
          m_children(0), m_cells(0) {}