//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_SCHEMA_HPP
#define KNDB_SCHEMA_HPP

#include <Pager.hpp>
#include <HeapFileManager.hpp>
#include <Table.hpp>

using std::byte;
using variants = std::variant<int, char, bool>;
template<typename T>
using array = std::array<T, Constants::PG_SZ>;
using std::vector;

class Schema {
public:
    Schema(Pager& pgr, HeapFileManager& hfm) : pager(pgr), heapFileManager(hfm) {};

    void createTable(std::string name, vector<variants> types) {
        Table table("table 1", types, pager, heapFileManager);
    }

private:
    Pager& pager;
    HeapFileManager& heapFileManager;
};


#endif //KNDB_SCHEMA_HPP
