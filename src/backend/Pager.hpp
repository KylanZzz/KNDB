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

#include "utility.hpp"
#include "IOHandler.hpp"
#include "Page.hpp"

using std::vector;
using ByteVec = std::vector<std::byte>;
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
     * @param ioHandler handles disk I/O requests.
     */
    Pager(IOHandler &ioHandler);

    /**
     * @brief Retrieves a page.
     * @tparam T type of page that is expected to be returned. This type must be a subclass of class Page
     *           and have a constructor that takes in a ByteVec& and page number.
     * @param pageID the requested page's id.
     * @return the requested page.
     */
    template<typename T>
    T &getPage(int pageID);

    /**
     * @brief Creates a new page.
     * @tparam T type of page that you would like to create. This type must be a subclass of class Page
     *           and have a constructor that takes in a ByteVec& and page number.
     * @return the new page that was created.
     */
    template<typename T>
    T &createNewPage();

    /**
     * @brief Frees a page
     * @tparam T The type of the page that is expected.
     * @param pageID the page id to be freed.
     */
    template<typename T>
    void freePage(int pageID);

    ~Pager();

private:
    void freePageBit(size_t pageID);
    size_t allocPageBit();
    void initFSMPage(size_t pageID);

    IOHandler &m_ioHandler;

    // TODO: LRU cache using implementation here: https://leetcode.com/problems/lru-cache/
    //      - but use template for key and val, and make sure val is a unique_ptr<val> instead
    //      - key is page number, val is PagePtr
    vector<PagePtr> m_cache;
};

#endif //KNDB_PAGER_HPP
