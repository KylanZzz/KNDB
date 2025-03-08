//
// Created by Kylan Chen on 3/1/25.
//

#ifndef KNDB_KNDB_TYPES_HPP
#define KNDB_KNDB_TYPES_HPP

#include <string>
#include <vector>

namespace kndb_types {
    struct RowPtr {
        size_t pageID;
        size_t cellID;

        bool operator==(const RowPtr& other) {
            return other.pageID == this->pageID && other.cellID == this->cellID;
        }
    };
    using variants = std::variant<int, char, bool, float, double, std::string>;
    using std::string;
    using std::byte;
    using std::vector;
    using ByteVec = std::vector<std::byte>;
    using ByteVecPtr = std::unique_ptr<std::vector<std::byte>>;
}

#endif //KNDB_KNDB_TYPES_HPP
