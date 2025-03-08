//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_TPP
#define KNDB_BTREE_TPP

#include "Btree.hpp"
#include "BtreeNodePage.hpp"

#define B_NODE(id) m_pager.getPage<BtreeNodePage<T>>(id)

template<typename T>
Btree<T>::Btree(size_t rootPageId, Pager &pgr) : m_rootPageID(rootPageId), m_pager(pgr) {}

template<typename T>
T Btree<T>::search(variants key) {
    RowPtr row = searchRowPtr(key, m_rootPageID);
    return B_NODE(row.pageID).getCells()[row.cellID].value;
}

template <typename T>
RowPtr Btree<T>::searchRowPtr(variants targ_key, size_t currPageID) {
    // 1. iterate through keys (linearly, for now) to look for key
    //      - if key == targ, return
    //      - if we have passed the key amount at cells[idx], we look at child[idx]
    //      - if reached end of node, search for rightmost child
    // how do we know if its out of bounds? if leaf, and not found

    auto& node = B_NODE(currPageID);
    auto& cells = node.getCells();
    auto& children = node.getChildren();
    bool leaf = B_NODE(currPageID).isLeaf();

    size_t idx = 0;
    while (idx < cells.size() && cells[idx].key < targ_key)
        idx++;

    if (targ_key == cells[idx].key)
        return {currPageID, idx};

    if (leaf) // leaf, and target key has not been found, so doesnt exist
        return {cts::SIZE_T_OUT_OF_BOUNDS, cts::SIZE_T_OUT_OF_BOUNDS};

    return searchRowPtr(targ_key, children[idx]);
}

template<typename T>
void Btree<T>::insert(T values, variants key) {

}

template<typename T>
void Btree<T>::remove(variants key) {

}

template<typename T>
void Btree<T>::update(T values, variants key) {
    RowPtr row = searchRowPtr(key, m_rootPageID);
    B_NODE(row.pageID).getCells()[row.cellID].value = values;
}

#endif //KNDB_BTREE_TPP