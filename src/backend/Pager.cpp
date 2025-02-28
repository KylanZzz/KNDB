//
// Created by Kylan Chen on 10/13/24.
//

#include "Pager.hpp"

template<typename T>
T &Pager::getPage(int pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    // check if page is in cache
    for (auto &pagePtr: m_cache) {
        if (pagePtr->getPageID() == pageID) {
            return dynamic_cast<T &>(*pagePtr);
        }
    }

    // if not in cache, load it into cache
    ByteVec vec(cts::PG_SZ);
    m_ioHandler.readBlock((void *) vec.data(), pageID);

    // create a new page in cache (which is just a list for now)
    m_cache.emplace_back(std::make_unique<T>(vec, pageID));

    // dynamic cast to catch potential type safety issues
    return dynamic_cast<T &>(*m_cache.back());
}

template<typename T>
T &Pager::createNewPage() {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    ByteVec vec(cts::PG_SZ);

    // create new block in db file
    size_t new_page_no = m_fsm.allocPage();
    m_ioHandler.readBlock((void *) vec.data(), new_page_no);

    // fill first bytes with correct page_id_type
    size_t page_type_id = get_page_type_id<T>();
    memcpy(vec.data(), &page_type_id, db_sizeof<size_t>());

    // create a new page in cache (which is just a list for now)
    m_cache.emplace_back(std::make_unique<T>(vec, new_page_no));

    // dynamic cast to catch potential type safety issues
    return dynamic_cast<T &>(*m_cache.back());
}

Pager::~Pager() {
    // write everything in cache to disk
    ByteVec temp(cts::PG_SZ);
    for (const auto &page_ptr: m_cache) {
        page_ptr->to_bytes(temp);
        m_ioHandler.writeBlock((void *) temp.data(), page_ptr->getPageID());
    }
}

template<typename T>
void Pager::freePage(int pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    // potentially set the page to all 0s for safety
    ByteVec temp(cts::PG_SZ, byte(0));
    m_ioHandler.writeBlock((void *) temp.data(), pageID);

    // use freeSpaceMap to free page
    m_fsm.freePage(pageID);
}
