//
// Created by Kylan Chen on 10/13/24.
//

#include <utility>

#include "Table.hpp"

#include <Btree.hpp>

#include "BtreeNodePage.hpp"
#include "Pager.hpp"
#include "TablePage.hpp"

#define T_PAGE m_pager.getPage<TablePage>(m_tablePageID)

namespace backend {

Table::Table(string name, Pager &pgr, const u32 tablePageId) : m_pager(pgr), m_tablePageID
        (tablePageId), m_name(std::move(name)) {
    u16 deg = calculateDegree(T_PAGE.getTypes()[0], T_PAGE.getTypes());
    m_btree = std::make_unique<Btree<Vec<Vari>>>(T_PAGE.getBtreePageID(), pgr, deg);
}

Table::Table(string name, Pager &pgr, const Vec<Vari> &types) : m_pager(pgr),
                                                                m_name(std::move(name)) {
    m_tablePageID = m_pager.createNewPage<TablePage>(types, cts::U32_INVALID).getPageID();
    u16 deg = calculateDegree(T_PAGE.getTypes()[0], T_PAGE.getTypes());
    u32 btree_pg = m_pager.createNewPage<BtreeNodePage<Vec<Vari>>>(
            deg, cts::U32_INVALID, true, true
    ).getPageID();
    T_PAGE.setBtreePageID(btree_pg);
    m_btree = std::make_unique<Btree<Vec<Vari>>>(T_PAGE.getBtreePageID(), m_pager, deg);
}

u64 Table::getNumTuples() const {
    return T_PAGE.getNumTuples();
}

Vec<Vari> Table::getTypes() const {
    return T_PAGE.getTypes();
}

u32 Table::getTablePageID() const {
    return m_tablePageID;
}

string Table::getName() {
    return m_name;
}

void Table::insertTuple(Vec<Vari> values) const {
    if (values.size() != T_PAGE.getTypes().size())
        throw std::runtime_error("Tuple has incorrect number of values.");

    for (int i = 0; i < values.size(); i++)
        if (variant_to_type_id(values[i]) != variant_to_type_id(T_PAGE.getTypes()[i]))
            throw std::runtime_error("Tuple has one or more incorrect types.");

    u32 og_root = m_btree->getRootPage();
    m_btree->insert(values, values[0]);
    T_PAGE.addTuple();

    if (m_btree->getRootPage() != og_root)
        T_PAGE.setBtreePageID(m_btree->getRootPage());
}

Vec<Vari> Table::readTuple(const Vari &key) const {
    if (variant_to_type_id(T_PAGE.getTypes()[0]) != variant_to_type_id(key))
        throw std::runtime_error("Key is incorrect type.");

    return m_btree->search(key);
}

void Table::drop() {
    // ???
}

void Table::updateTuple(const Vec<Vari> &values) const {
    if (values.size() != T_PAGE.getTypes().size())
        throw std::runtime_error("Tuple has incorrect number of values.");

    for (int i = 0; i < values.size(); i++)
        if (variant_to_type_id(values[i]) != variant_to_type_id(T_PAGE.getTypes()[i]))
            throw std::runtime_error("Tuple has one or more incorrect types.");

    m_btree->update(values, values[0]);
}

void Table::deleteTuple(const Vari &key) const {
    if (variant_to_type_id(T_PAGE.getTypes()[0]) != variant_to_type_id(key))
        throw std::runtime_error("Key is incorrect type.");

    u32 og_root = m_btree->getRootPage();
    m_btree->remove(key);
    T_PAGE.removeTuple();

    if (m_btree->getRootPage() != og_root)
        T_PAGE.setBtreePageID(m_btree->getRootPage());
}

} // namespace backend