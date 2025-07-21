//
// Created by Kylan Chen on 3/8/25.
//

#include "Pager.hpp"
#include "assume.hpp"
#include "kndb_types.hpp"

namespace backend {

Pager::Pager(FreeSpaceMap &freeSpaceMap, IOHandler &ioHandler, PageCache& pageCache) :
    m_freeSpaceMap(freeSpaceMap), m_ioHandler(ioHandler), m_pageCache(pageCache) {}

} // namespace backend
