//
// Created by kylan on 7/20/2025.
//

#ifndef FREESPACEMAP_HPP
#define FREESPACEMAP_HPP

#include "PageCache.hpp"
#include "kndb_types.hpp"

namespace backend {

/**
 * @class FreeSpaceMap
 * @brief Manages free and used pages in the database via a bitmap-based paging system.
 *
 * The FreeSpaceMap tracks allocation status for all pages in the database. It uses a linked list of
 * FSMPage instances, each of which tracks the allocation bitmap for a fixed number of pages (e.g., 32).
 *
 * Each FSMPage is stored as a page in the database and cached via PageCache. The first bit (bit 0) of
 * each FSMPage refers to the page itself; all other bits represent usable page IDs.
 *
 * This class does not perform any disk I/O directly. It relies on PageCache to retrieve or store FSMPages.
 * It also does not create new FSMPages—it assumes the caller (usually Pager) checks if space is full
 * (via isFull()) and creates new FSMPages before calling allocBit(). New pages can be linked using
 * linkFSMPage().
 *
 * This class does not perform bounds checking on pageIDs for performance. It is the caller's
 * responsibility to ensure correctness.
 */
class FreeSpaceMap {
public:
    /**
    * @brief Constructs a FreeSpaceMap backed by the given PageCache.
    * @param cache Reference to a PageCache used to retrieve and store FSMPage data.
    */
    FreeSpaceMap(PageCache& cache);

    /**
     * @brief Allocates a free page from the bitmap.
     *
     * Caller must ensure space is available by calling isFull() beforehand and linking a new FSMPage if needed.
     *
     * @return The pgid_t (page ID) of the newly allocated page.
     * @throws Aborts if no space is available or internal invariant is violated.
     */
    pgid_t allocBit();

    /**
     * @brief Marks the given page ID as free.
     *
     * Caller must ensure the page is currently allocated. Calling freeBit on an already freed
     * page is undefined behavior.
     *
     * @param pageID The ID of the page to mark as free.
     */
    void freeBit(pgid_t pageID);

    /**
     * @brief Checks whether the given page ID is marked free.
     *
     * Caller must ensure the pageID is within range. No bounds check is performed.
     *
     * @param pageID The ID of the page to check.
     * @return True if the page is free, false otherwise.
     */
    bool isFree(pgid_t pageID);

    /**
     * @brief Checks if all current FSMPages are full.
     *
     * This is used to determine whether a new FSMPage must be allocated.
     *
     * @return True if no FSMPage has free space, false otherwise.
     */
    bool isFull();

    /**
     * @brief adds a new (empty) FSMPage to the end of free linked list.
     *
     * This is used to add an FSMPage that has already been allocated to the
     * front of the free space bitmap linked list.
     *
     * @param newFSMPageID of the new FSMPage
     */
    void linkFSMPage(pgid_t newFSMPageID);

private:
    PageCache& m_cache;
};


}

#endif //FREESPACEMAP_HPP
