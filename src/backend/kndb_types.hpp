//
// Created by Kylan Chen on 3/1/25.
//

#ifndef KNDB_KNDB_TYPES_HPP
#define KNDB_KNDB_TYPES_HPP

#include <string>
#include <vector>
#include <array>
#include <variant>
#include <memory>

#include "constants.hpp"

#ifdef _WIN32
#include <windows.h> // for windows 'byte' -> unsigned char
#endif // _WIN32

namespace backend {

template<typename T>
using Vec = std::vector<T>;

template<typename T, size_t N>
using Arr = std::array<T, N>;

template<typename T>
using PgArr = std::array<T, cts::PG_SZ>;

template<typename T>
using Ptr = std::unique_ptr<T>;

using Vari = std::variant<int, char, bool, float, double, std::string>;
using string = std::string;

#ifdef _WIN32
using byte = byte; // use windows byte typedef
#else
using byte = std::byte;
#endif // _WIN32

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using pgid_t = u32;
using cellid_t = u32;
using childid_t = u32;
using offset_t = u16;
using degree_t = u16;
using pgtypeid_t = u8;
using typeid_t = u8;
using bitmapidx_t = u32;
using blockid_t = u32;

struct RowPos {
    pgid_t pageID;
    cellid_t cellID;
};

//    struct SecIdxVal {
//        size_t indexes[5];
//        size_t numIndexes;
//        size_t pageID;
//    };

} // namespace backend

#endif //KNDB_KNDB_TYPES_HPP
