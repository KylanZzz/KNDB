//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_UTILITY_HPP
#define KNDB_UTILITY_HPP

#include "SchemaPage.hpp"
#include "FSMPage.hpp"
#include "kndb_types.hpp"
#include "constants.hpp"
#include "TablePage.hpp"

using namespace kndb_types;

template<typename T>
inline size_t db_sizeof() { return sizeof(T); }

template<>
inline size_t db_sizeof<std::string>() { return cts::STR_SZ; }

template<typename T>
inline size_t db_sizeof(T &) { return db_sizeof<T>(); }

inline size_t db_sizeof(vector<variants> &vec) {
    size_t res = 0;
    for (const auto &var: vec) {
        std::visit([&res](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            res += db_sizeof<T>();
        }, var);
    }
    return res;
}

namespace variant_conversion_id {
    enum {
        CHAR, INT, BOOL, STRING, FLOAT, DOUBLE
    };
}

namespace page_type_conversion_id {
    enum {
        SCHEMA_PAGE, FSM_PAGE, TABLE_PAGE
    };
}

inline variants type_id_to_variant(size_t type_id) {
    switch (type_id) {
        case variant_conversion_id::CHAR:
            return char();
        case variant_conversion_id::INT:
            return int();
        case variant_conversion_id::BOOL:
            return bool();
        case variant_conversion_id::STRING:
            return std::string();
        case variant_conversion_id::FLOAT:
            return float();
        case variant_conversion_id::DOUBLE:
            return double();
        default:
            throw std::runtime_error("Invalid type id");
    }
}

inline size_t variant_to_type_id(variants v) {
    size_t res = std::numeric_limits<size_t>::max();

    std::visit([&res](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, char>) {
            res = variant_conversion_id::CHAR;
        } else if constexpr (std::is_same_v<T, int>) {
            res = variant_conversion_id::INT;
        } else if constexpr (std::is_same_v<T, bool>) {
            res = variant_conversion_id::BOOL;
        } else if constexpr (std::is_same_v<T, std::string>) {
            res = variant_conversion_id::STRING;
        } else if constexpr (std::is_same_v<T, double>) {
            res = variant_conversion_id::DOUBLE;
        } else if constexpr (std::is_same_v<T, float>) {
            res = variant_conversion_id::FLOAT;
        }
    }, v);

    if (res == std::numeric_limits<size_t>::max()) {
        throw std::runtime_error("Unsupported variant; cannot convert to type id");
    }

    return res;
}

template<typename T>
inline size_t get_page_type_id() {
    throw std::runtime_error("Unsupported page type id");
}

template<>
inline size_t get_page_type_id<SchemaPage>() {
    return page_type_conversion_id::SCHEMA_PAGE;
}

template<>
inline size_t get_page_type_id<FSMPage>() {
    return page_type_conversion_id::FSM_PAGE;
}

template<>
inline size_t get_page_type_id<TablePage>() {
    return page_type_conversion_id::TABLE_PAGE;
}

#endif //KNDB_UTILITY_HPP
