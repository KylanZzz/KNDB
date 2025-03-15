//
// Created by Kylan Chen on 3/1/25.
//

#ifndef KNDB_KNDB_TYPES_HPP
#define KNDB_KNDB_TYPES_HPP

#include <string>
#include <vector>
#include <variant>
#include <memory>

namespace kndb_types {
    using variants = std::variant<int, char, bool, float, double, std::string>;
    using std::string;
    using std::vector;
    using ByteVec = std::vector<std::byte>;
    using ByteVecPtr = std::unique_ptr<std::vector<std::byte>>;

    struct RowPtr {
        size_t pageID;
        size_t cellID;

        bool operator==(const RowPtr& other) const {
            return other.pageID == this->pageID && other.cellID == this->cellID;
        }
    };
//
//    struct SecIdxVal {
//        size_t indexes[5];
//        size_t numIndexes;
//        size_t pageID;
//    };

}

#endif //KNDB_KNDB_TYPES_HPP
