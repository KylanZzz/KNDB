//
// Created by Kylan Chen on 2/28/25.
//

#include <cassert>
#include "FSMPage.hpp"
#include "utility.hpp"

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
    deserialize(page_type_id, bytes, offset);

    // check if page type is correct
    if (page_type_id != cts::pg_type_id::FSM_PAGE)
        throw std::runtime_error("page type id is incorrect");

    // deserialize next Page id
    deserialize(m_nextPageID, bytes, offset);

    // deserialize free blocks count
    deserialize(m_freeBlocks, bytes, offset);

    // rest of the page is for bitmap
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
    if (!isFree(idx))
        throw std::invalid_argument("that bit is already in use");

    assert(m_freeBlocks-- > 0);

    m_bitmap[idx / 8] ^= 1 << (idx % 8);
}

bool FSMPage::isFree(u32 idx) {
    if (idx >= (m_bitmap.size() * 8))
        throw std::invalid_argument("idx is out of bounds");

    return !(m_bitmap[idx / 8] & 1 << (idx % 8));
}

u32 FSMPage::findNextFree() {
    for (int i = 0; i < m_bitmap.size() * 8; i++)
        if (isFree(i)) return i;

    throw std::invalid_argument("There are no more free nodes");
}

u32 FSMPage::getSpaceLeft() const {
    return m_freeBlocks;
}

void FSMPage::freeBit(u32 idx) {
    if (isFree(idx))
        throw std::invalid_argument("that bit is already free");

    assert(m_freeBlocks++ <= m_bitmap.size() * 8);

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

    // serialize page_type_id
    u8 page_type_id = cts::pg_type_id::FSM_PAGE;
    serialize(page_type_id, buf, offset);

    // serialize next page ID
    serialize(m_nextPageID, buf, offset);

    // serialize free blocks count
    serialize(m_freeBlocks, buf, offset);

    // serialize bitmap
    memcpy(buf.data() + offset, m_bitmap.data(), m_bitmap.size());
    offset += m_bitmap.size();
}

