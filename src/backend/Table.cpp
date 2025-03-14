//
// Created by Kylan Chen on 10/13/24.
//

#include <utility>

#include "Table.hpp"
#include "BtreeNodePage.hpp"
#include "Pager.hpp"
#include "TablePage.hpp"

#define T_PAGE m_pager.getPage<TablePage>(m_tablePageID)

Table::Table(string name, Pager &pgr, size_t tablePageId) : m_pager(pgr), m_tablePageID
        (tablePageId), m_name(name) {
    size_t metadataBuffer = 100;
    size_t cell_size = db_sizeof(T_PAGE.getTypes()) + db_sizeof(T_PAGE.getTypes()[0]);
    size_t free_space = cts::PG_SZ - metadataBuffer;
    size_t page_ptr_size = db_sizeof<size_t>();

    size_t deg = (free_space + cell_size) / (2 * (cell_size + page_ptr_size));

    // Page has not been initialized
    if (T_PAGE.getBtreePageID() == cts::SIZE_T_OUT_OF_BOUNDS) {
        // new btree node has no parent ptr
        m_pager.createNewPage<BtreeNodePage<vector<variants>>>(
                deg, cts::SIZE_T_OUT_OF_BOUNDS, true, true
        );
    }

    m_btree = std::make_unique<Btree<vector<variants>>>(T_PAGE.getBtreePageID(), pgr, deg);
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
    if (values.size() != T_PAGE.getTypes().size())
        throw std::runtime_error("Tuple has incorrect number of values.");

    for (int i = 0; i < values.size(); i++)
        if (variant_to_type_id(values[i]) != variant_to_type_id(T_PAGE.getTypes()[i]))
            throw std::runtime_error("Tuple has one or more incorrect types.");

    size_t prev_id = m_btree->getRootPage();
    m_btree->insert(values, values[0]);
    T_PAGE.addTuple();

    if (m_btree->getRootPage() != prev_id)
        T_PAGE.setBtreePageID(m_btree->getRootPage());
}

vector<variants> Table::readTuple(variants key) {
    if (variant_to_type_id(T_PAGE.getTypes()[0]) != variant_to_type_id(key))
        throw std::runtime_error("Key is incorrect type.");

    return m_btree->search(std::move(key));
}

void Table::dropTable() {
    // ???
}

void Table::updateTuple(vector<variants> values) {
    if (values.size() != T_PAGE.getTypes().size())
        throw std::runtime_error("Tuple has incorrect number of values.");

    for (int i = 0; i < values.size(); i++)
        if (variant_to_type_id(values[i]) != variant_to_type_id(T_PAGE.getTypes()[i]))
            throw std::runtime_error("Tuple has one or more incorrect types.");

    m_btree->update(values, values[0]);
}

void Table::deleteTuple(variants key) {
    if (variant_to_type_id(T_PAGE.getTypes()[0]) != variant_to_type_id(key))
        throw std::runtime_error("Key is incorrect type.");

    size_t prev_id = m_btree->getRootPage();
    m_btree->remove(key);
    T_PAGE.removeTuple();

    if (m_btree->getRootPage() != prev_id)
        T_PAGE.setBtreePageID(m_btree->getRootPage());
}