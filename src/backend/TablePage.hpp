//
// Created by Kylan Chen on 3/3/25.
//

#ifndef KNDB_TABLEPAGE_HPP
#define KNDB_TABLEPAGE_HPP

#include "Page.hpp"

class TablePage : public Page {
public:
    TablePage(ByteVec &bytes, size_t pageID);

    vector<variants> getTypes();

    size_t getBtreePageNo();

    void addTuple();

    void removeTuple();

    void init(const vector<variants>& types, size_t btreePageID);

    void toBytes(ByteVec &vec) override;

private:
    vector<variants> m_types;
    size_t m_btreePageID;
    size_t m_numTuples;
};


#endif //KNDB_TABLEPAGE_HPP
