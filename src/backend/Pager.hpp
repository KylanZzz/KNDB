//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_PAGER_HPP
#define KNDB_PAGER_HPP

#include <unordered_map>

#include "kndb_types.hpp"
#include "Page.hpp"
#include "IOHandler.hpp"


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
     * @param ioHandler handles disk I/O requests.
     */
    Pager(IOHandler &ioHandler);

    /**
     * @brief Retrieves a page.
     *
     * @tparam T type of page that is expected to be returned.  This type
     * must be a subclass of class Page and have a constructor that takes in a
     * Vec<Byte>& and page number.
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
    T &getPage(u32 pageID);

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
     * Note: The returned page will ALWAYS be 0-initialized besides the start
     * of the page, which contains the page_type_id.
     */
    template<typename T, typename ...Args>
    T &createNewPage(Args &&... args);

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
    void freePage(u32 pageID);

    /**
     * All pages are guaranteed to be written
     */
    ~Pager();

private:
    // helper functions to manipulate the free space bitmap
    void freePageBit(u32 pageID);

    // allocates a pg in the bitmap and returns the pg id
    u32 allocPageBit();

    bool isFree(u32 pageID);

    IOHandler &m_ioHandler;

    // TODO: LRU cache using implementation here: https://leetcode.com/problems/lru-cache/
    //      - but use template for key and val, and make sure val is a unique_ptr<val> instead
    //      - key is page number, val is PagePtr
    std::unordered_map<u32, Ptr<Page>> m_cache;
};

} // namespace backend

#include "Pager.tpp"

#endif //KNDB_PAGER_HPP
