//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_HPP
#define KNDB_BTREE_HPP

#include "Pager.hpp"

template <typename T>
class Btree {
public:
    Btree(size_t rootPageId, Pager &pgr, size_t degree);

    T search(const Vari &key);

    void insert(T values, Vari key);

    void remove(Vari key);

    void update(T values, const Vari &key);

    size_t getRootPage() const {return m_rootPageID;}

private:
    RowPtr searchRowPtr(Vari key, size_t currPageID);
    void split(size_t currPageID);

    Pager& m_pager;
    size_t m_rootPageID;
    size_t m_degree;
};

#include "Btree.tpp"

#endif //KNDB_BTREE_HPP
