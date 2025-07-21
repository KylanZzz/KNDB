//
// Created by Kylan Chen on 3/8/25.
//

#include "Pager.hpp"
#include "assume.hpp"
#include "kndb_types.hpp"

namespace backend {

Pager::Pager(FreeSpaceMap &freeSpaceMap, IOHandler &ioHandler, PageCache& pageCache) :
    m_freeSpaceMap(freeSpaceMap), m_ioHandler(ioHandler), m_pageCache(pageCache) {}

bool Pager::isFree(pgid_t pageID) const {
    if (pageID >= m_ioHandler.getNumBlocks()) {
        return true;
    }
    return m_freeSpaceMap.isFree(pageID);
}

void Pager::freePage(pgid_t pageID) const {
    ASSUME_S(pageID < m_ioHandler.getNumBlocks(), "PageID is out of bounds");
    ASSUME_S(!m_freeSpaceMap.isFree(pageID), "That page is freed already");

    m_freeSpaceMap.freeBit(pageID);
}


} // namespace backend

