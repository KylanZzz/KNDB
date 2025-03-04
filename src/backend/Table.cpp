//
// Created by Kylan Chen on 10/13/24.
//

#include <utility>

#include "Table.hpp"
#include "BtreePage.hpp"

#define T_PAGE m_pager.getPage<TablePage>(m_tablePageID)

// this assumes that table already exists
Table::Table(string name, Pager &pgr, size_t tablePageId) : m_pager(pgr), m_tablePageID
        (tablePageId), m_name(name) {}

// this is creating a new table
Table::Table(string name, Pager &pgr, size_t tablePageId, vector<variants> types) :
        m_pager(pgr), m_tablePageID(tablePageId), m_name(name) {

    m_btree = std::make_unique<Btree>(types, m_pager.createNewPage<BtreePage>().getPageID(), pgr);

    // initialize tablePage
    T_PAGE.init(types, m_btree->getRootPageID());
}

size_t Table::getNumTuples() {
    return T_PAGE.getNumTuples();
}

vector<variants> Table::getTypes() {
    return T_PAGE.getTypes();
}

string Table::getName() {
    return m_name;
}

void Table::createTuple(vector<variants> values) {
    size_t prev_id = m_btree->getRootPageID();
    m_btree->insert(values, values[0]);
    T_PAGE.addTuple();

    if (m_btree->getRootPageID() != prev_id)
        T_PAGE.setBtreePageID(m_btree->getRootPageID());
}

vector<variants> Table::readTuple(variants key) {
    return m_btree->search(std::move(key));
}

void Table::dropTable() {
    // ???
}

void Table::updateTuple(vector<variants> values) {
    m_btree->update(values, values[0]);
}

void Table::deleteTuple(variants key) {
    size_t prev_id = m_btree->getRootPageID();
    m_btree->remove(key);
    T_PAGE.removeTuple();

    if (m_btree->getRootPageID() != prev_id)
        T_PAGE.setBtreePageID(m_btree->getRootPageID());
}