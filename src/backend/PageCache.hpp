//
// Created by kylan on 7/20/2025.
//

#ifndef PAGECACHE_HPP
#define PAGECACHE_HPP

#include "IOHandler.hpp"
#include "kndb_types.hpp"
#include "Page.hpp"
#include "list"
#include "unordered_map"

namespace backend {

/**
 * @brief Manages in-memory caching of Page objects to reduce disk I/O.
 *
 * PageCache handles retrieval, insertion, eviction, and writing of pages.
 * It interacts directly with the IOHandler to read/write pages to disk when
 * necessary.
 */
class PageCache {
public:
    /**
     * @brief Constructs a PageCache with a reference to the underlying IOHandler.
     *
     * @param ioHandler Reference to the IOHandler used for disk I/O.
     * @param capacity Max cache size.
     */
    PageCache(IOHandler& ioHandler, size_t capacity);

    /**
     * @brief Retrieves a typed reference to a cached page, or loads it from disk if not cached.
     *
     * @tparam T Derived type of Page expected by the caller.
     * @param pageID ID of the page to retrieve.
     * @return Reference to the loaded or cached page of type T.
     * @throws std::bad_cast if the cached page cannot be cast to T.
     * @throws std::runtime_error if the page does not exist on disk.
     */
    template <typename T>
    T& retrievePage(pgid_t pageID);

    /**
     * @brief Inserts a newly created page into the cache.
     *
     * @param page A unique_ptr to the Page object to cache.
     *
     * If there was a previous page with the same pageID,
     * then it will be overwritten.
     */
    void insertPage(Ptr<Page> page);

    /**
     * @brief Writes the given page to disk via the IOHandler.
     *
     * @param pageID ID of the page to evict from the cache.
     */
    void writePage(pgid_t pageID);

    /**
     * @brief Flushes all cached pages to disk.
     */
    ~PageCache();

private:
    using list_it = std::list<Ptr<Page>>::iterator;

    IOHandler& m_ioHandler;
    size_t m_capacity;
    std::list<Ptr<Page>> m_list;
    std::unordered_map<pgid_t, list_it> m_map;

    void updateLRU(Ptr<Page> page);
};

}

#include "PageCache.tpp"

#endif //PAGECACHE_HPP
