//
// Created by Kylan Chen on 12/20/24.
//

#include <cassert>

#include "SchemaPage.hpp"
#include "utility.hpp"

/*
Info needed to be stored:
    - # Tables
        - Table names
        - # Types in table
        - Table types
            - IE: string, int, string
        - Table pageID

Format:
    size_t # Tables --- string name1 ---
    size_t # types --- size_t type1 ---
    size_t type2 --- size_t type3 ---
    size_t table1PageId --- string name2
    --- size_t # types --- size_t type1
    --- size_t type 2 --- size_t
    table2PageId --- etc.

*/
SchemaPage::SchemaPage(ByteVec &bytes, size_t pageID) : Page(pageID) {
    size_t offset = 0;

    size_t page_type_id, num_tables;
    serialize(page_type_id, bytes, offset);

    // check if page type is correct
    if (page_type_id != get_page_type_id<SchemaPage>())
        throw std::runtime_error("page_type_id does not match any valid page type");

    // deserialize # of tables
    serialize(num_tables, bytes, offset);

    // deserialize each table individually
    for (int i = 0; i < num_tables; ++i) {
        table_descriptor tab_desc;

        serialize(tab_desc.name, bytes, offset);
        serialize(tab_desc.pageID, bytes, offset);

        m_tables.push_back(tab_desc);
    }

    assert (offset <= cts::PG_SZ);
}

SchemaPage::SchemaPage(size_t pageID) : Page(pageID), m_tables(0) {}

size_t SchemaPage::getNumTables() {
    return m_tables.size();
}

std::unordered_map<string, size_t> SchemaPage::getTables() {
    std::unordered_map<string, size_t> res;
    for (const auto &table: m_tables)
        res[table.name] = table.pageID;
    return res;
}

size_t SchemaPage::freeSpace() {
    size_t used = 0;
    used += db_sizeof<size_t>(); // page_type_id
    used += db_sizeof<size_t>(); // # tables

    // table info
    for (const auto &table: m_tables) {
        used += db_sizeof<>(table.name);
        used += db_sizeof<>(table.pageID);
    }

    return cts::PG_SZ - used;
}

void SchemaPage::toBytes(ByteVec &vec) {
    assert(vec.size() == cts::PG_SZ);

    size_t offset = 0;

    // deserialize page_type_id
    size_t page_type_id = get_page_type_id<SchemaPage>();
    deserialize(page_type_id, vec, offset);

    // deserialize # of tables
    size_t num_tables = m_tables.size();
    deserialize(num_tables, vec, offset);

    // deserialize each table
    for (const auto &tab_desc: m_tables) {
        deserialize(tab_desc.name, vec, offset);

        deserialize(tab_desc.pageID, vec, offset);
    }
}

void SchemaPage::addTable(string name, size_t pageID) {
    if (name.length() + 1 > db_sizeof<string>())
        throw std::invalid_argument("Name is too long");

    if (name.empty())
        throw std::invalid_argument("Name cannot be empty");

    for (int i = 0; i < m_tables.size(); ++i) {
        if (m_tables[i].name == name) {
            throw std::invalid_argument("Table name already exists");
        }
    }

    // check if we have enough space to store table
    table_descriptor new_table{std::move(name), pageID};

    if (freeSpace() < db_sizeof<string>() + db_sizeof<size_t>())
        throw std::runtime_error("Not enough space in page to add table");

    m_tables.push_back(std::move(new_table));
}

void SchemaPage::removeTable(string targ_name) {
    if (targ_name.length() + 1 > db_sizeof<string>())
        throw std::invalid_argument("Name is too long");

    if (targ_name.empty())
        throw std::invalid_argument("Name cannot be empty");

    for (int i = 0; i < m_tables.size(); ++i) {
        if (m_tables[i].name == targ_name) {
            m_tables.erase(m_tables.begin() + i);
            return;
        }
    }

    throw std::invalid_argument("target name was not found in tables list");
}
