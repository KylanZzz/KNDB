//
// Created by Kylan Chen on 2/9/25.
//

#ifndef KNDB_IOHANDLER_HPP
#define KNDB_IOHANDLER_HPP

#include <cstddef>
#include <string>

#include "kndb_types.hpp"
#ifdef _WIN32
    #include <windows.h>
#else // includes for posix
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif //_WIN32

namespace backend {

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
     *
     * @param fileName The name of the file to be used for storage.
     *
     * Opens or creates a file for managing database storage. If
     * the file is created, default size is 0.
     */
    explicit IOHandler(std::string_view fileName);

    /**
     * @brief Gets the total number of blocks in the database file.
     *
     * @return The number of blocks currently allocated.
     */
    u32 getNumBlocks() const;

    /**
     * @brief Creates a new block in the file.
     *
     * @return The Block No of the newly created block (0-indexed).
     */
    u32 createNewBlock();

    /**
     * @brief Writes data to a block in the file.
     *
     * @param arr Pointer to the data to be written.
     * @param BlockNo The ID of the block to write to (0-indexed).
     */
    void writeBlock(void *arr, u32 BlockNo) const;

    /**
     * @brief Reads data from a block in the file.
     *
     * @param arr Pointer to the buffer where data will be read into.
     * @param BlockNo The ID of the block to read from (0-indexed).
     */
    void readBlock(void *arr, u32 BlockNo) const;

    ~IOHandler();

private:
#ifdef _WIN32
    HANDLE m_handle;
#else
    int m_fd;
#endif // _WIN32
    u32 m_blocks;
};

} // namespace backend

#endif //KNDB_IOHANDLER_HPP
