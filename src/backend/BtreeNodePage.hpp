//
// Created by Kylan Chen on 3/3/25.
//

#ifndef KNDB_BTREENODEPAGE_HPP
#define KNDB_BTREENODEPAGE_HPP

#include "Page.hpp"
#include "kndb_types.hpp"

using namespace kndb_types;

class BtreeNodePage : public Page {
private:
    struct cell {
        variants key;
        vector<variants> tuple;
    };
public:
    BtreeNodePage(ByteVec &bytes, size_t pageID);

    BtreeNodePage(size_t minDeg, size_t parentID, bool is_root, bool is_leaf,
                  const vector<variants> &types, variants key, size_t pageID);

    vector<size_t> &getChildren() { return m_children; }

    vector<cell> &getCells() { return m_cells; }

    const vector<variants> &getTypes() { return m_types; }

    size_t getParentID() const { return m_parentID; }

    size_t getDegree() const { return m_degree; }

    bool isFull() { return m_cells.size() == 2 * m_degree - 1; }

    bool& isLeaf() { return m_leaf; }

    bool& isRoot() { return m_root; }

    void toBytes(ByteVec &vec) override;

private:
    size_t m_degree;
    size_t m_parentID;
    bool m_leaf;
    bool m_root;
    variants m_key;
    vector<variants> m_types;
    vector<size_t> m_children;
    vector<cell> m_cells;
};


#endif //KNDB_BTREENODEPAGE_HPP
