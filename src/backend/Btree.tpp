//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_TPP
#define KNDB_BTREE_TPP

#include "Btree.hpp"
#include "BtreeNodePage.hpp"

#define B_NODE(id) m_pager.getPage<BtreeNodePage<T>>(id)
#define B_NEW(deg, par, root, leaf) m_pager.createNewPage<BtreeNodePage<T>>(deg, par, root, leaf);

template<typename T>
Btree<T>::Btree(u32 rootPageId, Pager &pgr, u16 degree) : m_rootPageID(rootPageId), m_pager
(pgr), m_degree(degree) {}

template <typename T>
RowPos Btree<T>::searchRowPtr(Vari targ_key, u32 currPageID) {
    // 1. iterate through keys (linearly, for now) to look for key
    //      - if key == targ, return
    //      - if we have passed the key amount at cells[idx], we look at child[idx]
    //      - if reached end of node, search for rightmost child
    // how do we know if its out of bounds? if leaf, and not found

    auto& node = B_NODE(currPageID);
    auto& cells = node.cells();
    auto& children = node.children();
    bool leaf = B_NODE(currPageID).leaf();

    assert (B_NODE(currPageID).leaf() || cells.size() == 0 || cells.size() == children.size() - 1);

    u32 idx = 0;
    while (idx < cells.size() && cells[idx].key < targ_key)
        idx++;

    if (idx < cells.size() && targ_key == cells[idx].key)
        return {currPageID, idx};

    if (leaf) // leaf, and target key has not been found, so doesn't exist
        return {currPageID, cts::U32_INVALID};

    // non-leaf node, so we search the children
    return searchRowPtr(targ_key, children[idx]);
}

template<typename T>
T Btree<T>::search(const Vari &key) {
    RowPos row = searchRowPtr(key, m_rootPageID);
    if (row.cellID == cts::U32_INVALID)
        throw std::invalid_argument("Row was not found in btree");
    return B_NODE(row.pageID).cells()[row.cellID].value;
}

template<typename T>
void Btree<T>::update(T values, const Vari &key) {
    RowPos row = searchRowPtr(key, m_rootPageID);
    if (row.cellID == cts::U32_INVALID)
        throw std::invalid_argument("Row was not found in btree");
    B_NODE(row.pageID).cells()[row.cellID].value = values;
}

template<typename T>
void Btree<T>::insert(T values, Vari key) {
    // 1. find node that cell belongs in
    //      1b. if node already contains the key, throw error
    RowPos row = searchRowPtr(key, m_rootPageID);
    if (row.cellID != cts::U32_INVALID)
        throw std::invalid_argument("Tuple with that key already exists");
    assert(B_NODE(row.pageID).leaf());

    // 2. insert into cell
    //      2b. find position that cell belongs in
    auto& cells = B_NODE(row.pageID).cells();
    u16 idx = 0;
    while (idx < cells.size() && cells[idx].key < key)
        idx++;
    cells.insert(cells.begin() + idx, {key, values});

    // 3. call split on the leaf we inserted into
    split(row.pageID);
}

template <typename T>
void Btree<T>::split(u32 currPageID) {
//    1. if node is NOT full, return (doesn't need to be split)
    auto& node = B_NODE(currPageID);
    auto& cells = node.cells();
    auto& children = node.children();

    if (cells.size() < node.maxKeys())
        return;

//    2. if NOT root
    if (!node.root()) {
        //    2b. find the current node page id in the parent's children[], store as IDX
        auto& parent = B_NODE(node.parent());
        auto& p_children = parent.children();
        auto& p_cells = parent.cells();
        u16 idx = cts::U16_INVALID;
        for (int i = 0; i < p_children.size(); i++) {
            if (p_children[i] == currPageID) {
                idx = i;
                break;
            }
        }
        assert (idx != cts::U16_INVALID);

        //    2c. move median cell of curr node to IDX position of parent node
        u16 median = cells.size() / 2;
        p_cells.insert(p_cells.begin() + idx, cells[median]);
        cells.erase(cells.begin() + median);

        //    2d. create new node right half of current nodes, and remove that half from og node
        //        - split both cells[] and children[] (only split children if not leaf)
        auto& newNode = B_NEW(m_degree, parent.getPageID(), false, node.leaf());
        auto& n_cells = newNode.cells();
        auto& n_children = newNode.children();
        // set right half of cells to new node
        n_cells.assign(cells.begin() + median, cells.end());
        // set left half of cells to old node (resizing is quicker)
        cells.resize(median);

        if (!node.leaf()) { // if not leaf, we need to copy over children as well
            n_children.assign(children.begin() + median + 1, children.end());
            // update parent ptrs of children
            for (auto pg: n_children)
                B_NODE(pg).setParent(newNode.getPageID());
            children.resize(median + 1);
        }

        //    2e. insert new pageID into parent node children[] (into the position
        //    where you deleted IDX originally (plus one for right node)
        p_children.insert(p_children.begin() + idx + 1, newNode.getPageID());
    } else {
        //    3. if IS root
        //        3b. create new node (new root) with middle cell of curr node as the single cell
        //             - update root
        auto& newRoot = B_NEW(m_degree, cts::U32_INVALID, true, false);
        m_rootPageID = newRoot.getPageID();
        node.setRoot(false);
        node.setParent(newRoot.getPageID());
        auto& p_children = newRoot.children();
        auto& p_cells = newRoot.cells();

        u16 median = cells.size() / 2;
        p_cells.push_back(cells[median]);
        cells.erase(cells.begin() + median);

        //    3c. create two new nodes with left and right half of current nodes
        //             - split both cells[] and children[]
        auto& newNode = B_NEW(m_degree, newRoot.getPageID(), false,
                                                                   B_NODE(currPageID).leaf());
        auto& n_cells = newNode.cells();
        auto& n_children = newNode.children();
        // set right half of cells to new node
        n_cells.assign(cells.begin() + median, cells.end());
        // set left half of cells to old node (resizing is quicker)
        cells.resize(median);

        if (!node.leaf()) { // if not leaf, we need to copy over children as well
            n_children.assign(children.begin() + median + 1, children.end());
            // update parent ptrs of children
            for (auto pg: n_children)
                B_NODE(pg).setParent(newNode.getPageID());
            children.resize(median + 1);
        }

        //    3d. insert new pageIDs into parent node children[]
        //             - (it should be empty so just insert anywhere)
        p_children.push_back(currPageID);
        p_children.push_back(newNode.getPageID());
    }

//    4. call split on parent (if not root)
    if (!node.root())
        split(node.parent());

}

template<typename T>
void Btree<T>::remove(Vari key) {

}

#endif //KNDB_BTREE_TPP