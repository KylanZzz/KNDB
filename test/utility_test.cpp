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
}

TEST(UtilityTest, sizeOfVariantsNonemptyVectorWorks) {
    vector<variants> vec = {int(), int(), double(), string(), string(), float()};
    size_t expected = sizeof(int) + sizeof(int) + sizeof(double) + sizeof(float) + cts::STR_SZ +
            cts::STR_SZ;
    ASSERT_EQ(db_sizeof(vec), expected);
}

TEST(UtilityTest, sizeOfVariantsEmptyVectorWorks) {
    vector<variants> vec = {};
    ASSERT_EQ(db_sizeof(vec), 0);
}

TEST(UtilityTest, sizeOfVariantsSingleElementVectorWorks) {
    vector<variants> vec = {int()};
    ASSERT_EQ(db_sizeof(vec), sizeof(int));
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