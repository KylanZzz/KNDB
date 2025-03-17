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
    Btree(u32 rootPageId, Pager &pgr, u16 degree);

    T search(const Vari &key);

    void insert(T values, Vari key);

    void remove(Vari key);

    void update(T values, const Vari &key);

    u32 getRootPage() const { return m_rootPageID; }

private:
    RowPos searchRowPtr(Vari targ_key, u32 currPageID);

    void split(u32 currPageID);

    Pager &m_pager;
    u32 m_rootPageID;
    u16 m_degree;
};

} // namespace backend

#include "Btree.tpp"

#endif //KNDB_BTREE_HPP
