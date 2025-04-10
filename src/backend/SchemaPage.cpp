//
// Created by Kylan Chen on 12/20/24.
//

#include "SchemaPage.hpp"
#include "utility.hpp"
#include "assume.hpp"

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
SchemaPage::SchemaPage(std::span<const byte> bytes, pgid_t pageID) : Page(pageID) {
    ASSUME_S(bytes.size() == cts::PG_SZ, "Buffer is not the correct size");
    u16 offset = 0;

    u8 page_type_id, num_tables;
    db_deserialize(page_type_id, bytes, offset);
    ASSUME_S(page_type_id == cts::pg_type_id::SCHEMA_PAGE, "Page ID is invalid");

    db_deserialize(num_tables, bytes, offset);

    for (int i = 0; i < num_tables; ++i) {
        string name;
        pgid_t pageID;

        db_deserialize(name, bytes, offset);
        db_deserialize(pageID, bytes, offset);

        m_tables.emplace(name, pageID);
    }

    ASSUME_S(offset < cts::PG_SZ, "Offset is out of bounds, an error has occurred during serialization");
}

SchemaPage::SchemaPage(pgid_t pageID) : Page(pageID), m_tables(0) {
}

u8 SchemaPage::getNumTables() const {
    return m_tables.size();
}

const std::unordered_map<string, pgid_t>& SchemaPage::getTables() {
    return m_tables;
}

offset_t SchemaPage::freeSpace() const {
    offset_t used = 0;

    used += db_sizeof<u8>(); // page_type_id
    used += db_sizeof<u8>(); // # tables
    used += m_tables.size() * (db_sizeof<string>() + db_sizeof<pgid_t>());

    return cts::PG_SZ - used;
}

void SchemaPage::addTable(const string& name, pgid_t pageID) {
    ASSUME_S(name.length() <= cts::MAX_STR_LEN, "Name is too long");
    ASSUME_S(!name.empty(), "Name cannot be empty");
    ASSUME_S(freeSpace() >= db_sizeof<string>() + db_sizeof<pgid_t>(),
             "There is not enough space in this page to add another table");

    m_tables.emplace(name, pageID);
}

void SchemaPage::removeTable(const string &targ_name) {
    ASSUME_S(targ_name.length() <= cts::MAX_STR_LEN, "Name is too long");
    ASSUME_S(!targ_name.empty(), "Name cannot be empty");
    ASSUME_S(m_tables.contains(targ_name), "Table with that name does not exist");

    m_tables.erase(targ_name);
}

void SchemaPage::toBytes(std::span<byte> buf) {
    ASSUME_S(buf.size() == cts::PG_SZ, "Buffer is incorrectly sized");
    u16 offset = 0;

    u8 page_type_id = cts::pg_type_id::SCHEMA_PAGE;
    db_serialize(page_type_id, buf, offset);

    u8 num_tables = m_tables.size();
    db_serialize(num_tables, buf, offset);

    for (const auto &[name, pageID]: m_tables) {
        db_serialize(name, buf, offset);
        db_serialize(pageID, buf, offset);
    }
    ASSUME_S(offset <= cts::PG_SZ, "Offset out of bounds");
}
} // namespace backend
