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
template <typename T>
class BtreeNodePage : public Page {
private:
    struct cell {
        variants key;
        T value;
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
     * @param deg Degree of the B-tree.
     * @param parentID Parent node ID.
     * @param is_root Indicates if this node is the root.
     * @param is_leaf Indicates if this node is a leaf.
     * @param pageID Page ID of this node.
     */
    BtreeNodePage(size_t deg, size_t parentID, bool is_root, bool is_leaf, size_t pageID);

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
     * @brief Retrieves the parent node ID.
     * @return The parent node ID.
     */
    size_t getParentID() const { return m_parentID; }

    /**
     * @brief Retrieves the minimum number of keys a node can contain.
     * @return The min number of keys.
     */
    size_t getMinKeys() const { return m_degree - 1; }

    /**
     * @brief Retrieves the maximum number of keys a node can contain.
     * @return The max number of keys.
     */
    size_t getMaxKeys() const { return 2 * m_degree - 1; }

    /**
     * @brief Checks if the node is a leaf.
     * @return Reference to the boolean indicating if the node is a leaf.
     */
    bool &isLeaf() { return m_leaf; }

    /**
     * @brief Checks if the node is the root.
     * @return Reference to the boolean indicating if the node is the root.
     */
    bool &isRoot() { return m_root; }

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
    vector<size_t> m_children;
    vector<cell> m_cells;
};

#include "BtreeNodePage.tpp"

#endif //KNDB_BTREENODEPAGE_HPP
