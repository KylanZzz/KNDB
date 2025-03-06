//
// Created by Kylan Chen on 3/3/25.
//

#ifndef KNDB_BTREENODEPAGE_HPP
#define KNDB_BTREENODEPAGE_HPP

#include "Page.hpp"
#include "kndb_types.hpp"

using namespace kndb_types;

// children vec<PageID> -> list of all children
// cells vec<pair<variant, vec<variants>>> -> each cell contains a [key, value], should be in sorted order
// size_t minDeg (t)
// size_t numKeys (n)
// bool isLeaf

// int *keys;
//int t;
//TreeNode **C;
//int n;
//bool leaf;
class BtreeNodePage : public Page {
private:
    struct cell {
        variants key;
        vector<variants> tuple;
    };
public:
    BtreeNodePage(ByteVec &bytes, size_t pageID);

    BtreeNodePage(size_t minDeg, size_t parentID, bool is_root, bool is_leaf,
                  const vector<variants> &types, size_t pageID);

    vector<size_t> &getChildrenPages() { return m_children; }

    vector<cell> &getCells() { return m_cells; }

    vector<variants> &getTypes() { return m_types; }

    size_t getParentID() { return m_parentID; }

    size_t getDegree() { return m_degree; }

    bool isFull() { return (m_cells.size() == 2 * m_degree - 1); }

    bool isLeaf() { return m_isLeaf; }

    bool isRoot() { return m_isRoot; }

    void toBytes(ByteVec &vec) override;

private:
    size_t m_degree;
    size_t m_parentID;
    bool m_isLeaf;
    bool m_isRoot;
    variants m_key;
    vector<variants> m_types;
    vector<size_t> m_children;
    vector<cell> m_cells;
};


#endif //KNDB_BTREENODEPAGE_HPP
