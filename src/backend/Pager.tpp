//
// Created by Kylan Chen on 3/7/25.
//

#ifndef KNDB_PAGER_TPP
#define KNDB_PAGER_TPP

#include "Pager.hpp"
#include "FSMPage.hpp"
#include "constants.hpp"
#include "assume.hpp"

namespace backend {

template <typename T>
T &Pager::getPage(pgid_t pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");
    ASSUME_S(pageID < m_ioHandler.getNumBlocks(), "PageID is out of bounds");
    ASSUME_S((std::is_same_v<T, FSMPage>) || !isFree(pageID), "That page is freed");

    if (m_cache.contains(pageID))
        return dynamic_cast<T &>(*m_cache[pageID]);

    // if not in cache, load it into cache
    PgArr<byte> buf;
    m_ioHandler.readBlock(buf.data(), pageID);

    // create a new page in cache (which is just a list for now)
    m_cache.emplace(pageID, std::make_unique<T>(buf, pageID));

    // dynamic cast to catch potential type safety issues
    return dynamic_cast<T &>(*m_cache[pageID]);
}

template<typename T, typename ...Args>
T &Pager::createNewPage(Args&&... args) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    if (m_ioHandler.getNumBlocks() >= cts::MAX_BLOCKS)
        throw std::runtime_error("Database has reached block limit.");

    // create new block in db file
    pgid_t new_page_no = allocPageBit();

    // create a new page in cache (which is just a list for now)
    m_cache.emplace(new_page_no, std::make_unique<T>(std::forward<Args>(args)..., new_page_no));

    // dynamic cast to catch potential type safety issues
    return dynamic_cast<T &>(*m_cache[new_page_no]);
}

template <typename T>
void Pager::freePage(pgid_t pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");
    ASSUME_S(pageID < m_ioHandler.getNumBlocks(), "PageID is out of bounds");

    PgArr<byte> buf{};
    m_ioHandler.writeBlock(buf.data(), pageID);

    // mark page as free in bitmap
    freePageBit(pageID);

    // remove from cache
    m_cache.erase(pageID);
}

} // namespace backend

#endif //KNDB_PAGER_TPP
