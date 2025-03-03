//
// Created by Kylan Chen on 2/28/25.
//

#ifndef KNDB_FSMPAGE_HPP
#define KNDB_FSMPAGE_HPP

#include "Page.hpp"
#include "constants.hpp"

/**
 * @class FSMPage
 * @brief Represents a Free Space Map page in the database.
 *
 * FSMPage keeps track of free and allocated blocks using a bitmap.
 * It allows querying, allocating, and freeing space within a page.
 * The FSMPage also maintains a link to the next FSM page if more space maps
 * are needed.
 */
class FSMPage : public Page {
public:
    /**
     * @brief Constructs an FSMPage from existing serialized data.
     *
     * @param pageID The ID of the page.
     * @param bytes serialized data
     */
    FSMPage(ByteVec &bytes, size_t pageID);

    /**
    * @brief Constructs a new, empty FSMPage.
    * @param pageID The ID of the page.
    */
    FSMPage(size_t pageID);

    /**
     * @brief Checks if a specific bit (block) in the bitmap is free.
     *
     * @param idx The index of the bit to check.
     *
     * @throws std::runtime_error If idx is out of bounds.
     *
     * @return True if the bit is free, false otherwise.
     */
    bool isFree(size_t idx);

    /**
     * @brief Checks if this FSM page has a reference to the next FSM page.
     *
     * @return True if there is a next FSM page, false otherwise.
     */
    bool hasNextPage();

    /**
     * @brief Gets the number of free blocks remaining in the bitmap.
     *
     * @return The count of free blocks in this FSM page.
     */
    size_t getSpaceLeft();

    /**
     * @brief Allocates a bit (block) in the bitmap.
     *
     * @param idx The index of the bit to allocate.
     *
     * @throws std::invalid_argument If the bit is already allocated.
     */
    void allocBit(size_t idx);

    /**
    * @brief Frees a previously allocated bit (block) in the bitmap.
     *
    * @param idx The index of the bit to free.
     *
    * @throws std::invalid_argument If the bit is already free.
    */
    void freeBit(size_t idx);

    /**
     * @brief Finds the next available free block.
     *
     * @return The index of the next free bit.
     *
     * @throws std::invalid_argument If no free bits are available.
     */
    size_t findNextFree();

    /**
     * @brief Retrieves the ID of the next FSM page.
     *
     * @return The PageID of the next FSM page.
     *
     * @throws std::invalid_argument If there is no next page.
     */
    size_t getNextPageID();

    /**
     * @brief Sets the next FSM page ID.
     *
     * @param pageID The ID of the next FSM page.
     */
    void setNextPageID(size_t pageID);

    /**
     * @brief Get the max number of blocks that a single FSMPage can represent.
     *
     * @return The number of blocks (or pages).
     */
    static size_t getBlocksInPage() { return (cts::PG_SZ - sizeof(size_t) * 3) * 8; }

    void toBytes(ByteVec &vec) override;

private:

    static constexpr size_t NO_NEXT_PAGE = std::numeric_limits<size_t>::max();

    std::vector<u_int8_t> m_bitmap;
    size_t m_nextPageID;
    size_t m_freeBlocks;
};


#endif //KNDB_FSMPAGE_HPP
