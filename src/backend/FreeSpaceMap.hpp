//
// Created by Kylan Chen on 2/27/25.
//

#ifndef KNDB_FREESPACEMAP_HPP
#define KNDB_FREESPACEMAP_HPP

#include <cassert>

#include "IOHandler.hpp"

// Forward declare pager to avoid circular dependency
class Pager;

class FreeSpaceMap {
public:
    FreeSpaceMap(IOHandler &ioHandler, size_t startPageId) : m_ioHandler(ioHandler), m_startPageId(startPageId) {};

    void setPager(Pager *pager) {
        assert(m_pager == nullptr);
        m_pager = pager;
    }

    size_t allocPage();

    void freePage(size_t pageID);

private:
    IOHandler &m_ioHandler;
    Pager *m_pager;
    size_t m_startPageId;
};


#endif //KNDB_FREESPACEMAP_HPP
