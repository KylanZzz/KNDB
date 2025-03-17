//
// Created by Kylan Chen on 3/3/25.
//

#ifndef KNDB_BTREENODEPAGE_HPP
#define KNDB_BTREENODEPAGE_HPP

#include "Page.hpp"
#include "kndb_types.hpp"

using namespace kndb;

/**
 * @class BtreeNodePage
 * @brief Represents a node in a B-tree structure stored in a database.
 * @tparam T Type stored by the Btree Node. Currently supported types include trivially copyable
 * types (structs used as data class included) and vector<variants> (representing tuples)
 *
 * This class manages keys, child pointers, and tuples within a B-tree node.
 *
 * Important assumptions made during deserialization and serialization:
 * 1. Leaf nodes have no children
 * 2. Non-leaf nodes have numCells + 1 children if numCells is nonzero, and 0 children otherwise.
 * 3. If the Btree Node stores tuples, each tuple has the same fixed number of attributes.
 * 4. The node has no more than 'getMaxKeys()' number of cells.
 */
template<typename T>
class BtreeNodePage : public Page {
public:
    struct cell {
        Vari key;
        T value;
    };

public:
    /**
     * @brief Constructs a BtreeNodePage from serialized data.
     * @param bytes Serialized data.
     * @param pageID The page ID.
     */
    BtreeNodePage(std::span<const byte> bytes, uint32_t pageID);

    /**
     * @brief Constructs a new B-tree node.
     * @param deg Degree of the B-tree.
     * @param parentID Parent node ID.
     * @param is_root Indicates if this node is the root.
     * @param is_leaf Indicates if this node is a leaf.
     * @param pageID Page ID of this node.
     */
    BtreeNodePage(uint16_t deg, uint32_t parentID, bool is_root, bool is_leaf, uint32_t pageID);

    /**
     * @brief Retrieves child node IDs of this Btree node.
     * @return A list of pageIDs to children nodes.
     */
    Vec<uint32_t> &children() { return m_children; }

    /**
     * @brief Retrieves all the stored key-tuple cells in the node.
     * @return A list of key-tuple pairs that represent a row in the database.
     */
    Vec<cell> &cells() { return m_cells; }

    /**
     * @brief Retrieves the parent node ID.
     * @return The parent node ID.
     */
    uint16_t parent() const { return m_parentID; }

    /**
     * @brief Retrieves the minimum number of keys a node can contain.
     * @return The min number of keys.
     */
    uint16_t minKeys() const { return m_degree - 1; }

    /**
     * @brief Retrieves the maximum number of keys a node can contain.
     * @return The max number of keys.
     */
    uint16_t maxKeys() const { return 2 * m_degree - 1; }

    /**
     * @brief Checks if the node is a leaf.
     * @return Reference to the boolean indicating if the node is a leaf.
     */
    bool leaf() const { return m_leaf; }

    /**
     * @brief Checks if the node is the root.
     * @return Reference to the boolean indicating if the node is the root.
     */
    bool root() const { return m_root; }

    /**
     * @brief Sets the root status of the node.
     * @param isRoot True if the node is a root, false otherwise.
     */
    void setRoot(bool isRoot) { m_root = isRoot; }

    /**
     * @brief Sets the leaf status of the node.
     * @param isLeaf True if the node is a leaf, false otherwise.
     */
    void setLeaf(bool isLeaf) { m_leaf = isLeaf; }

    /**
     * @brief Sets the parent node.
     * @param parent The page ID of the new parent node.
     */
    void setParent(uint32_t parent) { m_parentID = parent; }

    /**
     * @brief Serializes the B-tree node into a byte vector.
     * @param vec The byte vector to store serialized data.
     */
    void toBytes(std::span<byte> buffer) override;

private:
    uint16_t m_degree;
    uint32_t m_parentID;
    bool m_leaf;
    bool m_root;
    Vec<uint32_t> m_children;
    Vec<cell> m_cells;
};

#include "BtreeNodePage.tpp"

#endif //KNDB_BTREENODEPAGE_HPP
