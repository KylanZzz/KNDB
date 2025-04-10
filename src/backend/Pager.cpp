//
// Created by Kylan Chen on 3/8/25.
//

#include "Pager.hpp"
#include "assume.hpp"
#include "kndb_types.hpp"

namespace backend{

Pager::Pager(IOHandler &ioHandler) : m_ioHandler(ioHandler) {
    // db has just been created, we need to init first FSM_ID page so pager can actually do what
    // pager needs to do
    if (m_ioHandler.getNumBlocks() == 0) {
        // create FSMPage
        pgid_t pgid = m_ioHandler.createNewBlock();
        ASSUME_S(pgid == cts::pgid::FSM_ID, "First page created is not the correct FSM page ID");
        m_cache.emplace(pgid, std::make_unique<FSMPage>(pgid));
    }
}

pgid_t Pager::allocPageBit() {
    FSMPage* currPage = &getPage<FSMPage>(cts::pgid::FSM_ID);

    //1. if current there is free bit in bitmap:
    //    - return next free bit in bitmap and set bit to 1
    //2. else
    //    - start from first page of FSM_ID bitmap, keep going
    //     until you find a page with a free bitmap
    int idx = 0;
    while (currPage->getSpaceLeft() == 0 && currPage->hasNextPage()) {
        idx++;
        currPage = &getPage<FSMPage>(currPage->getNextPageID());
    }

    // free bitmap has been found
    if (currPage->getSpaceLeft() > 0) {
        pgid_t localIdx= currPage->findNextFree();
        currPage->allocBit(localIdx);
        // need to create new block
        if ((idx * FSMPage::getBlocksInPage()) + localIdx == m_ioHandler.getNumBlocks()) {
            pgid_t id = m_ioHandler.createNewBlock();
            ASSUME_S(id == ((idx * FSMPage::getBlocksInPage()) + localIdx), "New block created is not correct ID");
        }
        return (idx * FSMPage::getBlocksInPage()) + localIdx;
    }

    //3. else you go through all of FSM_ID pages and find no free bitmap
    //    - request new page from IOHandler
    //    - set previous last page->next to new pageNo
    //    - instantiate new page with free bitmap (set first bit to 1, since
    //      first page is for bitmap)
    pgid_t newPageID = m_ioHandler.createNewBlock();
    currPage->setNextPageID(newPageID);
    m_cache.emplace(newPageID, std::make_unique<FSMPage>(newPageID));
    currPage = &getPage<FSMPage>(newPageID);

    pgid_t localIdx = currPage->findNextFree();
    currPage->allocBit(localIdx);
    pgid_t new_id = m_ioHandler.createNewBlock();
    idx++;
    ASSUME_S(new_id == (idx * FSMPage::getBlocksInPage() + localIdx), "New block created is not correct ID");
    return (idx * FSMPage::getBlocksInPage()) + localIdx;
}

void Pager::freePageBit(pgid_t pageID) {
    //Q: if calculated pageID >= total # blocks (IOHandler)
    //    - throw error, out of bounds pageID
    if (pageID >= m_ioHandler.getNumBlocks())
        throw std::invalid_argument("pageID is out of bounds");

    //1. calculate which FSMPageId the target pageID is in
    FSMPage* currPage = &getPage<FSMPage>(cts::pgid::FSM_ID);
    pgid_t blocksPerPage = FSMPage::getBlocksInPage();

    //2. get page(maybe using linked-list style search), set the bit to 0
    // the FSMPage (node) that contains the page we want to free
    // 0-indexed
    pgid_t nodeNo = pageID / blocksPerPage;
    for (int i = 0; i < nodeNo; i++) {
        if (!currPage->hasNextPage())
            throw std::runtime_error("FSMPage has no next page");
        currPage = &getPage<FSMPage>(currPage->getNextPageID());
    }

    pgid_t localIdx = pageID % blocksPerPage;
    if (currPage->isFree(localIdx))
        throw std::invalid_argument("page is already free, cannot free");
    currPage->freeBit(localIdx);
}

bool Pager::isFree(pgid_t pageID) {
    FSMPage* currPage = &getPage<FSMPage>(cts::pgid::FSM_ID);
    pgid_t pageIdx = pageID / FSMPage::getBlocksInPage();
    for (int i = 0; i < pageIdx; i++) {
        if (!currPage->hasNextPage())
            throw std::runtime_error("FSMPage has no next page");
        currPage = &getPage<FSMPage>(currPage->getNextPageID());
    }
    pgid_t localIdx = pageID % FSMPage::getBlocksInPage();
    return currPage->isFree(localIdx);
}

Pager::~Pager() {
    // write everything in cache to disk
    PgArr<byte> temp;
    for (const auto &[pgid, page_ptr]: m_cache) {
        page_ptr->toBytes(temp);
        m_ioHandler.writeBlock((void *) temp.data(), page_ptr->getPageID());
    }
}

} // namespace backend