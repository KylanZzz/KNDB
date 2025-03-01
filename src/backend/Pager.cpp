//
// Created by Kylan Chen on 10/13/24.
//

#include <cassert>
#include "Pager.hpp"

template SchemaPage& Pager::createNewPage<SchemaPage>();
template FSMPage& Pager::createNewPage<FSMPage>();

Pager::Pager(IOHandler &ioHandler) : m_ioHandler(ioHandler) {
    // db has just been created, we need to init first FSM page
    if (m_ioHandler.getNumBlocks() == 0)
        initFSMPage(cts::FSM_PAGE_NO);

    assert(ioHandler.getNumBlocks() == 1);
}

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
    size_t new_page_no = allocPageBit();
    m_ioHandler.readBlock((void *) vec.data(), new_page_no);

    // fill first bytes with correct page_id_type
    size_t page_type_id = get_page_type_id<T>();
    memcpy(vec.data(), &page_type_id, db_sizeof<size_t>());

    // create a new page in cache (which is just a list for now)
    m_cache.emplace_back(std::make_unique<T>(vec, new_page_no));

    // dynamic cast to catch potential type safety issues
    return dynamic_cast<T &>(*m_cache.back());
}

template<typename T>
void Pager::freePage(int pageID) {
    static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

    // potentially set the page to all 0s for safety
    ByteVec temp(cts::PG_SZ, byte(0));
    m_ioHandler.writeBlock((void *) temp.data(), pageID);

    // use freeSpaceMap to free page
    freePageBit(pageID);
}

void Pager::freePageBit(size_t pageID) {
    //Q: if calculated pageID >= total # blocks (IOHandler)
    //    - throw error, out of bounds pageID
    if (pageID >= m_ioHandler.getNumBlocks())
        throw std::invalid_argument("pageID is out of bounds");

    //1. calculate which FSMPageId the target pageID is in
    FSMPage currPage = getPage<FSMPage>(cts::FSM_PAGE_NO);
    size_t blocksPerPage = currPage.getBlocksInPage();

    //2. get page(maybe using linked-list style search), set the bit to 0
    // the FSMPage (node) that contains the page we want to free
    // 0-indexed
    int nodeNo = pageID / blocksPerPage;
    for (int i = 0; i < nodeNo; i++) {
        currPage = getPage<FSMPage>(currPage.getNextPageID());
    }

    int localIdx = pageID % blocksPerPage;
    if (currPage.isFree(localIdx))
        throw std::invalid_argument("page is already free, cannot free");
    currPage.freeBit(localIdx);
}


size_t Pager::allocPageBit() {
    FSMPage currPage = getPage<FSMPage>(cts::FSM_PAGE_NO);

    //1. if current there is free bit in bitmap:
    //    - return next free bit in bitmap and set bit to 1
    //2. else
    //    - start from first page of FSM bitmap, keep going
    //     until you find a page with a free bitmap
    int idx = 0;
    while (currPage.getSpaceLeft() == 0 && currPage.hasNextPage()) {
        idx += currPage.getBlocksInPage();
        currPage = getPage<FSMPage>(currPage.getNextPageID());
    }

    // free bitmap has been found
    if (currPage.getSpaceLeft() > 0) {
        size_t localIdx= currPage.findNextFree();
        currPage.allocBit(localIdx);
        assert(m_ioHandler.createNewBlock() == (idx + localIdx));
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
    currPage = getPage<FSMPage>(newPageID);

    int localIdx = currPage.findNextFree();
    currPage.allocBit(localIdx);
    assert(m_ioHandler.createNewBlock() == (idx + localIdx));
    return idx + localIdx;
}

void Pager::initFSMPage(size_t pageID) {
    assert(m_ioHandler.createNewBlock() == pageID);
    FSMPage firstPage(pageID);
    ByteVec bytes(cts::PG_SZ);
    firstPage.toBytes(bytes);
    m_ioHandler.writeBlock(bytes.data(), pageID);
}

Pager::~Pager() {
    // write everything in cache to disk
    ByteVec temp(cts::PG_SZ);
    for (const auto &page_ptr: m_cache) {
        page_ptr->toBytes(temp);
        m_ioHandler.writeBlock((void *) temp.data(), page_ptr->getPageID());
    }
}
