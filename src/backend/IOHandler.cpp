//
// Created by Kylan Chen on 2/9/25.
//

#include "utility.hpp"
#include "IOHandler.hpp"

namespace backend {

blockid_t IOHandler::getNumBlocks() const {
    return m_blocks;
}

IOHandler::IOHandler(std::string_view fileName) {
#ifdef _WIN32
    m_handle = CreateFile(
        string(fileName).c_str(),            // File name
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
    m_fd = open(string(fileName).c_str(), O_RDWR | O_CREAT, 0644);

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

blockid_t IOHandler::createNewBlock() {
#ifdef _WIN32
    LARGE_INTEGER newPos;
    newPos.QuadPart = ++m_blocks * cts::PG_SZ;
    if (!SetFilePointerEx(m_handle, newPos, nullptr, FILE_BEGIN))
        throw std::runtime_error("Failed to move file pointer");

    if (!SetEndOfFile(m_handle))
        throw std::runtime_error("Failed to truncate file");
#else
    blockid_t new_sz = ++m_blocks * cts::PG_SZ;
    if (ftruncate(m_fd, new_sz) == -1)
        throw std::runtime_error("Failed to increase file size");
#endif //_WIN32

    return m_blocks - 1;
}

void IOHandler::writeBlock(void *arr, blockid_t BlockNo) const {
    if (BlockNo >= m_blocks)
        throw std::runtime_error("BlockNo out of bounds");
#ifdef _WIN32
    LARGE_INTEGER fileOffset;
    fileOffset.QuadPart = static_cast<LONGLONG>(BlockNo) * cts::PG_SZ;
    SetFilePointerEx(m_handle, fileOffset, nullptr, FILE_BEGIN);

    DWORD written;
    if (!WriteFile(m_handle, arr, cts::PG_SZ, &written, nullptr)) {
        throw std::runtime_error("WriteFile failed, error code: " + std::to_string(GetLastError()));
    }

    //TODO: usually we should not flush after every write;
    // this is just for development/debugging purposes
    if (!FlushFileBuffers(m_handle))
        throw std::runtime_error("FlushFileBuffers failed, error code: " + std::to_string(GetLastError()));

#else

    if (pwrite(m_fd, arr, cts::PG_SZ, BlockNo * cts::PG_SZ) == -1)
        throw std::runtime_error("error while writing file");

    //TODO: usually we should not flush after every write;
    // this is just for development/debugging purposes
    if (fsync(m_fd) == -1)
        throw std::runtime_error("Error while fsyncing file");
#endif
}

void IOHandler::readBlock(void *arr, blockid_t BlockNo) const {
    if (BlockNo >= m_blocks)
        throw std::runtime_error("BlockNo out of bounds");
#ifdef _WIN32
    // Move file pointer manually
    LARGE_INTEGER fileOffset;
    fileOffset.QuadPart = static_cast<LONGLONG>(BlockNo) * cts::PG_SZ;
    if (!SetFilePointerEx(m_handle, fileOffset, nullptr, FILE_BEGIN))
        throw std::runtime_error("SetFilePointerEx failed, error code: " + std::to_string(GetLastError()));

    // Read file
    DWORD bytesRead;
    if (!ReadFile(m_handle, arr, cts::PG_SZ, &bytesRead, nullptr) || bytesRead != cts::PG_SZ)
        throw std::runtime_error("ReadFile failed, error code: " + std::to_string(GetLastError()));
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

} // namespace backend