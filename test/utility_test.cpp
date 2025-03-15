//
// Created by Kylan Chen on 3/4/25.
//

#include <gtest/gtest.h>

#include "utility.hpp"

TEST(UtilityTest, sizeOfVariantsWorks) {
    Vari s = int();
    ASSERT_EQ(db_sizeof(s), sizeof(int));
    s = double();
    ASSERT_EQ(db_sizeof(s), sizeof(double));
    s = String();
    ASSERT_EQ(db_sizeof(s), cts::STR_SZ);

    const Vari c_s = int();
    ASSERT_EQ(db_sizeof(c_s), sizeof(int));

    const Vari c_s2 = String();
    ASSERT_EQ(db_sizeof(c_s2), cts::STR_SZ);
}

TEST(UtilityTest, serializationOfVariantsWork) {
    size_t offset = 0;
    Vec<Byte> vec(1000);
    Vec<Vari> list = {3, "Kylan", "Apple", double(4.1323), 'a', float(3.14159)};
    for (const auto& item: list) {
        serialize(item, vec, offset);
    }

    offset = 0;
    Vec<Vari> types = {int(), String(), String(), double() , char(), float()};
    Vec<Vari> res;
    for (const auto& type: types) {
        Vari temp;
        deserialize(temp, vec, offset, type);
        res.push_back(temp);
    }

    ASSERT_EQ(res, list);
}