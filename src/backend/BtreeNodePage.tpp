//
// Created by Kylan Chen on 3/3/25.
//
#ifndef KNDB_BTREENODEPAGE_TPP
#define KNDB_BTREENODEPAGE_TPP

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

template<typename T>
BtreeNodePage<T>::BtreeNodePage(ByteVec &bytes, size_t pageID) : Page(pageID) {
    size_t offset = 0;

    size_t page_type_id;
    deserialize(page_type_id, bytes, offset);

    // check if page type is correct
    if (page_type_id != cts::pg_type_id::BTREE_NODE_PAGE)
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

    // deserialize key type
    variants key;
    deserialize(key_type_id, bytes, offset);
    key = type_id_to_variant(key_type_id);

    vector<variants> types;
    if constexpr (std::is_same<T, vector<variants>>::value) {
        // deserialize number of types
        deserialize(numTypes, bytes, offset);

        // deserialize tuple types
        for (int i = 0; i < numTypes; ++i) {
            size_t type_id;
            deserialize(type_id, bytes, offset);
            types.push_back(type_id_to_variant(type_id));
        }
    }

    // deserialize cells
    for (int i = 0; i < numCells; i++) {
        auto &cell = m_cells[i];
        // deserialize key
        deserialize(cell.key, bytes, offset, key);

        // deserialize tuple
        if constexpr (std::is_same<T, vector<variants>>::value) {
            cell.value.resize(numTypes);
            for (int j = 0; j < numTypes; ++j)
                deserialize(cell.value[j], bytes, offset, types[j]);
        }
        else
            deserialize(cell.value, bytes, offset);
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

template<typename T>
void BtreeNodePage<T>::toBytes(ByteVec &vec) {
    size_t offset = 0;

    assert(m_cells.empty() || m_children.size() == m_cells.size() + 1);

    // serialize page type id
    size_t page_type_id = cts::pg_type_id::BTREE_NODE_PAGE;
    serialize(page_type_id, vec, offset);

    // serialize degree
    serialize(m_degree, vec, offset);

    // serialize parentID
    serialize(m_parentID, vec, offset);

    // serialize isLeaf
    serialize(m_leaf, vec, offset);

    // serialize isRoot
    serialize(m_root, vec, offset);

    size_t numCells = m_cells.size();

    //  serialize num cells
    serialize(numCells, vec, offset);

    if (numCells == 0) return;

    // serialize key type id
    size_t key_type_id = variant_to_type_id(m_cells[0].key);
    serialize(key_type_id, vec, offset);

    if constexpr (std::is_same<T, vector<variants>>::value) {
        // serialize num types
        serialize(m_cells[0].value.size(), vec, offset);

        // serialize tuple types
        size_t type_id;
        for (int i = 0; i < m_cells[0].value.size(); i++) {
            type_id = variant_to_type_id(m_cells[0].value[i]);
            serialize(type_id, vec, offset);
        }
    }

    // serialize cells
    for (int i = 0; i < numCells; i++) {
        const auto &cell = m_cells[i];

        serialize(cell.key, vec, offset);

        if constexpr (std::is_same<T, vector<variants>>::value)
            for (int j = 0; j < m_cells[0].value.size(); ++j)
                serialize(cell.value[j], vec, offset);
        else
            serialize(cell.value, vec, offset);
    }

    for (int i = 0; i < numCells + 1; i++)
        serialize(m_children[i], vec, offset);
}

template<typename T>
BtreeNodePage<T>::BtreeNodePage(size_t deg, size_t parentID, bool is_root, bool is_leaf, size_t
pageID)
        : Page(pageID), m_leaf(is_leaf), m_root(is_root), m_degree(deg), m_parentID(parentID),
          m_children(0), m_cells(0) {}

#endif //KNDB_BTREENODEPAGE_TPP