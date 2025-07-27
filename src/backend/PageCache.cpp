//
// Created by kylan on 7/20/2025.
//

#include "PageCache.hpp"
#include "assume.hpp"

namespace backend {

PageCache::PageCache(IOHandler& ioHandler, size_t capacity): m_ioHandler(ioHandler), m_capacity(capacity) {
}

void PageCache::updateLRU(Ptr<Page> page) {

    // 1. if key is in queue:
    pgid_t pageID = page->getPageID();
    if (m_map.contains(pageID)) {
        m_list.erase(m_map[pageID]);
    }

    // 2. add new node [key] to FRONT of list
    m_list.push_front(std::move(page));

    // 3. update map [key] -> new iterator
    m_map[pageID] = m_list.begin();

    // 4. if size > cap, remove BACK element
    if (m_list.size() > m_capacity) {
        pgid_t backPageID = m_list.back()->getPageID();
        writePage(backPageID);
        m_map.erase(backPageID);
        m_list.pop_back();
    }
}

void PageCache::insertPage(Ptr<Page> page) {
    updateLRU(std::move(page));
}

void PageCache::writePage(pgid_t pageID) {
    Page& page = *(*m_map[pageID]);
    if (m_map[pageID] == list_it()) {
        return;
    }

    PgArr<byte> buf;
    page.toBytes(buf);
    m_ioHandler.writeBlock(buf.data(), pageID);
}

PageCache::~PageCache() {
    PgArr<byte> buf;
    for (const auto& [pageID, page_it]: m_map) {
        auto& page = *(*page_it);
        page.toBytes(buf);
        m_ioHandler.writeBlock(buf.data(), pageID);
    }
}


}
