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

namespace kndb_types {
    template<typename T>
    using Vec = std::vector<T>;

    template <typename T, size_t N>
    using Arr = std::array<T, N>;

    template<typename T>
    using PgArr = std::array<T, cts::PG_SZ>;

    template<typename T>
    using VecPtr = std::unique_ptr<std::vector<T>>;

    template<typename T>
    using PrArrPtr = std::unique_ptr<PgArr<T>>;

    using Vari = std::variant<int, char, bool, float, double, std::string>;
    using String = std::string;
    using Byte = std::byte;

    struct RowPtr {
        size_t pageID;
        size_t cellID;
    };
//
//    struct SecIdxVal {
//        size_t indexes[5];
//        size_t numIndexes;
//        size_t pageID;
//    };

}

#endif //KNDB_KNDB_TYPES_HPP
