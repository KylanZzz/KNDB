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
    ASSUME_S(!m_freeSpaceMap.isFree(pageID), "That page is freed");

    return m_pageCache.retrievePage<T>(pageID);
}

template<typename T, typename ...Args>
T &Pager::createNewPage(Args&&... args) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    // new FSMPage needed
    if (m_ioHandler.getNumBlocks() == 0 || m_freeSpaceMap.isFull()) {
        auto numFSMPages = m_ioHandler.getNumBlocks() / FSMPage::getBlocksInPage();
        if (numFSMPages == cts::MAX_FSMPAGES)
            throw std::runtime_error("DB has reached maximum size limit");

        pgid_t newFsmPageNo = m_ioHandler.createMultipleBlocks(FSMPage::getBlocksInPage());
        auto newFsmPage = std::make_unique<FSMPage>(newFsmPageNo);
        m_pageCache.insertPage(std::move(newFsmPage));

        if (newFsmPageNo != 0 ) m_freeSpaceMap.linkFSMPage(newFsmPageNo);
    }

    pgid_t newPageNo = m_freeSpaceMap.allocBit();
    auto newPage = std::make_unique<T>(std::forward<Args>(args)..., newPageNo);
    m_pageCache.insertPage(std::move(newPage));

    return getPage<T>(newPageNo);
}

template <typename T>
void Pager::freePage(pgid_t pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");
    ASSUME_S(pageID < m_ioHandler.getNumBlocks(), "PageID is out of bounds");
    ASSUME_S(!m_freeSpaceMap.isFree(pageID), "That page is freed already");

    m_freeSpaceMap.freeBit(pageID);
}

} // namespace backend

#endif //KNDB_PAGER_TPP