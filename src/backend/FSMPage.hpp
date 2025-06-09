//
// Created by Kylan Chen on 2/28/25.
//

#ifndef KNDB_FSMPAGE_HPP
#define KNDB_FSMPAGE_HPP

#include "kndb_types.hpp"
#include "Page.hpp"
#include "constants.hpp"

/**
 * @class FSMPage
 * @brief Represents a Free Space Map page in the database.
 *
 * FSMPage keeps track of free and allocated blocks using a bitmap.
 * It allows querying, allocating, and freeing space within a page.
 * The FSMPage also maintains a link to the next FSM_ID page if more space maps
 * are needed.
 */

namespace backend {
    class FSMPage : public Page {
    public:
        /**
         * @brief Constructs an FSMPage from existing serialized data.
         *
         * @param pageID The ID of the page.
         * @param bytes serialized data
         */
        FSMPage(std::span<const byte> bytes, pgid_t pageID);

        /**
        * @brief Constructs a new, empty FSMPage.
        * @param pageID The ID of the page.
        */
        FSMPage(pgid_t pageID);

        /**
         * @brief Checks if a specific bit (block) in the bitmap is free.
         *
         * @param idx The index of the bit to check.
         *
         * @return True if the bit is free, false otherwise.
         */
        bool isFree(bitmapidx_t idx) const;

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
        bitmapidx_t getSpaceLeft() const;

        /**
         * @brief Allocates a bit (block) in the bitmap.
         *
         * @param idx The index of the bit to allocate.
         */
        void allocBit(bitmapidx_t idx);

        /**
        * @brief Frees a previously allocated bit (block) in the bitmap.
        *
        * @param idx The index of the bit to free.
        */
        void freeBit(bitmapidx_t idx);

        /**
         * @brief Finds the next available free block.
         *
         * @return The index of the next free bit.
         */
        bitmapidx_t findNextFree() const;

        /**
         * @brief Retrieves the ID of the next FSM_ID page.
         *
         * @return The PageID of the next FSM_ID page.
         *
         * @throws std::invalid_argument If there is no next page.
         */
        pgid_t getNextPageID() const;

        /**
         * @brief Sets the next FSM_ID page ID.
         *
         * @param pageID The ID of the next FSM_ID page.
         */
        void setNextPageID(pgid_t pageID);

        /**
         * @brief Get the max number of blocks that a single FSMPage can represent.
         *
         * @return The number of blocks (or pages).
         */
        static bitmapidx_t getBlocksInPage() { return (cts::PG_SZ - sizeof(u32) * 3) * 8; }

        void toBytes(std::span<byte> buf) override;

    private:
        Vec<u8> m_bitmap;
        pgid_t m_nextPageID;
        bitmapidx_t m_freeBlocks;
    };
}

#endif //KNDB_FSMPAGE_HPP
