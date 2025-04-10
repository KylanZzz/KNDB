//
// Created by Kylan Chen on 3/4/25.
//

#include <gtest/gtest.h>

#include "utility.hpp"

using namespace backend;

TEST(UtilityTest, sizeOfVariantsWorks) {
    Vari s = int();
    ASSERT_EQ(db_sizeof(s), sizeof(int));
    s = double();
    ASSERT_EQ(db_sizeof(s), sizeof(double));
    s = string();
    ASSERT_EQ(db_sizeof(s), cts::MAX_STR_SZ);

    const Vari c_s = int();
    ASSERT_EQ(db_sizeof(c_s), sizeof(int));

    const Vari c_s2 = string();
    ASSERT_EQ(db_sizeof(c_s2), cts::MAX_STR_SZ);
}

TEST(UtilityTest, serializationOfVariantsWork) {
    u16 offset = 0;
    Vec<byte> vec(1000);
    Vec<Vari> list = {3, "Kylan", "Apple", double(4.1323), 'a', float(3.14159)};
    for (const auto& item: list) {
        db_serialize (item, vec, offset);
    }

    offset = 0;
    Vec<Vari> types = {int(), string(), string(), double() , char(), float()};
    Vec<Vari> res;
    for (const auto& type: types) {
        Vari temp;
        db_deserialize(temp, vec, offset, type);
        res.push_back(temp);
    }

    ASSERT_EQ(res, list);
}