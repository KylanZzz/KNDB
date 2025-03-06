//
// Created by Kylan Chen on 10/13/24.
//

#include "Schema.hpp"
#include "BtreeNodePage.hpp"

#define S_PAGE m_pager.getPage<SchemaPage>(m_schemaPageID)

Schema::Schema(Pager &pgr, size_t schemaPageID) : m_pager(pgr), m_schemaPageID(schemaPageID) {
    // add existing tables
    for (const auto &[name, pageID]: S_PAGE.getTables())
        m_tables.emplace_back(std::make_unique<Table>(name, m_pager, pageID));
}

void Schema::createTable(string name, vector<variants> types) {
    if (name.length() + 1 > db_sizeof<string>())
        throw std::invalid_argument("Name is too long");

    if (name.empty())
        throw std::invalid_argument("Name cannot be empty");

    for (auto &table: m_tables)
        if (table->getName() == name)
            throw std::invalid_argument("Table with that name already exists");

    size_t tabID = m_pager.createNewPage<TablePage>(types, cts::SIZE_T_OUT_OF_BOUNDS).getPageID();
    S_PAGE.addTable(name, tabID);

    m_tables.emplace_back(std::make_unique<Table>(
            name,
            m_pager,
            tabID
    ));
}

vector<variants> Schema::getTableTypes(string table) {
    for (auto &tab: m_tables)
        if (tab->getName() == table) return tab->getTypes();

    throw std::invalid_argument("Table name not found in schema.");
}

void Schema::dropTable(string name) {
    int idx = -1;
    for (int i = 0; i < m_tables.size(); i++)
        if (m_tables[i]->getName() == name) idx = i;

    if (idx == -1)
        throw std::invalid_argument("Table name not found in schema.");

    S_PAGE.removeTable(name);

    m_tables[idx]->dropTable();
    m_tables.erase(m_tables.begin() + idx);
}

vector<string> Schema::getTableNames() {
    vector<string> res;
    for (auto &table: S_PAGE.getTables())
        res.push_back(table.first);
    return res;
}

void Schema::removeTuple(string table, variants key) {
    for (auto &tab: m_tables)
        if (tab->getName() == table) return tab->deleteTuple(key);

    throw std::invalid_argument("Table name not found in schema.");
}

void Schema::updateTuple(string table, vector<variants> values) {
    for (auto &tab: m_tables)
        if (tab->getName() == table)
            return tab->updateTuple(values);

    throw std::invalid_argument("Table name not found in schema.");
}

void Schema::insertTuple(string table, vector<variants> values) {
    for (auto &tab: m_tables)
        if (tab->getName() == table)
            return tab->createTuple(values);

    throw std::invalid_argument("Table name not found in schema.");
}

vector<variants> Schema::getTuple(string table, variants key) {
    for (auto &tab: m_tables)
        if (tab->getName() == table) return tab->readTuple(key);

    throw std::invalid_argument("Table name not found in schema.");
}

size_t Schema::getNumTuples(string table) {
    for (auto &tab: m_tables)
        if (tab->getName() == table) return tab->getNumTuples();

    throw std::invalid_argument("Table name not found in schema.");
}

bool Schema::sameTypes(vector<variants> vec1, vector<variants> vec2) {
    if (vec1.size() != vec2.size()) return false;
    for (int i = 0; i < vec1.size(); i++)
        if (variant_to_type_id(vec1[i]) != variant_to_type_id(vec2[i])) return false;
    return true;
}

