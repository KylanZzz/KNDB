//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_HPP
#define KNDB_BTREE_HPP

#include "Pager.hpp"

namespace backend {

/**
 * @class Btree
 * @brief Represents a generic B-tree supporting insertion, deletion, update, and search operations.
 *
 * This class implements a B-tree with a specified degree. It operates on pages managed by a Pager,
 * storing key-value pairs persistently. Keys are of type 'variant' and values are of type 'T'.
 *
 * All keys must be unique within the B-tree. Duplicate insertions or operations on missing keys
 * will result in exceptions being thrown.
 *
 * @tparam T The type of values stored in the B-tree. This can be any standard type (e.g., int, double, std::string),
 * or a 'vector<variant>' representing a row of structured values. The supported variant types
 * are defined in 'kndb_types.hpp'.
 */
template<typename T>
class Btree {
public:
    /**
     * @brief Constructs a B-tree.
     * @param rootPageId The page ID of the root node in the tree.
     * @param pgr Reference to the Pager used for managing pages.
     * @param degree Degree of the B-tree (t). Each node will store between t-1 and 2t-1 keys.
     */
    Btree(pgid_t rootPageId, Pager &pgr, degree_t degree);

    /**
     * @brief Searches for a key in the B-tree.
     * @param key The key to search for.
     * @return The value associated with the key.
     * @throws std::invalid_argument if the key does not exist in the tree.
     */
    T search(const Vari &key);

    /**
     * @brief Inserts a key-value pair into the B-tree.
     * @param values The value to associate with the key.
     * @param key The key to insert.
     * @throws std::invalid_argument if the key already exists in the tree.
     */
    void insert(T values, Vari key);

    /**
     * @brief Removes a key-value pair from the B-tree.
     * @param key The key to remove.
     * @throws std::invalid_argument if the key does not exist in the tree.
     */
    void remove(Vari key);

    /**
     * @brief Updates the value associated with an existing key.
     * @param values The new value to assign.
     * @param key The key to update.
     * @throws std::invalid_argument if the key does not exist in the tree.
     */
    void update(T values, const Vari &key);

    /**
     * @brief Returns the root page ID of the B-tree.
     * @return The page ID of the root node.
     */
    pgid_t getRootPage() const { return m_rootPageID; }

    /**
     * Deletes the Btree and all its nodes.
     */
    void deleteTree();

private:
    RowPos searchRowPtr(Vari targ_key, pgid_t currPageID);

    void split(pgid_t currPageID);

    Pager &m_pager;
    pgid_t m_rootPageID;
    degree_t m_degree;
};

} // namespace backend

#include "Btree.tpp"

#endif //KNDB_BTREE_HPP
