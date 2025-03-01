//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_UTILITY_HPP
#define KNDB_UTILITY_HPP

#include "SchemaPage.hpp"
#include "FSMPage.hpp"

using variants = std::variant<int, char, bool, std::string>;
using std::byte;
using std::vector;
using ByteVec = std::vector<std::byte>;
using ByteVecPtr = std::unique_ptr<ByteVec>;
using std::string;

namespace cts { // constants
    constexpr size_t PG_SZ = 4096;
    constexpr size_t CACHE_SZ = 10;
    constexpr size_t STR_SZ = 32;

    constexpr size_t FSM_PAGE_NO = 0;
    constexpr size_t SCHEMA_PAGE_NO = 1;
}

template <typename T>
inline size_t db_sizeof() { return sizeof(T); }

template <>
inline size_t db_sizeof<std::string>() {
    return cts::STR_SZ;
}

inline variants type_id_to_variant(size_t type_id) {
    switch (type_id){
        case 1:
            return char();
        case 2:
            return int();
        case 3:
            return bool();
        case 4:
            return std::string();
        default:
            throw std::runtime_error("Invalid type id");
    }
}

inline size_t variant_to_type_id(variants v) {
    size_t res = -1;

    std::visit([&res](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, char>){
            res = 1;
        } else if constexpr (std::is_same_v<T, int>) {
            res = 2;
        } else if constexpr (std::is_same_v<T, bool>) {
            res = 3;
        } else if constexpr (std::is_same_v<T, std::string>) {
            res = 4;
        }
    }, v);

    if (res == -1) {
        throw std::runtime_error("Unsupported variant; cannot convert to type id");
    }

    return res;
}

template <typename T>
inline size_t get_page_type_id() {
    throw std::runtime_error("Unsupported page type id");
}

template <>
inline size_t get_page_type_id<SchemaPage>() {
    return 1;
}

template <>
inline size_t get_page_type_id<FSMPage>() {
    return 2;
}


#endif //KNDB_UTILITY_HPP
