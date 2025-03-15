//
// Created by Kylan Chen on 2/9/25.
//

#include "utility.hpp"
#include "IOHandler.hpp"

size_t IOHandler::getNumBlocks() const {
    return m_blocks;
}

IOHandler::IOHandler(std::string_view fileName) {
#ifdef _WIN32
    m_handle = CreateFile(
        String(fileName).c_str(),            // File name
        GENERIC_READ | GENERIC_WRITE, // Access mode
        0,                         // No sharing
        nullptr,                      // Security attributes
        OPEN_ALWAYS,             // Create new or overwrite
        FILE_ATTRIBUTE_NORMAL,      // Normal file
        nullptr        // No template
    );


    if (m_handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open file");

    // Get file size
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(m_handle, &fileSize)) {
        CloseHandle(m_handle);
        throw std::runtime_error("Failed to get file size");
    }

    m_blocks = fileSize.QuadPart / cts::PG_SZ;
#else //_WIN32
    m_fd = open(String(fileName).c_str(), O_RDWR | O_CREAT, 0644);

    if (m_fd == -1)
        throw std::runtime_error("Failed to open file");

    struct stat f_stat;
    if (fstat(m_fd, &f_stat) == -1) {
        close(m_fd);
        throw std::runtime_error("Failed to get stat of file for size");
    }

    m_blocks = f_stat.st_size / cts::PG_SZ;
#endif //_WIN32
}

size_t IOHandler::createNewBlock() {
#ifdef _WIN32
    LARGE_INTEGER newPos;
    newPos.QuadPart = ++m_blocks * cts::PG_SZ;
    if (!SetFilePointerEx(m_handle, newPos, nullptr, FILE_BEGIN))
        throw std::runtime_error("Failed to move file pointer");

    if (!SetEndOfFile(m_handle))
        throw std::runtime_error("Failed to truncate file");
#else
    size_t new_sz = ++m_blocks * cts::PG_SZ;
    if (ftruncate(m_fd, new_sz) == -1)
        throw std::runtime_error("Failed to increase file size");
#endif //_WIN32

    return m_blocks - 1;
}

void IOHandler::writeBlock(void *arr, size_t BlockNo) const {
    if (BlockNo >= m_blocks)
        throw std::runtime_error("BlockNo out of bounds");
#ifdef _WIN32
    OVERLAPPED overlapped = {};
    overlapped.Offset = static_cast<DWORD>(BlockNo * cts::PG_SZ);
    overlapped.OffsetHigh = static_cast<DWORD>((BlockNo * cts::PG_SZ) >> 32);

    DWORD written;
    if (!WriteFile(m_handle, arr, cts::PG_SZ, &written, &overlapped) || written != cts::PG_SZ)
        throw std::runtime_error("error while writing file");
    //TODO: usually we should not flush after every write;
    // this is just for development/debugging purposes
    if (!FlushFileBuffers(m_handle))
        throw std::runtime_error("Error while flushing file buffers");
#else

    if (pwrite(m_fd, arr, cts::PG_SZ, BlockNo * cts::PG_SZ) == -1)
        throw std::runtime_error("error while writing file");

    //TODO: usually we should not flush after every write;
    // this is just for development/debugging purposes
    if (fsync(m_fd) == -1)
        throw std::runtime_error("Error while fsyncing file");
#endif
}

void IOHandler::readBlock(void *arr, size_t BlockNo) const {
    if (BlockNo >= m_blocks)
        throw std::runtime_error("BlockNo out of bounds");
#ifdef _WIN32
    OVERLAPPED overlapped = {};
    overlapped.Offset = static_cast<DWORD>(BlockNo * cts::PG_SZ);
    overlapped.OffsetHigh = static_cast<DWORD>((BlockNo * cts::PG_SZ) >> 32);

    DWORD bytesRead;
    if (!ReadFile(m_handle, arr, cts::PG_SZ, &bytesRead, &overlapped) || bytesRead != cts::PG_SZ)
        throw std::runtime_error("error while reading file");
#else
    if (pread(m_fd, arr, cts::PG_SZ, BlockNo * cts::PG_SZ) == -1)
        throw std::runtime_error("error while reading file");
#endif//_WIN32
}

IOHandler::~IOHandler() {
#ifdef _WIN32
    FlushFileBuffers(m_handle);
    CloseHandle(m_handle);
#else
    fsync(m_fd);
    close(m_fd);
#endif//_WIN32
}
