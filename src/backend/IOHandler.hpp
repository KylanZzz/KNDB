//
// Created by Kylan Chen on 2/9/25.
//

#ifndef KNDB_IOHANDLER_HPP
#define KNDB_IOHANDLER_HPP

#include <cstddef>
#include <string>

using std::string;

/**
 * @class IOHandler
 * @brief Provides an interface for low-level file I/O operations.
 *
 * Handles file interactions such as reading and writing, as well as
 * increasing the size of the file when needed.
 */
class IOHandler {
public:
    /**
     * @brief Constructs an IOHandler for a given file.
     * @param fileName The name of the file to be used for storage.
     *
     * Opens or creates a file for managing database storage. If
     * the file is created, default size is 0.
     */
    explicit IOHandler(string fileName);

    /**
     * @brief Gets the total number of blocks in the database file.
     * @return The number of blocks currently allocated.
     */
    size_t getNumBlocks();

    /**
     * @brief Creates a new block in the file.
     * @return The Block No of the newly created block (0-indexed).
     *
     * Expands the file by appending an empty block.
     */
    size_t createNewBlock();

    /**
     * @brief Writes data to a block in the file.
     * @param arr Pointer to the data to be written.
     * @param BlockNo The ID of the block to write to (0-indexed).
     *
     * Uses low-level system calls to write data to the specified block.
     */
    void writeBlock(void *arr, size_t BlockNo);

    /**
     * @brief Reads data from a block in the file.
     * @param arr Pointer to the buffer where data will be read into.
     * @param BlockNo The ID of the block to read from (0-indexed).
     *
     * Uses low-level system calls to read data from the specified block.
     */
    void readBlock(void *arr, size_t BlockNo);

    ~IOHandler();

private:
    int m_fd;
    size_t m_blocks;
};


#endif //KNDB_IOHANDLER_HPP
