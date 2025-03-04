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
    // this assumes that table already exists
    Table(string tableName, Pager& pgr, size_t tablePageId);

    // this is creating a new table
    Table(string tableName, Pager& pgr, size_t tablePageId, vector<variants> types);

    // deletes itself
    void dropTable();

    // get name of table
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
    string m_name;
    size_t m_tablePageID;
};


#endif //KNDB_TABLE_HPP
