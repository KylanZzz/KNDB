//
// Created by Kylan Chen on 3/3/25.
//

#ifndef KNDB_BTREENODEPAGE_HPP
#define KNDB_BTREENODEPAGE_HPP

#include "Page.hpp"
#include "kndb_types.hpp"

using namespace kndb_types;

/**
 * @class BtreeNodePage
 * @brief Represents a node in a B-tree structure stored in a database.
 *
 * This class manages keys, child pointers, and tuples within a B-tree node.
 */
class BtreeNodePage : public Page {
private:
    struct cell {
        variants key;
        vector<variants> tuple;
    };
public:
    /**
     * @brief Constructs a BtreeNodePage from serialized data.
     * @param bytes Serialized data.
     * @param pageID The page ID.
     */
    BtreeNodePage(ByteVec &bytes, size_t pageID);

    /**
     * @brief Constructs a new B-tree node.
     * @param minDeg Minimum degree of the B-tree.
     * @param parentID Parent node ID.
     * @param is_root Indicates if this node is the root.
     * @param is_leaf Indicates if this node is a leaf.
     * @param types Data types stored in this node.
     * @param key Key associated with this node.
     * @param pageID Page ID of this node.
     */
    BtreeNodePage(size_t minDeg, size_t parentID, bool is_root, bool is_leaf,
                  const vector<variants> &types, variants key, size_t pageID);

    /**
     * @brief Retrieves child node IDs of this Btree node.
     * @return A list of pageIDs to children nodes.
     */
    vector<size_t> &getChildren() { return m_children; }

    /**
     * @brief Retrieves all the stored key-tuple cells in the node.
     * @return A list of key-tuple pairs that represent a row in the database.
     */
    vector<cell> &getCells() { return m_cells; }

    /**
     * @brief Retrieves the types stored in each tuple.
     * @return A list of types.
     */
    const vector<variants> &getTypes() { return m_types; }

    /**
     * @brief Retrieves the parent node ID.
     * @return The parent node ID.
     */
    size_t getParentID() const { return m_parentID; }

    /**
     * @brief Retrieves the B-tree node degree.
     * @return The degree of the node.
     */
    size_t getDegree() const { return m_degree; }

    /**
     * @brief Checks if the node is full.
     * @return True if the node is full, false otherwise.
     */
    bool isFull() { return m_cells.size() >= 2 * m_degree - 1; }

    /**
     * @brief Checks if the node is a leaf.
     * @return Reference to the boolean indicating if the node is a leaf.
     */
    bool& isLeaf() { return m_leaf; }

    /**
     * @brief Checks if the node is the root.
     * @return Reference to the boolean indicating if the node is the root.
     */
    bool& isRoot() { return m_root; }

    /**
     * @brief Serializes the B-tree node into a byte vector.
     * @param vec The byte vector to store serialized data.
     */
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
