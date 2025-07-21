//
// Created by kylan on 7/20/2025.
//

#include "PageCache.hpp"
#include "assume.hpp"

namespace backend {

PageCache::PageCache(IOHandler& ioHandler): m_ioHandler(ioHandler) {
}

void PageCache::insertPage(std::unique_ptr<Page> page) {
    m_cache[page->getPageID()] = std::move(page);
}

void PageCache::writePage(pgid_t pageID) {
    if (!m_cache.contains(pageID)) {
        return;
    }

    auto& pageToWrite = m_cache[pageID];
    PgArr<byte> buf;
    pageToWrite->toBytes(buf);
    m_ioHandler.writeBlock(buf.data(), pageToWrite->getPageID());
}

PageCache::~PageCache() {
    PgArr<byte> buf;
    for (const auto& [pageID, page]: m_cache) {
        page->toBytes(buf);
        m_ioHandler.writeBlock((void*) buf.data(), pageID);
    }
}


}
