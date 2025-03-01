//
// Created by Kylan Chen on 2/27/25.
//

#ifndef KNDB_FREESPACEMAP_HPP
#define KNDB_FREESPACEMAP_HPP

#include <cassert>

#include "IOHandler.hpp"
#include "Pager.hpp"

/**
 * @class FreeSpaceMap
 * @brief Manages all the information about the free space bitmaps in the
 *      database.
 *
 * Provides an interface to allocate new blocks and free old blocks in the
 * database. If the database has just been created, this class will also be
 * responsible for creating and populating the first free space bitmap.
 */
class FreeSpaceMap {
public:
    /**
     * @brief Constructs a Free Space Map.
     *
     * @param ioHandler reference to IOHandler.
     * @param startPageId the pageID of the first bitmap.
     */
    FreeSpaceMap(IOHandler &ioHandler, size_t startPageId);

    /**
     * @brief Sets the Pager instance used for fetching pages.
     *
     * Ensures the pager is only set once. This function should be called after
     * initialization to link FreeSpaceMap with the database’s paging system.
     *
     * @param pager Pointer to the Pager instance.
     * @throws Assertion failure if `m_pager` is already set.
     */
    void setPager(Pager *pager) {
        assert(m_pager == nullptr);
        m_pager = pager;
    }

    /**
     * @brief Allocates a new free page.
     *
     * Searches the free space map(s) for an available page, marks it as
     * allocated, and returns its page ID.
     *
     * @return The allocated page ID.
     * @throws std::runtime_error If no free pages are available.
     */
    size_t allocPage();


    /**
     * @brief Frees a previously allocated page.
     *
     * Marks the given page ID as free in the free space map, allowing it to be
     * reused in future allocations.
     *
     * @param pageID The ID of the page to be freed.
     * @throws std::invalid_argument If `pageID` is already free.
     */
    void freePage(size_t pageID);

private:
    void initFSMPage(size_t pageID);

    IOHandler &m_ioHandler;
    Pager *m_pager;
    size_t m_startPageId;
};


#endif //KNDB_FREESPACEMAP_HPP
