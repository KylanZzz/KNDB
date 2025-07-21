//
// Created by kylan on 7/20/2025.
//

#ifndef PAGECACHE_TPP
#define PAGECACHE_TPP

namespace backend {

template <typename T>
T& PageCache::retrievePage(pgid_t pageID) {
    if (m_cache.contains(pageID))
        return dynamic_cast<T &>(*m_cache[pageID]);

    // if not in cache, load it into cache
    PgArr<byte> buf;
    m_ioHandler.readBlock(buf.data(), pageID);

    // create a new page in cache (which is just a list for now)
    m_cache.emplace(pageID, std::make_unique<T>(buf, pageID));

    // dynamic cast to catch potential type safety issues
    return dynamic_cast<T &>(*m_cache[pageID]);
}


}

#endif //PAGECACHE_TPP
