//
// Created by Kylan Chen on 3/7/25.
//

#ifndef KNDB_PAGER_TPP
#define KNDB_PAGER_TPP

#include <cassert>

#include "Pager.hpp"
#include "FSMPage.hpp"
#include "constants.hpp"

namespace backend {

template <typename T>
T &Pager::getPage(u32 pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    if (pageID >= m_ioHandler.getNumBlocks())
        throw std::invalid_argument("pageID out of bounds");

    if (!std::is_same_v<T, FSMPage> && isFree(pageID))
        throw std::invalid_argument("that page is not being used right now");

    // check if page is in cache
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

    // create new block in db file
    u32 new_page_no = allocPageBit();

    if (m_ioHandler.getNumBlocks() > cts::MAX_BLOCKS)
        throw std::runtime_error("Database has reached block limit.");

    // create a new page in cache (which is just a list for now)
    m_cache.emplace(new_page_no, std::make_unique<T>(std::forward<Args>(args)..., new_page_no));

    // dynamic cast to catch potential type safety issues
    return dynamic_cast<T &>(*m_cache[new_page_no]);
}

template <typename T>
void Pager::freePage(u32 pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    if (pageID >= m_ioHandler.getNumBlocks())
        throw std::invalid_argument("Invalid pageID");

    // TODO: this is inefficient since we don't really need to wipe the page,
    //  but doing this for safety.
    PgArr<byte> buf{};
    m_ioHandler.writeBlock(buf.data(), pageID);

    // mark page as free in bitmap
    freePageBit(pageID);

    // remove from cache
    m_cache.erase(pageID);
}

} // namespace backend

#endif //KNDB_PAGER_TPP
