//
// Created by Kylan Chen on 10/13/24.
//

#ifndef KNDB_UTILITY_HPP
#define KNDB_UTILITY_HPP

#include <span>

#include "kndb_types.hpp"
#include "constants.hpp"
#include "assume.hpp"

namespace backend {

template<typename T>
inline size_t db_sizeof() { return sizeof(T); }

template<>
inline size_t db_sizeof<string>() { return cts::MAX_STR_SZ; }

template<typename T>
inline size_t db_sizeof(T &&) { return db_sizeof<std::decay_t<T> >(); }

inline size_t db_sizeof(Vari &val) {
    return std::visit([](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        return db_sizeof<T>();
    }, val);
}

inline size_t db_sizeof(const Vari &val) {
    return std::visit([](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        return db_sizeof<T>();
    }, val);
}

namespace variant_conversion_id {
enum {
    CHAR = 1, INT, BOOL, STRING, FLOAT, DOUBLE
};
}

inline Vari type_id_to_variant(const u8 type_id) {
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
            ASSUME_S(false, "type_id does not represent a valid type");
    }
}

inline u8 variant_to_type_id(const Vari &v) {
    u8 res = cts::U8_INVALID;

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

    if (res == cts::U8_INVALID) {
        throw std::runtime_error("Unsupported variant; cannot convert to type id");
    }

    return res;
}

template<typename T>
inline void db_deserialize(T &src, std::span<const byte> bytes, u16 &offset) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    memcpy(&src, bytes.data() + offset, db_sizeof<T>());
    offset += db_sizeof<T>();
}

inline void db_deserialize(string &src, std::span<const byte> bytes, u16 &offset) {
    char buf[db_sizeof<std::string>()];
    memcpy(buf, bytes.data() + offset, db_sizeof<std::string>());
    src = string(buf);
    offset += db_sizeof<std::string>();
}

inline void
db_deserialize(Vari &src, std::span<const byte> bytes, u16 &offset, const Vari &type) {
    std::visit([&bytes, &offset, &src](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        T buf;
        db_deserialize(buf, bytes, offset);
        src = buf;
    }, type);
}

template<typename T>
inline void db_serialize(const T &val, std::span<byte> bytes, u16 &offset) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    memcpy(bytes.data() + offset, &val, db_sizeof<T>());
    offset += db_sizeof<T>();
}

inline void db_serialize(const std::string &val, std::span<byte> bytes, u16 &offset) {
    memcpy(bytes.data() + offset, val.data(), db_sizeof<std::string>());
    offset += db_sizeof<std::string>();
}

inline void db_serialize(const Vari &val, std::span<byte> bytes, u16 &offset) {
    std::visit([&bytes, &offset](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        db_serialize(arg, bytes, offset);
    }, val);
}

template<typename KeyType>
inline u16 calculateDegree(const KeyType &key, const Vec<Vari> &values) {
    static constexpr u16 metadataBuffer = 100;
    constexpr u16 free_space = cts::PG_SZ - metadataBuffer;

    u16 cell_size = 0;
    cell_size += db_sizeof(key);
    for (const auto &value: values) {
        cell_size += db_sizeof(value);
    }
    const u16 page_ptr_size = db_sizeof<u32>();

    return (free_space + cell_size) / (2 * (cell_size + page_ptr_size));
}

inline bool sameTypes(const Vec<Vari> &vec1, const Vec<Vari> &vec2) {
    if (vec1.size() != vec2.size()) return false;
    for (int i = 0; i < vec1.size(); i++)
        if (variant_to_type_id(vec1[i]) != variant_to_type_id(vec2[i])) return false;
    return true;
}

} // namespace backend

#endif //KNDB_UTILITY_HPP
