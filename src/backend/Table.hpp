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
    // Constructs a Table from an existing TablePage
    Table(string name, Pager& pgr, size_t tablePageId);

    // Constructs a new Table with the given TablePage and list of types. It assumes that
    // TablePage has been created but NOT initialized yet.
    Table(string name, Pager& pgr, size_t tablePageId, vector<variants> types);

    // deletes itself
    void dropTable();

    string getName();

    size_t getNumTuples();

    //CRUD
    void createTuple(vector<variants> values);

    vector<variants> readTuple(variants key);

    void updateTuple(vector<variants> values);

    void deleteTuple(variants key);

    vector<variants> getTypes();

private:
    Pager& m_pager;
    std::unique_ptr<Btree> m_btree;
    size_t m_tablePageID;
    string m_name;
};


#endif //KNDB_TABLE_HPP
