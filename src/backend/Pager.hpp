//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_PAGER_HPP
#define KNDB_PAGER_HPP

#include <vector>
#include <memory>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <cassert>

#include "IOHandler.hpp"
#include "Page.hpp"
#include "kndb_types.hpp"
#include "constants.hpp"
#include "FSMPage.hpp"
#include "utility.hpp"

using namespace kndb_types;

using PagePtr = std::unique_ptr<Page>;

/**
 * @class Pager
 * @brief Handles database page management.
 *
 * Manages reading, writing, and tracking of database pages. This class acts as
 * the main interface for other classes to use database pages.
 */
class Pager {
public:

    /**
     * @brief Constructs a Pager.
     *
     * @param ioHandler handles disk I/O requests.
     */
    Pager(IOHandler &ioHandler) : m_ioHandler(ioHandler) {
        // db has just been created, we need to init first FSM page so pager can actually do what
        // pager needs to do
        if (m_ioHandler.getNumBlocks() == 0) {
            // create FSMPage
            int pgid = m_ioHandler.createNewBlock();
            assert(pgid == cts::FSM_PAGE_NO);
            m_cache.emplace_back(std::make_unique<FSMPage>(pgid));
//            writePage(cts::FSM_PAGE_NO);

            // create SchemaPage (DEPRECATED. PAGER SHOULD NOT CREATE SCHEMA, CALLER SHOULD)
//            size_t spgid = createNewPage<SchemaPage>().getPageID();
//            assert(spgid == cts::SCHEMA_PAGE_NO);
        }
    }

    /**
     * @brief Retrieves a page.
     *
     * @tparam T type of page that is expected to be returned.  This type
     * must be a subclass of class Page and have a constructor that takes in a
     * ByteVec& and page number.
     *
     * @param pageID the requested page's id.
     *
     * Note: This function call is non-owning and the page is not guaranteed to
     * still exist after out of scope. The returned page is meant to be used
     * immediately after and not stored elsewhere. Ownership of the page is
     * exclusive to the pager, which may destruct it at any time. The timing
     * of page writes to disk is also not deterministically defined. In other
     * words, the pager will flush pages to disk as it sees fit and users of
     * this class should not rely on specific write timings.
     *
     * @throw std::invalid_argument if the pageID is out of bounds or if the
     * page is not currently used right now (freed).
     *
     * @throw std::runtime_error if the pageID does not correspond with the
     * correct page type T. IE: If a page with ID 3 has been constructed with
     * createPage of type Btree Page, you can only call getPage(3) with type
     * Btree page from now on. if you dont, it will throw a runtime error.
     *
     * @return a reference to the requested page.
     */
    template<typename T>
    T &getPage(size_t pageID) {
        static_assert(std::is_base_of<Page, T>::value, "T must be a derived class of Page");

        if (pageID >= m_ioHandler.getNumBlocks())
            throw std::invalid_argument("pageID out of bounds");

        if (!std::is_same_v<T, FSMPage> && isFree(pageID))
            throw std::invalid_argument("that page is not being used right now");

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

    /**
     * @brief Creates a new page
     *
     * @tparam T type of page that you would like to create. This type must
     * be a subclass of class Page and have a constructor that takes in a
     * ByteVec& and page number.
     *
     * @throws std::runtime_error if the max page limit has been reached.
     *
     * @return the new page that was created.
     *
     * Note: The returned page will ALWAYS be 0-initialized besides the start
     * of the page, which contains the page_type_id.
     */
    template<typename T, typename ...Args>
    T &createNewPage(Args&&... args) {
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

    /**
     * @brief Frees a page
     *
     * @tparam T The type of the page that is expected.
     *
     * @param pageID the page id to be freed.
     *
     * @throw std::invalid_argument if the pageID is out of bounds or if the
     * page is already freed.
     */
    template<typename T>
    void freePage(size_t pageID) {
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

    /**
     * All pages are guaranteed to be written
     */
    ~Pager() {
        // write everything in cache to disk
        ByteVec temp(cts::PG_SZ);
        for (const auto &page_ptr: m_cache) {
            page_ptr->toBytes(temp);
            m_ioHandler.writeBlock((void *) temp.data(), page_ptr->getPageID());
        }
    }

private:
    // helper functions to manipulate the free space bitmap
    void freePageBit(size_t pageID) {
        //Q: if calculated pageID >= total # blocks (IOHandler)
        //    - throw error, out of bounds pageID
        if (pageID >= m_ioHandler.getNumBlocks())
            throw std::invalid_argument("pageID is out of bounds");

        //1. calculate which FSMPageId the target pageID is in
        FSMPage* currPage = &getPage<FSMPage>(cts::FSM_PAGE_NO);
        size_t blocksPerPage = FSMPage::getBlocksInPage();

        //2. get page(maybe using linked-list style search), set the bit to 0
        // the FSMPage (node) that contains the page we want to free
        // 0-indexed
        int nodeNo = pageID / blocksPerPage;
        for (int i = 0; i < nodeNo; i++) {
            if (!currPage->hasNextPage())
                throw std::runtime_error("FSMPage has no next page");
            currPage = &getPage<FSMPage>(currPage->getNextPageID());
        }

        int localIdx = pageID % blocksPerPage;
        if (currPage->isFree(localIdx))
            throw std::invalid_argument("page is already free, cannot free");
        currPage->freeBit(localIdx);
    }

    size_t allocPageBit() {
        FSMPage* currPage = &getPage<FSMPage>(cts::FSM_PAGE_NO);

        //1. if current there is free bit in bitmap:
        //    - return next free bit in bitmap and set bit to 1
        //2. else
        //    - start from first page of FSM bitmap, keep going
        //     until you find a page with a free bitmap
        int idx = 0;
        while (currPage->getSpaceLeft() == 0 && currPage->hasNextPage()) {
            idx++;
            currPage = &getPage<FSMPage>(currPage->getNextPageID());
        }

        // free bitmap has been found
        if (currPage->getSpaceLeft() > 0) {
            size_t localIdx= currPage->findNextFree();
            currPage->allocBit(localIdx);
            // need to create new block
            if ((idx * FSMPage::getBlocksInPage()) + localIdx == m_ioHandler.getNumBlocks()) {
                size_t id = m_ioHandler.createNewBlock();
                assert(id == ((idx * FSMPage::getBlocksInPage()) + localIdx));
            }
            return (idx * FSMPage::getBlocksInPage()) + localIdx;
        }

        //3. else you go through all of FSM pages and find no free bitmap
        //    - request new page from IOHandler
        //    - set previous last page->next to new pageNo
        //    - instantiate new page with free bitmap (set first bit to 1, since
        //      first page is for bitmap)
        size_t newPageID = m_ioHandler.createNewBlock();
        currPage->setNextPageID(newPageID);
        m_cache.emplace_back(std::make_unique<FSMPage>(newPageID));
        currPage = &getPage<FSMPage>(newPageID);

        int localIdx = currPage->findNextFree();
        currPage->allocBit(localIdx);
        int new_id = m_ioHandler.createNewBlock();
        idx++;
        assert(new_id == (idx * FSMPage::getBlocksInPage() + localIdx));
        return (idx * FSMPage::getBlocksInPage()) + localIdx;
    }

    bool isFree(size_t pageID) {
        FSMPage* currPage = &getPage<FSMPage>(cts::FSM_PAGE_NO);
        size_t pageIdx = pageID / FSMPage::getBlocksInPage();
        for (int i = 0; i < pageIdx; i++) {
            if (!currPage->hasNextPage())
                throw std::runtime_error("FSMPage has no next page");
            currPage = &getPage<FSMPage>(currPage->getNextPageID());
        }
        size_t localIdx = pageID % FSMPage::getBlocksInPage();
        return currPage->isFree(localIdx);
    }

    IOHandler &m_ioHandler;

    // TODO: LRU cache using implementation here: https://leetcode.com/problems/lru-cache/
    //      - but use template for key and val, and make sure val is a unique_ptr<val> instead
    //      - key is page number, val is PagePtr
    vector<PagePtr> m_cache;
};

#endif //KNDB_PAGER_HPP
