//
// Created by Kylan Chen on 3/1/25.
//

#ifndef KNDB_KNDB_TYPES_HPP
#define KNDB_KNDB_TYPES_HPP

#include <string>
#include <vector>
#include <array>
#include <constants.hpp>
#include <variant>
#include <memory>
#include <windows.h> // for windows 'byte' -> unsigned char

namespace kndb {
    template<typename T>
    using Vec = std::vector<T>;

    template <typename T, size_t N>
    using Arr = std::array<T, N>;

    template<typename T>
    using PgArr = std::array<T, cts::PG_SZ>;

    template<typename T>
    using Ptr = std::unique_ptr<T>;

    template<typename T>
    using PgArrPtr = std::unique_ptr<PgArr<T>>;

    using Vari = std::variant<int, char, bool, float, double, std::string>;
    using string = std::string;

#ifdef _WIN32
    using byte = byte; // use windows byte typedef
#else
    using byte = std::byte;
#endif

    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    struct RowPos {
        u32 pageID;
        u32 cellID; // could potentially change to 16 bytes if we need extra space eventually
    };
//
//    struct SecIdxVal {
//        size_t indexes[5];
//        size_t numIndexes;
//        size_t pageID;
//    };

}

#endif //KNDB_KNDB_TYPES_HPP
