//
// Created by Kylan Chen on 3/7/25.
//

#ifndef KNDB_PAGER_TPP
#define KNDB_PAGER_TPP

#include <cassert>

#include "Pager.hpp"
#include "FSMPage.hpp"
#include "constants.hpp"

template <typename T>
T &Pager::getPage(size_t pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    if (pageID >= m_ioHandler.getNumBlocks())
    throw std::invalid_argument("pageID out of bounds");

    if (!std::is_same_v<T, FSMPage> && isFree(pageID))
    throw std::invalid_argument("that page is not being used right now");

    // check if page is in cache
    for (auto &pagePtr: m_cache)
        if (pagePtr->getPageID() == pageID)
            return dynamic_cast<T &>(*pagePtr);

    // if not in cache, load it into cache
    ByteVec vec(cts::PG_SZ);
    m_ioHandler.readBlock((void *) vec.data(), pageID);

    // create a new page in cache (which is just a list for now)
    m_cache.emplace_back(std::make_unique<T>(vec, pageID));

    // dynamic cast to catch potential type safety issues
    return dynamic_cast<T &>(*m_cache.back());
}

template<typename T, typename ...Args>
T &Pager::createNewPage(Args&&... args) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    // create new block in db file
    size_t new_page_no = allocPageBit();

    if (m_ioHandler.getNumBlocks() > cts::MAX_BLOCKS)
        throw std::runtime_error("Database has reached block limit.");

    // create a new page in cache (which is just a list for now)
    m_cache.emplace_back(std::make_unique<T>(std::forward<Args>(args)..., new_page_no));

    // dynamic cast to catch potential type safety issues
    return dynamic_cast<T &>(*m_cache.back());
}

template <typename T>
void Pager::freePage(size_t pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    if (pageID >= m_ioHandler.getNumBlocks())
        throw std::invalid_argument("Invalid pageID");

    // TODO: this is inefficient since we don't really need to wipe the page,
    //  but doing this for safety.
    ByteVec temp(cts::PG_SZ, byte(0));
    m_ioHandler.writeBlock((void *) temp.data(), pageID);

    // mark page as free in bitmap
    freePageBit(pageID);

    // remove from cache
    int idx = 0;
    for (int i = 0; i < m_cache.size(); i++) {
        if (m_cache[i]->getPageID() == pageID) {
            idx = i;
            break;
        }
    }
    m_cache.erase(m_cache.begin() + idx);
}

#endif //KNDB_PAGER_TPP
