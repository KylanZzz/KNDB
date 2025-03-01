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

FSMPage::FSMPage(size_t pageID, ByteVec &bytes) : Page(pageID) {
    size_t offset = 0;

    // check if page type is correct
    size_t page_type_id;
    memcpy(&page_type_id, bytes.data() + offset, db_sizeof<size_t>());
    if (page_type_id != get_page_type_id<FSMPage>())
        throw std::runtime_error("page_type_id does not match any valid page type");
    offset += db_sizeof<size_t>();

    // deserialize next Page id
    memcpy(&m_nextPageID, bytes.data() + offset, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // deserialize free blocks count
    memcpy(&m_freeBlocks, bytes.data() + offset, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // rest of the page is for bitmap
    m_bitmap.resize(cts::PG_SZ - offset);
    memcpy(m_bitmap.data(), bytes.data() + offset, m_bitmap.size());
    offset += m_bitmap.size();
}

FSMPage::FSMPage(size_t pageID) : Page(pageID) {
    m_nextPageID = NO_NEXT_PAGE;
    m_bitmap.resize(cts::PG_SZ - db_sizeof<size_t>() * 3);
    m_freeBlocks = m_bitmap.size() * 8;
}

void FSMPage::allocBit(size_t idx) {
    if (!isFree(idx))
        throw std::invalid_argument("that bit is already in use");

    assert(m_freeBlocks-- > 0);

    m_bitmap[idx / 8] ^= 1 << (idx % 8);
}

bool FSMPage::isFree(size_t idx) {
    if (idx >= (m_bitmap.size() * 8))
        throw std::invalid_argument("idx is out of bounds");

    return !(m_bitmap[idx / 8] & 1 << (idx % 8));
}

size_t FSMPage::findNextFree() {
    for (int i = 0; i < m_bitmap.size() * 8; i++)
        if (isFree(i)) return i;

    throw std::invalid_argument("There are no more free nodes");
}

size_t FSMPage::getSpaceLeft() {
    return m_freeBlocks;
}

void FSMPage::freeBit(size_t idx) {
    if (isFree(idx))
        throw std::invalid_argument("that bit is already free");

    assert(m_freeBlocks++ <= m_bitmap.size() * 8);

    m_bitmap[idx / 8] ^= 1 << (idx % 8);
}

bool FSMPage::hasNextPage() {
    return m_nextPageID != NO_NEXT_PAGE;
}

size_t FSMPage::getNextPageID() {
    if (!hasNextPage()) throw std::invalid_argument("Has no next page");
    return m_nextPageID;
}

void FSMPage::setNextPageID(size_t pageID) {
    m_nextPageID = pageID;
}

size_t FSMPage::getBlocksInPage() {
    return (cts::PG_SZ - db_sizeof<size_t>() * 3) * 8;
}

void FSMPage::toBytes(ByteVec &vec) {
    assert(vec.size() == cts::PG_SZ);

    size_t offset = 0;

    // serialize page_type_id
    size_t page_type_id = get_page_type_id<FSMPage>();
    memcpy(vec.data() + offset, &page_type_id, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // serialize next page ID
    memcpy(vec.data() + offset, &m_nextPageID, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // serialize free blocks count
    memcpy(vec.data() + offset, &m_freeBlocks, db_sizeof<size_t>());
    offset += db_sizeof<size_t>();

    // serialize bitmap
    memcpy(vec.data() + offset, m_bitmap.data(), m_bitmap.size());
    offset += m_bitmap.size();
}

