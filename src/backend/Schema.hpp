//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_SCHEMA_HPP
#define KNDB_SCHEMA_HPP

#include "Pager.hpp"
#include "Table.hpp"

class Schema {
public:
    Schema(Pager& pgr, size_t metaInfoPageId) : pager(pgr) {};

    void createTable(std::string name, vector<variants> types) {
        size_t tablePageId = 10;
        Table table(name, types, pager, tablePageId);
    }

private:
    Pager& pager;
};


#endif //KNDB_SCHEMA_HPP
