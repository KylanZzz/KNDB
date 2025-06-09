//
// Created by Kylan Chen on 2/28/25.
//

#include "FSMPage.hpp"
#include "utility.hpp"
#include "assume.hpp"

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
FSMPage::FSMPage(std::span<const byte> bytes, pgid_t pageID) : Page(pageID) {
    ASSUME_S(bytes.size() == cts::PG_SZ, "Buffer is incorrectly sized");
    offset_t offset = 0;

    pgtypeid_t page_type_id;
    db_deserialize(page_type_id, bytes, offset);
    ASSUME_S(page_type_id == cts::pg_type_id::FSM_PAGE, "Page_type_id is incorrect type");

    db_deserialize(m_nextPageID, bytes, offset);
    db_deserialize(m_freeBlocks, bytes, offset);

    m_bitmap.resize(cts::PG_SZ - offset);
    memcpy(m_bitmap.data(), bytes.data() + offset, m_bitmap.size());
    offset += m_bitmap.size();

    ASSUME_S(offset <= cts::PG_SZ, "Offset out of bounds");
}

FSMPage::FSMPage(pgid_t pageID) : Page(pageID) {
    m_nextPageID = cts::PGID_INVALID;
    m_bitmap.resize(cts::PG_SZ - db_sizeof<u32>() * 3);
    m_freeBlocks = m_bitmap.size() * 8;
    allocBit(0);
}

void FSMPage::allocBit(bitmapidx_t idx) {
    ASSUME_S(idx < (m_bitmap.size() * 8), "Index is out of bounds");
    ASSUME_S(getSpaceLeft() > 0, "This bitmap page is already completely filled");
    ASSUME_S(isFree(idx), "That block is already being used");

    --m_freeBlocks;
    m_bitmap[idx / 8] ^= 1 << (idx % 8);
}

bool FSMPage::isFree(bitmapidx_t idx) const {
    ASSUME_S(idx < (m_bitmap.size() * 8), "Index is out of bounds");

    return !(m_bitmap[idx / 8] & 1 << (idx % 8));
}

bitmapidx_t FSMPage::findNextFree() const {
    ASSUME_S(getSpaceLeft() > 0, "This bitmap page is already completely filled");
    for (int i = 0; i < m_bitmap.size() * 8; i++)
        if (isFree(i)) return i;

    ASSUME_S(false, "No free page has been found");
}

bitmapidx_t FSMPage::getSpaceLeft() const {
    return m_freeBlocks;
}

void FSMPage::freeBit(const bitmapidx_t idx) {
    ASSUME_S(!isFree(idx), "That bit is already free");
    ASSUME_S(m_freeBlocks < m_bitmap.size() * 8, "Bitmap has more free blocks than feasibly possible");

    m_freeBlocks++;
    m_bitmap[idx / 8] ^= 1 << (idx % 8);
}

bool FSMPage::hasNextPage() const {
    return m_nextPageID != cts::PGID_INVALID;
}

pgid_t FSMPage::getNextPageID() const {
    if (!hasNextPage()) throw std::invalid_argument("Has no next page");
    return m_nextPageID;
}

void FSMPage::setNextPageID(pgid_t pageID) {
    m_nextPageID = pageID;
}

void FSMPage::toBytes(std::span<byte> buf) {
    ASSUME_S(buf.size() == cts::PG_SZ, "Buffer is incorrectly sized");

    offset_t offset = 0;

    pgtypeid_t page_type_id = cts::pg_type_id::FSM_PAGE;
    db_serialize(page_type_id, buf, offset);

    db_serialize(m_nextPageID, buf, offset);
    db_serialize(m_freeBlocks, buf, offset);

    memcpy(buf.data() + offset, m_bitmap.data(), m_bitmap.size());
    offset += m_bitmap.size();

    ASSUME_S(offset <= cts::PG_SZ, "Offset out of bounds");
}

} // namespace backend
