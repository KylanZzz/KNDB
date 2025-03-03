//
// Created by Kylan Chen on 3/1/25.
//

#ifndef KNDB_KNDB_TYPES_HPP
#define KNDB_KNDB_TYPES_HPP

#include <string>

namespace kndb_types {
    using variants = std::variant<int, char, bool, float, double, std::string>;
    using std::string;
    using std::byte;
    using std::vector;
    using ByteVec = std::vector<std::byte>;
    using ByteVecPtr = std::unique_ptr<std::vector<std::byte>>;
}

#endif //KNDB_KNDB_TYPES_HPP
