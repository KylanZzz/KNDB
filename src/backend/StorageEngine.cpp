//
// Created by Kylan Chen on 10/13/24.
//

#include "StorageEngine.hpp"

#include <Btree.hpp>
#include "utility.hpp"
#include "SchemaPage.hpp"

#define S_PAGE m_pager.getPage<SchemaPage>(m_schemaPageID)

StorageEngine::StorageEngine(Pager &pgr, size_t schemaPageID) : m_pager(pgr), m_schemaPageID(schemaPageID) {
    // add existing tables
    for (const auto &[name, pageID]: S_PAGE.getTables())
        m_tables.emplace_back(std::make_unique<Table>(name, m_pager, pageID));
}

void StorageEngine::createTable(const String& name, const Vec<Vari>& types) {
    if (name.length() + 1 > db_sizeof<String>())
        throw std::invalid_argument("Name is too long");

    if (name.empty())
        throw std::invalid_argument("Name cannot be empty");

    for (const auto &table: m_tables)
        if (table->getName() == name)
            throw std::invalid_argument("Table with that name already exists");

    m_tables.emplace_back(std::make_unique<Table>(name,m_pager, types));

    S_PAGE.addTable(m_tables.back()->getName(), m_tables.back()->getTablePageID());
}

Vec<Vari> StorageEngine::getTableTypes(const String& table) {
    for (const auto &tab: m_tables)
        if (tab->getName() == table) return tab->getTypes();

    throw std::invalid_argument("Table name not found in schema.");
}

void StorageEngine::dropTable(const String& name) {
    int idx = -1;
    for (int i = 0; i < m_tables.size(); i++)
        if (m_tables[i]->getName() == name) idx = i;

    if (idx == -1)
        throw std::invalid_argument("Table name not found in schema.");

    S_PAGE.removeTable(name);

    m_tables[idx]->drop();
    m_tables.erase(m_tables.begin() + idx);
}

Vec<String> StorageEngine::getTableNames() const {
    Vec<String> res;
    for (auto &table: S_PAGE.getTables())
        res.push_back(table.first);
    return std::move(res);
}

void StorageEngine::removeTuple(const String& table, const Vari& key) {
    for (auto &tab: m_tables)
        if (tab->getName() == table) return tab->deleteTuple(key);

    throw std::invalid_argument("Table name not found in schema.");
}

void StorageEngine::updateTuple(const String& table, const Vec<Vari>& values) {
    for (const auto &tab: m_tables)
        if (tab->getName() == table)
            return tab->updateTuple(values);

    throw std::invalid_argument("Table name not found in schema.");
}

void StorageEngine::insertTuple(const String& table, const Vec<Vari>& values) {
    for (auto &tab: m_tables)
        if (tab->getName() == table)
            return tab->insertTuple(values);

    throw std::invalid_argument("Table name not found in schema.");
}

Vec<Vari> StorageEngine::getTuple(const String& table, const Vari& key) {
    for (auto &tab: m_tables)
        if (tab->getName() == table) return tab->readTuple(key);

    throw std::invalid_argument("Table name not found in schema.");
}

size_t StorageEngine::getNumTuples(const String& table) {
    for (const auto &tab: m_tables)
        if (tab->getName() == table) return tab->getNumTuples();

    throw std::invalid_argument("Table name not found in schema.");
}


