//
// Created by Kylan Chen on 2/28/25.
//

#ifndef KNDB_FSMPAGE_HPP
#define KNDB_FSMPAGE_HPP

#include "Page.hpp"


class FSMPage : public Page {
public:
    FSMPage(ByteVec &bytes, size_t pageID);

    bool isFree(size_t idx);

    size_t getSpaceLeft();

    void allocBit(size_t idx);

    void freeBit(size_t idx);

    size_t getNextPageID();

    void to_bytes(ByteVec &vec) override;

private:
    ByteVec m_bitmap;
};


#endif //KNDB_FSMPAGE_HPP
