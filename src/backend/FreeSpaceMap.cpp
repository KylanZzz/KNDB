//
// Created by Kylan Chen on 2/27/25.
//

#include "FreeSpaceMap.hpp"
#include "FSMPage.hpp"
#include "utility.hpp"


FreeSpaceMap::FreeSpaceMap(IOHandler &ioHandler, size_t startPageId) :
    m_ioHandler (ioHandler), m_startPageId(startPageId), m_pager(nullptr) {

    // db has just been created, we need to init first FSM page
    if (m_ioHandler.getNumBlocks() == 0)
        initFSMPage(m_startPageId);

    assert(ioHandler.getNumBlocks() == 1);
}

void FreeSpaceMap::freePage(size_t pageID) {
    //Q: if calculated pageID >= total # blocks (IOHandler)
    //    - throw error, out of bounds pageID
    if (pageID >= m_ioHandler.getNumBlocks())
        throw std::invalid_argument("pageID is out of bounds");

    //1. calculate which FSMPageId the target pageID is in
    FSMPage currPage = m_pager->getPage<FSMPage>(m_startPageId);
    size_t blocksPerPage = currPage.getBlocksInPage();

    //2. get page(maybe using linked-list style search), set the bit to 0
    // the FSMPage (node) that contains the page we want to free
    // 0-indexed
    int nodeNo = pageID / blocksPerPage;
    for (int i = 0; i < nodeNo; i++) {
        currPage = m_pager->getPage<FSMPage>(currPage.getNextPageID());
    }

    int localIdx = pageID % blocksPerPage;
    if (currPage.isFree(localIdx))
        throw std::invalid_argument("page is already free, cannot free");
    currPage.freeBit(localIdx);
}

size_t FreeSpaceMap::allocPage() {
    FSMPage currPage = m_pager->getPage<FSMPage>(m_startPageId);

    //1. if current there is free bit in bitmap:
    //    - return next free bit in bitmap and set bit to 1
    //2. else
    //    - start from first page of FSM bitmap, keep going
    //     until you find a page with a free bitmap
    int idx = 0;
    while (currPage.getSpaceLeft() == 0 && currPage.hasNextPage()) {
        idx += currPage.getBlocksInPage();
        currPage = m_pager->getPage<FSMPage>(currPage.getNextPageID());
    }

    // free bitmap has been found
    if (currPage.getSpaceLeft() > 0) {
        size_t localIdx= currPage.findNextFree();
        currPage.allocBit(localIdx);
        return idx + localIdx;
    }

    //3. else you go through all of FSM pages and find no free bitmap
    //    - request new page from IOHandler
    //    - set previous last page->next to new pageNo
    //    - instantiate new page with free bitmap (set first bit to 1, since
    //      first page is for bitmap)
    size_t newPageID = m_ioHandler.createNewBlock();
    currPage.setNextPageID(newPageID);
    initFSMPage(newPageID);
    currPage = m_pager->getPage<FSMPage>(newPageID);

    int localIdx = currPage.findNextFree();
    currPage.allocBit(localIdx);
    return idx + localIdx;
}

void FreeSpaceMap::initFSMPage(size_t pageID) {
    FSMPage firstPage(pageID);
    ByteVec bytes(cts::PG_SZ);
    firstPage.toBytes(bytes);
    m_ioHandler.writeBlock(bytes.data(), pageID);
}