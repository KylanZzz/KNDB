//
// Created by Kylan Chen on 12/20/24.
//

#include <cassert>

#include "SchemaPage.hpp"
#include "utility.hpp"

namespace backend {

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
    assert(bytes.size() == cts::PG_SZ);
    u16 offset = 0;

    u8 page_type_id, num_tables;
    db_deserialize(page_type_id, bytes, offset);
    assert(page_type_id == cts::pg_type_id::SCHEMA_PAGE);

    db_deserialize(num_tables, bytes, offset);

    for (int i = 0; i < num_tables; ++i) {
        table_descriptor tab_desc;

        db_deserialize(tab_desc.name, bytes, offset);
        db_deserialize(tab_desc.pageID, bytes, offset);

        m_tables.push_back(tab_desc);
    }

    assert (offset < cts::PG_SZ);
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
    used += m_tables.size() * (db_sizeof<string>() + db_sizeof<u32>());

    return cts::PG_SZ - used;
}

void SchemaPage::addTable(string name, u32 pageID) {
    assert(name.length() < db_sizeof<string>());
    assert(!name.empty());
    assert(freeSpace() >= db_sizeof<string>() + db_sizeof<u32>());

    m_tables.push_back({std::move(name), pageID});
}

void SchemaPage::removeTable(const string &targ_name) {
    assert(targ_name.length() < db_sizeof<string>());
    assert(!targ_name.empty());

    for (int i = 0; i < m_tables.size(); ++i) {
        if (m_tables[i].name == targ_name) {
            m_tables.erase(m_tables.begin() + i);
            return;
        }
    }

    abort();
}

void SchemaPage::toBytes(std::span<byte> buf) {
    assert(buf.size() == cts::PG_SZ);
    u16 offset = 0;

    u8 page_type_id = cts::pg_type_id::SCHEMA_PAGE;
    db_serialize(page_type_id, buf, offset);

    u8 num_tables = m_tables.size();
    db_serialize(num_tables, buf, offset);

    for (const auto &tab_desc: m_tables) {
        db_serialize(tab_desc.name, buf, offset);
        db_serialize(tab_desc.pageID, buf, offset);
    }
}

} // namespace backend