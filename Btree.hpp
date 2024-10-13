//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_BTREE_HPP
#define KNDB_BTREE_HPP

#include <vector>
#include <Pager.hpp>
#include <HeapFileManager.hpp>
#include <constants.hpp>

using std::byte;
using variants = std::variant<int, char, bool>;
template<typename T>
using array = std::array<T, Constants::PG_SZ>;
using std::vector;

class Btree {
public:
    Btree(vector<variants> types, size_t PKidx, Pager &pgr, HeapFileManager &hfm) : pager(pgr), heapFileManager(hfm) {};
private:
    Pager pager;
    HeapFileManager heapFileManager;
};


#endif //KNDB_BTREE_HPP
