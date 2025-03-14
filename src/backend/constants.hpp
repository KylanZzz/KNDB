//
// Created by Kylan Chen on 3/1/25.
//

#ifndef KNDB_CONSTANTS_HPP
#define KNDB_CONSTANTS_HPP

#include <cstdio>

namespace cts { // constants
    // configuration
    const std::string DATABASE_NAME = "kylan.db";
    constexpr size_t PG_SZ = 4096;
    constexpr size_t CACHE_SZ = 100;
    constexpr size_t STR_SZ = 32;
    constexpr size_t MAX_BLOCKS = 100000;

    // DB page numbers
    namespace pgid {
        constexpr size_t FSM_ID = 0;
        constexpr size_t SCHEMA_ID = 1;
    }

    // page type id
    namespace pg_type_id {
        enum {
            SCHEMA_PAGE = 1, FSM_PAGE, TABLE_PAGE, BTREE_NODE_PAGE
        };
    }

    // other
    constexpr size_t SIZE_T_OUT_OF_BOUNDS = std::numeric_limits<size_t>::max();
}

#endif //KNDB_CONSTANTS_HPP
