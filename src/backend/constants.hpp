//
// Created by Kylan Chen on 3/1/25.
//

#ifndef KNDB_CONSTANTS_HPP
#define KNDB_CONSTANTS_HPP

#include <cstdio>

namespace cts { // constants
    constexpr size_t PG_SZ = 4096;
    constexpr size_t CACHE_SZ = 10;
    constexpr size_t STR_SZ = 32;
    constexpr size_t MAX_BLOCKS = 50000;

    constexpr size_t FSM_PAGE_NO = 0;
    constexpr size_t SCHEMA_PAGE_NO = 1;
}

#endif //KNDB_CONSTANTS_HPP
