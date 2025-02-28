//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_HPP
#define KNDB_BTREE_HPP

#include <vector>

#include "Pager.hpp"
#include "utility.hpp"

using std::byte;
using variants = std::variant<int, char, bool, std::string>;
using std::vector;

class Btree {
public:
    Btree(vector<variants> types, size_t rootPageId, Pager &pgr) : pager(pgr) {};
private:
    Pager& pager;
};

#endif //KNDB_BTREE_HPP
