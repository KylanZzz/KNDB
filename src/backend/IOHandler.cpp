//
// Created by Kylan Chen on 2/9/25.
//

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utility.hpp"
#include "IOHandler.hpp"

IOHandler::IOHandler(string fileName) {
    m_fd = open(fileName.c_str(), O_RDWR | O_CREAT, 0644);

    if (m_fd == -1)
        throw std::runtime_error("Failed to open file");

    struct stat f_stat;
    if (fstat(m_fd, &f_stat) == -1) {
        close(m_fd);
        throw std::runtime_error("Failed to get stat of file for size");
    }

    m_blocks = f_stat.st_size / cts::PG_SZ;
}

size_t IOHandler::getNumBlocks() {
    return m_blocks;
}

size_t IOHandler::createNewBlock() {
    size_t new_sz = ++m_blocks * cts::PG_SZ;
    if (ftruncate(m_fd, new_sz) == -1)
        throw std::runtime_error("Failed to increase file size");

    return m_blocks - 1;
}

void IOHandler::writeBlock(void *arr, size_t BlockNo) {
    if (BlockNo >= m_blocks)
        throw std::runtime_error("BlockNo out of bounds");

    if (pwrite(m_fd, arr, cts::PG_SZ, BlockNo * cts::PG_SZ) == -1)
        throw std::runtime_error("error while writing file");

    //TODO: usually we should not flush after every write;
    // this is just for development/debugging purposes
    fsync(m_fd);
}

void IOHandler::readBlock(void *arr, size_t BlockNo) {
    if (BlockNo >= m_blocks)
        throw std::runtime_error("BlockNo out of bounds");

    if (pread(m_fd, arr, cts::PG_SZ, BlockNo * cts::PG_SZ) == -1)
        throw std::runtime_error("error while writing file");
}

IOHandler::~IOHandler() {
    fsync(m_fd);
    close(m_fd);
}