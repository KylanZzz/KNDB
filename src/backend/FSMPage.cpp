//
// Created by Kylan Chen on 2/28/25.
//

#include <cassert>
#include "FSMPage.hpp"
#include "utility.hpp"

namespace backend {

//- page type id
//- next block no (-1 if there is no next
//page)
//- free space bitmap
//
//
//
//
//        Format:
//
//
//size_t page_type_id ----------------
//size_t nextBlockNo ------------------
//free space bitmap ------------------
//----------------------------------------
//----------------------------------------
//----------------------------------------
//----------------------------------------
FSMPage::FSMPage(std::span<const byte> bytes, u32 pageID) : Page(pageID) {
    u16 offset = 0;

    u8 page_type_id;
    db_deserialize(page_type_id, bytes, offset);
    assert(page_type_id == cts::pg_type_id::FSM_PAGE);

    db_deserialize(m_nextPageID, bytes, offset);
    db_deserialize(m_freeBlocks, bytes, offset);

    m_bitmap.resize(cts::PG_SZ - offset);
    memcpy(m_bitmap.data(), bytes.data() + offset, m_bitmap.size());
    offset += m_bitmap.size();

    assert(offset <= cts::PG_SZ);
}

FSMPage::FSMPage(u32 pageID) : Page(pageID) {
    m_nextPageID = cts::U32_INVALID;
    m_bitmap.resize(cts::PG_SZ - db_sizeof<u32>() * 3);
    m_freeBlocks = m_bitmap.size() * 8;
    allocBit(0);
}

void FSMPage::allocBit(u32 idx) {
    assert(idx < (m_bitmap.size() * 8));
    assert(isFree(idx));
    assert(m_freeBlocks > 0);

    --m_freeBlocks;
    m_bitmap[idx / 8] ^= 1 << (idx % 8);
}

bool FSMPage::isFree(u32 idx) {
    assert(idx < (m_bitmap.size() * 8));

    return !(m_bitmap[idx / 8] & 1 << (idx % 8));
}

u32 FSMPage::findNextFree() {
    assert(getSpaceLeft() > 0);
    for (int i = 0; i < m_bitmap.size() * 8; i++)
        if (isFree(i)) return i;
    assert(false);
}

u32 FSMPage::getSpaceLeft() const {
    return m_freeBlocks;
}

void FSMPage::freeBit(u32 idx) {
    assert(!isFree(idx));
    assert(m_freeBlocks <= m_bitmap.size() * 8);

    m_freeBlocks++;
    m_bitmap[idx / 8] ^= 1 << (idx % 8);
}

bool FSMPage::hasNextPage() const {
    return m_nextPageID != cts::U32_INVALID;
}

u32 FSMPage::getNextPageID() const {
    if (!hasNextPage()) throw std::invalid_argument("Has no next page");
    return m_nextPageID;
}

void FSMPage::setNextPageID(u32 pageID) {
    m_nextPageID = pageID;
}

void FSMPage::toBytes(std::span<byte> buf) {
    assert(buf.size() == cts::PG_SZ);

    u16 offset = 0;

    u8 page_type_id = cts::pg_type_id::FSM_PAGE;
    db_serialize(page_type_id, buf, offset);

    db_serialize(m_nextPageID, buf, offset);
    db_serialize(m_freeBlocks, buf, offset);

    memcpy(buf.data() + offset, m_bitmap.data(), m_bitmap.size());
    offset += m_bitmap.size();
}

} // namespace backend
