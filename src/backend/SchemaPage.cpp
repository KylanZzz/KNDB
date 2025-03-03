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

    // check if page type is correct
    size_t page_type_id;
    memcpy(&page_type_id, bytes.data() + offset, db_sizeof<size_t>());
    if (page_type_id != get_page_type_id<SchemaPage>())
        throw std::runtime_error("page_type_id does not match any valid page type");
    offset += db_sizeof<size_t>();

    // deserialize # of tables
    size_t num_tables;
    memcpy(&num_tables, bytes.data() + offset, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // deserialize each table individually
    for (int i = 0; i < num_tables; ++i) {
        table_descriptor tab_desc;

        // we use c_str as a buffer, then convert to a std::string internally
        // because std::string has weird behavior when copying .data() directly
        char c_str[db_sizeof<string>()];
        memcpy(c_str, bytes.data() + offset, db_sizeof<string>() - 1);
        tab_desc.name = string(c_str);
        offset += db_sizeof<string>();

        size_t num_types;
        memcpy(&num_types, bytes.data() + offset, db_sizeof<size_t>());
        offset += db_sizeof<size_t>();

        // deserialize list of types in table
        size_t type_id;
        for (int j = 0; j < num_types; ++j) {
            memcpy(&type_id, bytes.data() + offset, db_sizeof<size_t>());
            offset += db_sizeof<size_t>();
            tab_desc.types.push_back(type_id_to_variant(type_id));
        }

        memcpy(&tab_desc.pageID, bytes.data() + offset, db_sizeof<size_t>());
        offset += db_sizeof<size_t>();

        m_tables.push_back(tab_desc);
    }
    
    if (offset > cts::PG_SZ) {
        throw std::runtime_error("Schema Page has exceeded size of one page");
    }
}

vector<variants> SchemaPage::getTableTypes(string table_name) {
    if (table_name.length() + 1 > db_sizeof<string>())
        throw std::invalid_argument("Name is too long");

    if (table_name.empty())
        throw std::invalid_argument("Name cannot be empty");

    for (int i = 0; i < m_tables.size(); ++i) {
        if (m_tables[i].name == table_name) {
            return m_tables[i].types;
        }
    }

    throw std::invalid_argument("Table name does not exist in schema");
}

size_t SchemaPage::getNumTables() {
    return m_tables.size();
}

std::unordered_map<string, size_t> SchemaPage::getTables() {
    std::unordered_map<string, size_t> res;
    for (const auto& table: m_tables)
        res[table.name] = table.pageID;
    return res;
}

size_t SchemaPage::freeSpace() {
    size_t used = 0;
    used += db_sizeof<size_t>(); // page_type_id
    used += db_sizeof<size_t>(); // # tables

    // table info
    for (const auto& table: m_tables) {
        used += tableSpaceUsed(table);
    }

    return cts::PG_SZ - used;
}

inline size_t SchemaPage::tableSpaceUsed(table_descriptor tab_desc) {
    return db_sizeof<string>() + db_sizeof<size_t>() + (tab_desc.types.size() * db_sizeof<size_t>());
}

void SchemaPage::toBytes(ByteVec& vec) {
    assert(vec.size() == cts::PG_SZ);

    size_t offset = 0;

    // serialize page_type_id
    size_t page_type_id = get_page_type_id<SchemaPage>();
    memcpy(vec.data() + offset, &page_type_id, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // serialize # of tables
    size_t num_tables = m_tables.size();
    memcpy(vec.data() + offset, &num_tables, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // serialize each table
    for (int i = 0; i < num_tables; ++i) {
        const auto& tab_desc = m_tables[i];
        memcpy(vec.data() + offset, tab_desc.name.data(), tab_desc.name.length() + 1);
        offset += db_sizeof<string>();

        size_t num_types = tab_desc.types.size();
        memcpy(vec.data() + offset, &num_types, db_sizeof<size_t>());
        offset += db_sizeof<size_t>();

        // serialize list of types in each table
        for (int j = 0; j < num_types; ++j) {
            size_t type_id = variant_to_type_id(tab_desc.types[j]);
            memcpy(vec.data() + offset, &type_id, db_sizeof<size_t>());
            offset += db_sizeof<size_t>();
        }

        memcpy(vec.data() + offset, &tab_desc.pageID, db_sizeof<size_t>());
        offset += db_sizeof<size_t>();
    }
}

void SchemaPage::addTable(string name, vector<variants> types, size_t pageID) {
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
    table_descriptor new_table{std::move(name), std::move(types), pageID};

    if (freeSpace() < tableSpaceUsed(new_table))
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
