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
SchemaPage::SchemaPage(std::span<const byte> bytes, u32 pageID) : Page(pageID) {
    u16 offset = 0;

    u8 page_type_id, num_tables;
    deserialize(page_type_id, bytes, offset);

    // check if page type is correct
    if (page_type_id != cts::pg_type_id::SCHEMA_PAGE)
        throw std::runtime_error("page type id is incorrect");

    // deserialize # of tables
    deserialize(num_tables, bytes, offset);

    // deserialize each table individually
    for (int i = 0; i < num_tables; ++i) {
        table_descriptor tab_desc;

        deserialize(tab_desc.name, bytes, offset);
        deserialize(tab_desc.pageID, bytes, offset);

        m_tables.push_back(tab_desc);
    }

    assert (offset <= cts::PG_SZ);
}

SchemaPage::SchemaPage(u32 pageID) : Page(pageID), m_tables(0) {}

u8 SchemaPage::getNumTables() const {
    return m_tables.size();
}

std::unordered_map<string, u32> SchemaPage::getTables() {
    std::unordered_map<string, u32> res;
    for (const auto &table: m_tables)
        res[table.name] = table.pageID;
    return res;
}

u16 SchemaPage::freeSpace() {
    u16 used = 0;
    used += db_sizeof<u8>(); // page_type_id
    used += db_sizeof<u8>(); // # tables

    // table info
    for (const auto &table: m_tables) {
        used += db_sizeof<>(table.name);
        used += db_sizeof<>(table.pageID);
    }

    return cts::PG_SZ - used;
}

void SchemaPage::addTable(string name, u32 pageID) {
    if (name.length() + 1 > db_sizeof<string>())
        throw std::invalid_argument("Name is too long");

    if (name.empty())
        throw std::invalid_argument("Name cannot be empty");

    for (auto & m_table : m_tables) {
        if (m_table.name == name) {
            throw std::invalid_argument("Table name already exists");
        }
    }

    // check if we have enough space to store table
    table_descriptor new_table{std::move(name), pageID};

    if (freeSpace() < db_sizeof<string>() + db_sizeof<u32>())
        throw std::runtime_error("Not enough space in page to add table");

    m_tables.push_back(std::move(new_table));
}

void SchemaPage::removeTable(const string& targ_name) {
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

void SchemaPage::toBytes(std::span<byte> buf) {
    assert(buf.size() == cts::PG_SZ);

    u16 offset = 0;

    // deserialize page_type_id
    u8 page_type_id = cts::pg_type_id::SCHEMA_PAGE;
    serialize(page_type_id, buf, offset);

    // deserialize # of tables
    u8 num_tables = m_tables.size();
    serialize(num_tables, buf, offset);

    // deserialize each table
    for (const auto &tab_desc: m_tables) {
        serialize(tab_desc.name, buf, offset);

        serialize(tab_desc.pageID, buf, offset);
    }
}
