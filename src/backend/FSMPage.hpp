//
// Created by Kylan Chen on 2/28/25.
//

#ifndef KNDB_FSMPAGE_HPP
#define KNDB_FSMPAGE_HPP

#include "Page.hpp"


class FSMPage : public Page {
public:
    FSMPage(size_t PageID, ByteVec &bytes);

    FSMPage(size_t PageID);

    bool isFree(size_t idx);

    bool hasNextPage();

    size_t getSpaceLeft();

    void allocBit(size_t idx);

    void freeBit(size_t idx);

    size_t findNextFree();

    size_t getNextPageID();

    void setNextPageID(size_t PageID);

    void to_bytes(ByteVec &vec) override;

private:

    static constexpr size_t NO_NEXT_PAGE = std::numeric_limits<size_t>::max();

    std::vector<u_int8_t> m_bitmap;
    size_t m_nextPageID;
    size_t m_freeBlocks;
};


#endif //KNDB_FSMPAGE_HPP
