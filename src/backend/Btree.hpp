//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_HPP
#define KNDB_BTREE_HPP

#include "Pager.hpp"

namespace backend {

template<typename T>
class Btree {
public:
    Btree(pgid_t rootPageId, Pager &pgr, degree_t degree);

    T search(const Vari &key);

    void insert(T values, Vari key);

    void remove(Vari key);

    void update(T values, const Vari &key);

    pgid_t getRootPage() const { return m_rootPageID; }

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
