//
// Created by Kylan Chen on 3/3/25.
//
#ifndef KNDB_BTREENODEPAGE_TPP
#define KNDB_BTREENODEPAGE_TPP

#include "BtreeNodePage.hpp"
#include "utility.hpp"
#include "assume.h"

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
BtreeNodePage<T>::BtreeNodePage(u16 deg, pgid_t parentID, bool is_root, bool is_leaf, pgid_t pageID)
        : Page(pageID), m_leaf(is_leaf), m_root(is_root), m_degree(deg), m_parentID(parentID),
          m_children(0), m_cells(0) {
}

template<typename T>
BtreeNodePage<T>::BtreeNodePage(std::span<const byte> bytes, pgid_t pageID) : Page(pageID) {
    static_assert(std::is_trivially_copyable_v<T> || std::is_same_v<Vec<Vari>, T>);
    ASSUME_S(bytes.size() == cts::PG_SZ, "Buffer is incorrectly sized");
    offset_t offset = 0;

    u8 page_type_id;
    db_deserialize(page_type_id, bytes, offset);
    ASSUME_S(page_type_id == cts::pg_type_id::BTREE_NODE_PAGE, "Page_type_id is incorrect type");

    db_deserialize(m_degree, bytes, offset);
    db_deserialize(m_parentID, bytes, offset);
    db_deserialize(m_leaf, bytes, offset);
    db_deserialize(m_root, bytes, offset);

    cellid_t numCells;
    u8 numTypes, key_type_id;

    db_deserialize(numCells, bytes, offset);
    m_cells.resize(numCells);

    if (numCells == 0) return;

    Vari key;
    db_deserialize(key_type_id, bytes, offset);
    key = type_id_to_variant(key_type_id);

    Vec<Vari> types;
    if constexpr (std::is_same_v<T, Vec<Vari>>) {
        db_deserialize(numTypes, bytes, offset);

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
        if constexpr (std::is_same<T, Vec<Vari>>::value) {
            cell.value.resize(numTypes);
            for (int j = 0; j < numTypes; ++j)
                db_deserialize(cell.value[j], bytes, offset, types[j]);
        } else
            db_deserialize(cell.value, bytes, offset);
    }

    // serialize children
    if (!m_leaf)
        for (int i = 0; i < numCells + 1; i++) {
            childid_t child_id;
            db_deserialize(child_id, bytes, offset);
            m_children.push_back(child_id);
        }

    ASSUME_S(offset <= cts::PG_SZ, "Offset out of bounds");
}

template<typename T>
void BtreeNodePage<T>::toBytes(std::span<byte> buf) {
    ASSUME_S(buf.size() == cts::PG_SZ, "Buffer is incorrectly sized");
    ASSUME({
        if (m_leaf)
            return m_cells.empty() && m_children.empty();
        else
            return m_children.size() == cells.size() + 1 && !cells.empty();
    }, "Leaf node has incorrect number of cells/children");

    offset_t offset = 0;

    u8 page_type_id = cts::pg_type_id::BTREE_NODE_PAGE;
    db_serialize(page_type_id, buf, offset);

    db_serialize(m_degree, buf, offset);
    db_serialize(m_parentID, buf, offset);
    db_serialize(m_leaf, buf, offset);
    db_serialize(m_root, buf, offset);

    cellid_t numCells = m_cells.size();
    db_serialize(numCells, buf, offset);
    if (numCells == 0) return;

    u8 key_type_id = variant_to_type_id(m_cells[0].key);
    db_serialize(key_type_id, buf, offset);

    if constexpr (std::is_same<T, Vec<Vari> >::value) {
        u8 numTypes = m_cells[0].value.size();
        db_serialize(numTypes, buf, offset);

        u8 type_id;
        for (int i = 0; i < m_cells[0].value.size(); i++) {
            type_id = variant_to_type_id(m_cells[0].value[i]);
            db_serialize(type_id, buf, offset);
        }
    }

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

    ASSUME_S(offset <= cts::PG_SZ, "Offset out of bounds");
}

} // namespace backend

#endif //KNDB_BTREENODEPAGE_TPP
