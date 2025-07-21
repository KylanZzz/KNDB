//
// Created by kylan on 7/20/2025.
//

#include "FreeSpaceMap.hpp"
#include <FSMPage.hpp>
#include "assume.hpp"

namespace backend {

FreeSpaceMap::FreeSpaceMap(PageCache& cache) : m_cache(cache) {

}

pgid_t FreeSpaceMap::allocBit() {
    ASSUME_S(!isFull(), "There is no bitmap with free space left");

    auto& firstFSMPage = m_cache.retrievePage<FSMPage>(0);
    if (firstFSMPage.getSpaceLeft() != 0) {
        auto bit = firstFSMPage.findNextFree();
        firstFSMPage.allocBit(bit);
        return bit;
    }

    auto nextFSMPageID = firstFSMPage.getNextPageID();
    auto& nextFSMPage = m_cache.retrievePage<FSMPage>(nextFSMPageID);
    auto bit = nextFSMPage.findNextFree();
    nextFSMPage.allocBit(bit);

    // point to next free page (should be invalid if there is no space left in FSM)
    if (nextFSMPage.getSpaceLeft() == 0) {
        firstFSMPage.setNextPageID(nextFSMPage.getNextPageID());
    }

    return nextFSMPageID + bit;
}

void FreeSpaceMap::freeBit(pgid_t pageID) {
    ASSUME_S(!isFree(pageID), "That page is already freed");

    pgid_t fsm_pgid = pageID / FSMPage::getBlocksInPage();
    bitmapidx_t bit = pageID % FSMPage::getBlocksInPage();

    auto& currFSMPage = m_cache.retrievePage<FSMPage>(fsm_pgid * FSMPage::getBlocksInPage());
    auto& firstFSMPage = m_cache.retrievePage<FSMPage>(0);
    currFSMPage.freeBit(bit);

    if (currFSMPage.getPageID() == 0) {
        return;
    }

    // current page WAS full
    if (currFSMPage.getSpaceLeft() == 1) {
        pgid_t prevNextPageNo = firstFSMPage.getNextPageID();
        firstFSMPage.setNextPageID(currFSMPage.getPageID());
        currFSMPage.setNextPageID(prevNextPageNo);
    }
}

bool FreeSpaceMap::isFree(pgid_t pageID) {
    pgid_t fsm_pgid = pageID / FSMPage::getBlocksInPage();
    bitmapidx_t bit = pageID % FSMPage::getBlocksInPage();
    auto& fsm_page = m_cache.retrievePage<FSMPage>(fsm_pgid * FSMPage::getBlocksInPage());
    return fsm_page.isFree(bit);
}

bool FreeSpaceMap::isFull() {
    auto& firstFSMPage = m_cache.retrievePage<FSMPage>(0);
    return firstFSMPage.getSpaceLeft() == 0 && firstFSMPage.getNextPageID() == cts::PGID_INVALID;
}

void FreeSpaceMap::linkFSMPage(pgid_t newFSMPageID) {
    auto& firstFsmPage = m_cache.retrievePage<FSMPage>(0);
    auto& newFsmPage = m_cache.retrievePage<FSMPage>(newFSMPageID);

    auto prevNextPageNo = firstFsmPage.getNextPageID();
    firstFsmPage.setNextPageID(newFSMPageID);
    newFsmPage.setNextPageID(prevNextPageNo);
}

}
