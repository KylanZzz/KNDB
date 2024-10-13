//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_TABLE_HPP
#define KNDB_TABLE_HPP

#include <vector>
#include <Pager.hpp>
#include <HeapFileManager.hpp>
#include <constants.hpp>
#include <Btree.hpp>

using std::byte;
using variants = std::variant<int, char, bool>;
template <typename T>
using array = std::array<T, Constants::PG_SZ>;
using std::vector;

class Table {
public:
    Table(std::string tableName, vector<variants> types, Pager& pgr, HeapFileManager& hfm) {
        auto btree = Btree(types, 1, pgr, hfm);
    };
private:

};


#endif //KNDB_TABLE_HPP
