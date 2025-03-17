//
// Created by Kylan Chen on 3/3/25.
//
#ifndef KNDB_BTREENODEPAGE_TPP
#define KNDB_BTREENODEPAGE_TPP

#include <cassert>
#include <iostream>

#include "BtreeNodePage.hpp"
#include "utility.hpp"

namespace backend {

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
BtreeNodePage<T>::BtreeNodePage(std::span<const byte> bytes, u32 pageID) : Page(pageID) {
    u16 offset = 0;

    u8 page_type_id;
    db_deserialize(page_type_id, bytes, offset);

    // check if page type is correct
    if (page_type_id != cts::pg_type_id::BTREE_NODE_PAGE)
        throw std::runtime_error("page_type_id does not match any valid page type");

    // deserialize degree
    db_deserialize(m_degree, bytes, offset);

    // deserialize parentID
    db_deserialize(m_parentID, bytes, offset);

    // deserialize isLeaf
    db_deserialize(m_leaf, bytes, offset);

    // deserialize isLeaf
    db_deserialize(m_root, bytes, offset);

    u32 numCells;
    u8 numTypes, key_type_id;

    // deserialize number of cells
    db_deserialize(numCells, bytes, offset);
    m_cells.resize(numCells);

    if (numCells == 0) return;

    // deserialize key type
    Vari key;
    db_deserialize(key_type_id, bytes, offset);
    key = type_id_to_variant(key_type_id);

    Vec<Vari> types;
    if constexpr (std::is_same_v<T, Vec<Vari> >) {
        // deserialize number of types
        db_deserialize(numTypes, bytes, offset);

        // deserialize tuple types
        for (int i = 0; i < numTypes; ++i) {
            u8 type_id;
            db_deserialize(type_id, bytes, offset);
            types.push_back(type_id_to_variant(type_id));
        }
    }

    // deserialize cells
    for (int i = 0; i < numCells; i++) {
        auto &cell = m_cells[i];
        // deserialize key
        db_deserialize(cell.key, bytes, offset, key);

        // deserialize tuple
        if constexpr (std::is_same<T, Vec<Vari> >::value) {
            cell.value.resize(numTypes);
            for (int j = 0; j < numTypes; ++j)
                db_deserialize(cell.value[j], bytes, offset, types[j]);
        } else
            db_deserialize(cell.value, bytes, offset);
    }

    // serialize children
    if (!m_leaf)
        for (int i = 0; i < numCells + 1; i++) {
            u32 child_id;
            db_deserialize(child_id, bytes, offset);
            m_children.push_back(child_id);
        }

    assert(offset <= cts::PG_SZ);
}

template<typename T>
void BtreeNodePage<T>::toBytes(std::span<byte> buf) {
    assert(buf.size() == cts::PG_SZ);
    assert(m_leaf || m_cells.empty() || m_children.size() == m_cells.size() + 1);

    u16 offset = 0;

    // serialize page type id
    u8 page_type_id = cts::pg_type_id::BTREE_NODE_PAGE;
    db_serialize(page_type_id, buf, offset);

    // serialize degree
    db_serialize(m_degree, buf, offset);

    // serialize parentID
    db_serialize(m_parentID, buf, offset);

    // serialize isLeaf
    db_serialize(m_leaf, buf, offset);

    // serialize isRoot
    db_serialize(m_root, buf, offset);

    u32 numCells = m_cells.size();

    //  serialize num cells
    db_serialize(numCells, buf, offset);

    if (numCells == 0) return;

    // // deserialize key type
    // Vari key;
    // deserialize(key_type_id, bytes, offset);
    // key = type_id_to_variant(key_type_id);
    //
    // Vec<Vari> types;
    // if constexpr (std::is_same_v<T, Vec<Vari> >) {
    //     // deserialize number of types
    //     deserialize(numTypes, bytes, offset);
    //
    //     // deserialize tuple types
    //     for (int i = 0; i < numTypes; ++i) {
    //         u8 type_id;
    //         deserialize(type_id, bytes, offset);
    //         types.push_back(type_id_to_variant(type_id));
    //     }
    // }

    // serialize key type id
    u8 key_type_id = variant_to_type_id(m_cells[0].key);
    db_serialize(key_type_id, buf, offset);

    if constexpr (std::is_same<T, Vec<Vari> >::value) {
        // serialize num types
        u8 numTypes = m_cells[0].value.size();
        db_serialize(numTypes, buf, offset);

        // serialize tuple types
        u8 type_id;
        for (int i = 0; i < m_cells[0].value.size(); i++) {
            type_id = variant_to_type_id(m_cells[0].value[i]);
            db_serialize(type_id, buf, offset);
        }
    }

    // serialize cells
    for (int i = 0; i < numCells; i++) {
        const auto &cell = m_cells[i];

        db_serialize(cell.key, buf, offset);

        if constexpr (std::is_same<T, Vec<Vari> >::value)
            for (int j = 0; j < m_cells[0].value.size(); ++j)
                db_serialize(cell.value[j], buf, offset);
        else
            db_serialize(cell.value, buf, offset);
    }

    if (!m_leaf)
        for (int i = 0; i < numCells + 1; i++)
            db_serialize(m_children[i], buf, offset);
}

template<typename T>
BtreeNodePage<T>::BtreeNodePage(u16 deg, u32 parentID, bool is_root, bool is_leaf, u32 pageID)
        : Page(pageID), m_leaf(is_leaf), m_root(is_root), m_degree(deg), m_parentID(parentID),
          m_children(0), m_cells(0) {
}

} // namespace backend

#endif //KNDB_BTREENODEPAGE_TPP
