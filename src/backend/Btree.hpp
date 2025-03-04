//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_HPP
#define KNDB_BTREE_HPP

#include <vector>

#include "Pager.hpp"
#include "utility.hpp"

class Btree {
public:
    Btree(vector<variants> types, size_t rootPageId, Pager &pgr);

    vector<variants> search(variants key);

    void insert(vector<variants> values, variants key);

    void remove(variants key);

    void update(vector<variants> values, variants key);

    size_t getRootPageID();
private:
    Pager& pager;
};

#endif //KNDB_BTREE_HPP
