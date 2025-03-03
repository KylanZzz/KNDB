//
// Created by Kylan Chen on 12/20/24.
//

#ifndef KNDB_PAGE_HPP
#define KNDB_PAGE_HPP

#include <vector>
#include <memory>
#include <cstddef>
#include <iostream>

#include "kndb_types.hpp"

using namespace kndb_types;

/**
 * @class Page
 * @brief Represents a page in the database
 *
 * Represents a page of data in the database that can be written
 * to disk for persistence.
 */
class Page {
protected:
    size_t m_pageID;

public:
    /**
     * @brief Constructs a Page with a given ID.
     * @param pageID The unique page ID.
     */
    explicit Page(size_t pageID) : m_pageID(pageID) {};

    /**
     * @brief Gets the page ID.
     * @return The page ID.
     */
    size_t getPageID() const { return m_pageID; }

    /**
     * @brief Serializes the page into a byte vector.
     * @param vec The byte vector to store serialized data.
     */
    virtual void toBytes(ByteVec &vec) = 0;

    virtual ~Page() = default;
};


#endif //KNDB_PAGE_HPP
