//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_TABLE_HPP
#define KNDB_TABLE_HPP

#include <vector>
#include "Pager.hpp"
#include "Btree.hpp"
#include "kndb_types.hpp"

using namespace kndb_types;

class Table {
public:
    Table(string tableName, vector<variants> types, Pager& pgr, size_t
    tablePageId) {
        size_t rootPageId = 1;
        auto btree = Btree(types, rootPageId, pgr);
    };

private:
    string name;
};


#endif //KNDB_TABLE_HPP
