//
// Created by kylan on 7/20/2025.
//

#ifndef PAGECACHE_TPP
#define PAGECACHE_TPP

namespace backend {

template <typename T>
T& PageCache::retrievePage(pgid_t pageID) {
    if (m_map.contains(pageID)) {
        updateLRU(std::move(*m_map[pageID]));
    } else {
        PgArr<byte> buf;
        m_ioHandler.readBlock(buf.data(), pageID);
        Ptr<Page> page = std::make_unique<T>(buf, pageID);
        updateLRU(std::move(page));
    }

    return dynamic_cast<T &>(**m_map[pageID]);
}

}

#endif //PAGECACHE_TPP
