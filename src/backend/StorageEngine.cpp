//
// Created by Kylan Chen on 10/13/24.
//

#include "StorageEngine.hpp"

#include "Btree.hpp"
#include "utility.hpp"
#include "SchemaPage.hpp"

#define S_PAGE m_pager.getPage<SchemaPage>(m_schemaPageID)

namespace backend {
StorageEngine::StorageEngine(Pager &pgr, pgid_t schemaPageID) : m_pager(pgr),
                                                                m_schemaPageID(schemaPageID) {
    // add existing tables
    for (const auto &[name, pageID]: S_PAGE.getTables())
        m_tables.emplace_back(std::make_unique<Table>(name, m_pager, pageID));
}

void StorageEngine::createTable(const string &tableName, const Vec<Vari> &types) {
    if (tableName.length() + 1 > db_sizeof<string>())
        throw std::invalid_argument("Name is too long");

    if (tableName.empty())
        throw std::invalid_argument("Name cannot be empty");

    for (const auto &table: m_tables)
        if (table->getName() == tableName)
            throw std::invalid_argument("Table with that name already exists");

    m_tables.emplace_back(std::make_unique<Table>(tableName, m_pager, types));

    S_PAGE.addTable(m_tables.back()->getName(), m_tables.back()->getTablePageID());
}

Vec<Vari> StorageEngine::getTableTypes(const string &tableName) const {
    for (const auto &tab: m_tables)
        if (tab->getName() == tableName) return tab->getTypes();

    throw std::invalid_argument("Table name not found in schema.");
}

void StorageEngine::dropTable(const string &tableName) {
    int idx = -1;
    for (int i = 0; i < m_tables.size(); i++)
        if (m_tables[i]->getName() == tableName) idx = i;

    if (idx == -1)
        throw std::invalid_argument("Table name not found in schema.");

    S_PAGE.removeTable(tableName);

    m_tables[idx]->drop();
    m_tables.erase(m_tables.begin() + idx);
}

Vec<string> StorageEngine::getTableNames() const {
    Vec<string> res;
    for (auto &table: S_PAGE.getTables())
        res.push_back(table.first);
    return std::move(res);
}

void StorageEngine::removeTuple(const string &tableName, const Vari &key) const {
    for (auto &tab: m_tables)
        if (tab->getName() == tableName) return tab->deleteTuple(key);

    throw std::invalid_argument("Table name not found in schema.");
}

void StorageEngine::updateTuple(const string &tableName, const Vec<Vari> &values) const {
    for (const auto &tab: m_tables)
        if (tab->getName() == tableName)
            return tab->updateTuple(values);

    throw std::invalid_argument("Table name not found in schema.");
}

void StorageEngine::insertTuple(const string &tableName, const Vec<Vari> &values) const {
    for (auto &tab: m_tables)
        if (tab->getName() == tableName)
            return tab->insertTuple(values);

    throw std::invalid_argument("Table name not found in schema.");
}

Vec<Vari> StorageEngine::getTuple(const string &tableName, const Vari &key) const {
    for (auto &tab: m_tables)
        if (tab->getName() == tableName) return tab->readTuple(key);

    throw std::invalid_argument("Table name not found in schema.");
}

u64 StorageEngine::getNumTuples(const string &tableName) const {
    for (const auto &tab: m_tables)
        if (tab->getName() == tableName) return tab->getNumTuples();

    throw std::invalid_argument("Table name not found in schema.");
}
} // namespace backend
