//
// Created by Kylan Chen on 12/20/24.
//

#ifndef KNDB_PAGE_HPP
#define KNDB_PAGE_HPP

#include <cstddef>
#include <span>

#include "kndb_types.hpp"

using namespace kndb;

/**
 * @class Page
 * @brief Represents a page in the database
 *
 * Represents a page of data in the database that can be written
 * to disk for persistence.
 */
class Page {
protected:
    u32 m_pageID;

public:
    /**
     * @brief Constructs a Page with a given ID.
     * @param pageID The unique page ID.
     */
    explicit Page(u32 pageID) : m_pageID(pageID) {};

    /**
     * @brief Gets the page ID.
     * @return The page ID.
     */
    u32 getPageID() const { return m_pageID; }

    /**
     * @brief Serializes the page into a byte vector.
     * @param buffer The container that will be serialized to.
     */
    virtual void toBytes(std::span<byte> buffer) = 0;

    virtual ~Page() = default;
};


#endif //KNDB_PAGE_HPP
