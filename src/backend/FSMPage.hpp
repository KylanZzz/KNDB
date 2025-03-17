//
// Created by Kylan Chen on 2/28/25.
//

#ifndef KNDB_FSMPAGE_HPP
#define KNDB_FSMPAGE_HPP

#include "kndb_types.hpp"
#include "Page.hpp"
#include "constants.hpp"

using namespace kndb;

/**
 * @class FSMPage
 * @brief Represents a Free Space Map page in the database.
 *
 * FSMPage keeps track of free and allocated blocks using a bitmap.
 * It allows querying, allocating, and freeing space within a page.
 * The FSMPage also maintains a link to the next FSM_ID page if more space maps
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
    FSMPage(std::span<const byte> bytes, u32 pageID);

    /**
    * @brief Constructs a new, empty FSMPage.
    * @param pageID The ID of the page.
    */
    FSMPage(u32 pageID);

    /**
     * @brief Checks if a specific bit (block) in the bitmap is free.
     *
     * @param idx The index of the bit to check.
     *
     * @throws std::runtime_error If idx is out of bounds.
     *
     * @return True if the bit is free, false otherwise.
     */
    bool isFree(u32 idx);

    /**
     * @brief Checks if this FSM_ID page has a reference to the next FSM_ID page.
     *
     * @return True if there is a next FSM_ID page, false otherwise.
     */
    bool hasNextPage() const;

    /**
     * @brief Gets the number of free blocks remaining in the bitmap.
     *
     * @return The count of free blocks in this FSM_ID page.
     */
    u32 getSpaceLeft() const;

    /**
     * @brief Allocates a bit (block) in the bitmap.
     *
     * @param idx The index of the bit to allocate.
     *
     * @throws std::invalid_argument If the bit is already allocated.
     */
    void allocBit(u32 idx);

    /**
    * @brief Frees a previously allocated bit (block) in the bitmap.
     *
    * @param idx The index of the bit to free.
     *
    * @throws std::invalid_argument If the bit is already free.
    */
    void freeBit(u32 idx);

    /**
     * @brief Finds the next available free block.
     *
     * @return The index of the next free bit.
     *
     * @throws std::invalid_argument If no free bits are available.
     */
    u32 findNextFree();

    /**
     * @brief Retrieves the ID of the next FSM_ID page.
     *
     * @return The PageID of the next FSM_ID page.
     *
     * @throws std::invalid_argument If there is no next page.
     */
    u32 getNextPageID() const;

    /**
     * @brief Sets the next FSM_ID page ID.
     *
     * @param pageID The ID of the next FSM_ID page.
     */
    void setNextPageID(u32 pageID);

    /**
     * @brief Get the max number of blocks that a single FSMPage can represent.
     *
     * @return The number of blocks (or pages).
     */
    static u32 getBlocksInPage() { return (cts::PG_SZ - sizeof(u32) * 3) * 8; }

    void toBytes(std::span<byte> buf) override;

private:
    Vec<u8> m_bitmap;
    u32 m_nextPageID;
    u32 m_freeBlocks;
};


#endif //KNDB_FSMPAGE_HPP
