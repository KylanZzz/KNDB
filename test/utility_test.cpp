//
// Created by Kylan Chen on 3/4/25.
//

#include <gtest/gtest.h>

#include "utility.hpp"

TEST(UtilityTest, sizeOfVariantsWorks) {
    variants s = int();
    ASSERT_EQ(db_sizeof(s), sizeof(int));
    s = double();
    ASSERT_EQ(db_sizeof(s), sizeof(double));
    s = string();
    ASSERT_EQ(db_sizeof(s), cts::STR_SZ);

    const variants c_s = int();
    ASSERT_EQ(db_sizeof(c_s), sizeof(int));

    const variants c_s2 = string();
    ASSERT_EQ(db_sizeof(c_s2), cts::STR_SZ);
}

TEST(UtilityTest, serializationOfVariantsWork) {
    size_t offset = 0;
    ByteVec vec(1000);
    vector<variants> list = {3, "Kylan", "Apple", double(4.1323), 'a', float(3.14159)};
    for (const auto& item: list) {
        deserialize(item, vec, offset);
    }

    offset = 0;
    vector<variants> types = {int(), string(), string(), double() , char(), float()};
    vector<variants> res;
    for (const auto& type: types) {
        variants temp;
        serialize(temp, vec, offset, type);
        res.push_back(temp);
    }

    ASSERT_EQ(res, list);
}