//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_TABLE_HPP
#define KNDB_TABLE_HPP

#include <vector>
#include "Pager.hpp"
#include "utility.hpp"
#include "Btree.hpp"

class Table {
public:
    Table(std::string tableName, vector<variants> types, Pager& pgr, size_t tablePageId) {
        size_t rootPageId = 1;
        auto btree = Btree(types, rootPageId, pgr);
    };

private:
    std::string name;
};


#endif //KNDB_TABLE_HPP
