//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_PAGER_HPP
#define KNDB_PAGER_HPP

#include <unordered_map>

#include "kndb_types.hpp"
#include "IOHandler.hpp"
#include "FreeSpaceMap.hpp"
#include "PageCache.hpp"

/**
 * @class Pager
 * @brief Handles database page management.
 *
 * Manages reading, writing, and tracking of database pages. This class acts as
 * the main interface for other classes to use database pages.
 */
namespace backend {

class Pager {
public:

    /**
     * @brief Constructs a Pager.
     *
     * @param freeSpaceMap tracks pages with free space.
     * @param ioHandler handles disk I/O requests.
     * @param pageCache caches frequently used pages.
     */
    Pager(FreeSpaceMap& freeSpaceMap, IOHandler& ioHandler, PageCache &pageCache);

    /**
     * @brief Retrieves a page.
     *
     * @tparam T type of page that is expected to be returned.  This type
     * must be a subclass of class Page and have a constructor that takes in a
     * Vec<Byte>& and page number.
     *
     * @param pageID the requested page's id.
     *
     * @return a reference to the requested page.
     *
     * Note: This function call is non-owning and the page is not guaranteed to
     * still exist after out of scope. The returned page is meant to be used
     * immediately after and not stored elsewhere. Ownership of the page is
     * exclusive to the pager, which may destruct it at any time. The timing
     * of page writes to disk is also not deterministically defined. In other
     * words, the pager will flush pages to disk as it sees fit and users of
     * this class should not rely on specific write timings.
     */
    template<typename T>
    T &getPage(pgid_t pageID);

    /**
     * @brief Creates a new page
     *
     * @tparam T type of page that you would like to create. This type must
     * be a subclass of class Page and have a constructor that takes in a
     * Vec<Byte>& and page number.
     *
     * @throws std::runtime_error if the max page limit has been reached.
     *
     * @return the new page that was created.
     *
     * Note: This function will call the default constructor of whatever page
     * type it has created, which is the constructor with only one pageid_t
     * page id as the parameter.
     */
    template<typename T, typename ...Args>
    T &createNewPage(Args &&... args);

    /**
     * @brief Frees a page
     * @param pageID the page id to be freed.
     */
    void freePage(pgid_t pageID) const;

    /**
     * Checks whether a page is currently being used.
     * 
     * @param pageID the page id of the Page.
     * @return true if the page is not used and false otherwise.
     *
     * If the pageID passed in is out of bounds (larger than the
     * .db file), then it will return true.
     */
    bool isFree(pgid_t pageID) const;

    /**
     * All pages are guaranteed to be written
     */
    ~Pager() = default;

    Pager& operator=(Pager&& other) = delete;
    Pager& operator=(const Pager& other) = delete;
    Pager(Pager&& other) = delete;
    Pager(const Pager& other) = delete;

private:
    PageCache& m_pageCache;
    FreeSpaceMap& m_freeSpaceMap;
    IOHandler& m_ioHandler;
};

} // namespace backend

#include "Pager.tpp"

#endif //KNDB_PAGER_HPP
