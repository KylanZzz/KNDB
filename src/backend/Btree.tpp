//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_TPP
#define KNDB_BTREE_TPP

#include "Btree.hpp"
#include "BtreeNodePage.hpp"

#define B_NODE(id) m_pager.getPage<BtreeNodePage<T>>(id)

template<typename T>
Btree<T>::Btree(size_t rootPageId, Pager &pgr, size_t degree) : m_rootPageID(rootPageId), m_pager
(pgr), m_degree(degree) {}

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

    assert (B_NODE(currPageID).isLeaf() || cells.size() == 0 || cells.size() == children.size() - 1);

    size_t idx = 0;
    while (idx < cells.size() && cells[idx].key < targ_key)
        idx++;

    if (idx < cells.size() && targ_key == cells[idx].key)
        return {currPageID, idx};

    if (leaf) // leaf, and target key has not been found, so doesnt exist
        return {currPageID, cts::SIZE_T_OUT_OF_BOUNDS};

    // non-leaf node, so we search the children
    return searchRowPtr(targ_key, children[idx]);
}

template<typename T>
T Btree<T>::search(variants key) {
    RowPtr row = searchRowPtr(key, m_rootPageID);
    if (row.cellID == cts::SIZE_T_OUT_OF_BOUNDS)
        throw std::invalid_argument("Row was not found in btree");
    return B_NODE(row.pageID).getCells()[row.cellID].value;
}

template<typename T>
void Btree<T>::update(T values, variants key) {
    RowPtr row = searchRowPtr(key, m_rootPageID);
    if (row.cellID == cts::SIZE_T_OUT_OF_BOUNDS)
        throw std::invalid_argument("Row was not found in btree");
    B_NODE(row.pageID).getCells()[row.cellID].value = values;
}

template<typename T>
void Btree<T>::insert(T values, variants key) {
    // 1. find node that cell belongs in
    //      1b. if node already contains the key, throw error
    RowPtr row = searchRowPtr(key, m_rootPageID);
    if (row.cellID != cts::SIZE_T_OUT_OF_BOUNDS)
        throw std::invalid_argument("Tuple with that key already exists");
    assert(B_NODE(row.pageID).isLeaf());

    // 2. insert into cell
    //      2b. find position that cell belongs in
    auto& cells = B_NODE(row.pageID).getCells();
    size_t idx = 0;
    while (idx < cells.size() && cells[idx].key < key)
        idx++;
    cells.insert(cells.begin() + idx, {key, values});

    // 3. call split on the leaf we inserted into
    split(row.pageID);
    int a;
}

template <typename T>
void Btree<T>::split(size_t currPageID) {
//    1. if node is NOT full, return (doesnt need to be split)
    auto& cells = B_NODE(currPageID).getCells();
    auto& children = B_NODE(currPageID).getChildren();

    if (cells.size() < B_NODE(currPageID).getMaxKeys())
        return;

    bool currIsRoot = B_NODE(currPageID).isRoot();

//    2. if NOT root
    if (!B_NODE(currPageID).isRoot()) {
        //    2b. find the current node page id in the parent's children[], store as IDX
        size_t parentID = B_NODE(currPageID).getParentID();
        auto& p_children = B_NODE(parentID).getChildren();
        auto& p_cells = B_NODE(parentID).getCells();
        int idx = -1;
        for (int i = 0; i < p_children.size(); i++) {
            if (p_children[i] == currPageID) {
                idx = i;
                break;
            }
        }
        assert (idx != -1);

        //    2c. move median cell of curr node to IDX position of parent node
        size_t median = cells.size() / 2;
        p_cells.insert(p_cells.begin() + idx, cells[median]);
        cells.erase(cells.begin() + median);

        //    2d. create new node right half of current nodes, and remove that half from og node
        //        - split both cells[] and children[] (only split children if not leaf)
        size_t newPageID = m_pager.createNewPage<BtreeNodePage<T>>(m_degree, parentID, false,
                B_NODE(currPageID).isLeaf()).getPageID();
        auto& n_cells = B_NODE(newPageID).getCells();
        auto& n_children = B_NODE(newPageID).getChildren();
        // set right half of cells to new node
        n_cells.assign(cells.begin() + median, cells.end());
        // set left half of cells to old node (resizing is quicker)
        cells.resize(median);

        if (!B_NODE(currPageID).isLeaf()) { // if not leaf, we need to copy over children as well
            n_children.assign(children.begin() + median + 1, children.end());
            // update parent ptrs of children
            for (auto pg: n_children)
                B_NODE(pg).setParent(newPageID);
            children.resize(median + 1);
        }

        //    2e. insert new pageID into parent node children[] (into the position
        //    where you deleted IDX originally (plus one for right node)
        p_children.insert(p_children.begin() + idx + 1, newPageID);
    } else {
        //    3. if IS root
        //        3b. create new node (new root) with middle cell of curr node as the single cell
        //             - update root
        size_t newRootID = m_pager.createNewPage<BtreeNodePage<T>>(m_degree,
                cts::SIZE_T_OUT_OF_BOUNDS, true, false).getPageID();
        m_rootPageID = newRootID;
        B_NODE(currPageID).setRoot(false);
        B_NODE(currPageID).setParent(newRootID);
        auto& p_children = B_NODE(newRootID).getChildren();
        auto& p_cells = B_NODE(newRootID).getCells();

        size_t median = cells.size() / 2;
        p_cells.push_back(cells[median]);
        cells.erase(cells.begin() + median);

        //    3c. create two new nodes with left and right half of current nodes
        //             - split both cells[] and children[]
        size_t newPageID = m_pager.createNewPage<BtreeNodePage<T>>(m_degree, newRootID, false,
                                                                   B_NODE(currPageID).isLeaf()).getPageID();
        auto& n_cells = B_NODE(newPageID).getCells();
        auto& n_children = B_NODE(newPageID).getChildren();
        // set right half of cells to new node
        n_cells.assign(cells.begin() + median, cells.end());
        // set left half of cells to old node (resizing is quicker)
        cells.resize(median);

        if (!B_NODE(currPageID).isLeaf()) { // if not leaf, we need to copy over children as well
            n_children.assign(children.begin() + median + 1, children.end());
            // update parent ptrs of children
            for (auto pg: n_children)
                B_NODE(pg).setParent(newPageID);
            children.resize(median + 1);
        }

        //    3d. insert new pageIDs into parent node children[]
        //             - (it should be empty so just insert anywhere)
        p_children.push_back(currPageID);
        p_children.push_back(newPageID);
    }

//    4. call split on parent (if not root)
    if (!currIsRoot)
        split(B_NODE(currPageID).getParentID());

}

template<typename T>
void Btree<T>::remove(variants key) {

}

#endif //KNDB_BTREE_TPP