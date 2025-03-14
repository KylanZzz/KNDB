//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_HPP
#define KNDB_BTREE_HPP

#include <vector>

#include "Pager.hpp"
#include "utility.hpp"

template <typename T>
class Btree {
public:
    Btree(size_t rootPageId, Pager &pgr, size_t degree);

    T search(variants key);

    void insert(T values, variants key);

    void remove(variants key);

    void update(T values, variants key);

    size_t getRootPage() {return m_rootPageID;}

private:
    RowPtr searchRowPtr(variants key, size_t currPageID);
    void split(size_t currPageID);

    Pager& m_pager;
    size_t m_rootPageID;
    size_t m_degree;
};

#include "Btree.tpp"

#endif //KNDB_BTREE_HPP
