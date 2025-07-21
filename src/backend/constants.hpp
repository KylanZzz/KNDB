//
// Created by Kylan Chen on 3/1/25.
//

#ifndef KNDB_CONSTANTS_HPP
#define KNDB_CONSTANTS_HPP

#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>

namespace backend::cts { // constants

// configuration
constexpr std::string_view DATABASE_NAME{"kylan.db"};
constexpr uint8_t MAX_STR_SZ = 128;
constexpr uint8_t MAX_STR_LEN = MAX_STR_SZ - 1;
constexpr uint8_t MAX_TABLES = 100;
constexpr uint16_t PG_SZ = 4096; // 4kb pg size
constexpr uint32_t CACHE_SZ = 100000; // ≈ 400 mb cache
constexpr uint32_t MAX_FSMPAGES = 2; // ≈ 134 mb * 10 = 1.34 gb
constexpr uint8_t SCHEMA_ID = 1;

// page type id
namespace pg_type_id {
enum {
    SCHEMA_PAGE = 1, FSM_PAGE, TABLE_PAGE, BTREE_NODE_PAGE
};
}

constexpr size_t SIZE_T_INVALID = std::numeric_limits<size_t>::max();
constexpr uint8_t U8_INVALID = std::numeric_limits<uint8_t>::max();
constexpr uint16_t U16_INVALID = std::numeric_limits<uint16_t>::max();
constexpr uint32_t U32_INVALID = std::numeric_limits<uint32_t>::max();
constexpr uint64_t U64_INVALID = std::numeric_limits<uint64_t>::max();
constexpr uint32_t PGID_INVALID = std::numeric_limits<uint32_t>::max();
constexpr uint32_t CELLID_INVALID = std::numeric_limits<uint32_t>::max();
constexpr uint32_t CHILDID_INVALID = std::numeric_limits<uint32_t>::max();
constexpr uint32_t PGTYPEID_INVALID = std::numeric_limits<uint8_t>::max();

} // namespace constants

#endif //KNDB_CONSTANTS_HPP
