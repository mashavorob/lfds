/*
 * hash_map_mt.cpp
 *
 *  Created on: Apr 16, 2015
 *      Author: masha
 */

#include <gtest/gtest.h>

#include "map_insert_erase.hpp"

#include <utils/my-int-wrapper.hpp>
#include <hash_map.hpp>

static const int MapSize = static_cast<int>(1e5);
static const int NumRepetitions = MapSize*10;

TEST(HashMap_Generic, multithread)
{
    typedef lfds::my::int_wrapper<int> key_type;
    typedef lfds::my::int_wrapper<int> value_type;
    typedef lfds::my::int_wrapper_hash<key_type::type> hash_type;

    typedef lfds::hash_map<key_type, value_type, hash_type> map_type;

    typedef lfds::testing::map_insert_erase<map_type, MapSize, NumRepetitions> test_type;


    map_type map;
    test_type test(map);
    test.run();

    static map_type::size_type expected = 0;

    EXPECT_FALSE(map_type::INTEGRAL_KEY);
    EXPECT_FALSE(map_type::INTEGRAL_KEYVALUE);

    EXPECT_EQ(expected, test.getFailsOnInsert());
    EXPECT_EQ(expected, test.getFailsOnFind());
    EXPECT_EQ(expected, test.getFailsOnMissing());
}

TEST(HashMap_IntegralKey, multithread)
{
    typedef long long key_type;
    typedef long long value_type;

    typedef lfds::hash_map<key_type, value_type> map_type;
    typedef lfds::testing::map_insert_erase<map_type, MapSize, NumRepetitions> test_type;

    map_type map;
    test_type test(map);
    test.run();

    static map_type::size_type expected = 0;

    EXPECT_TRUE(map_type::INTEGRAL_KEY);
    EXPECT_FALSE(map_type::INTEGRAL_KEYVALUE);

    EXPECT_EQ(expected, test.getFailsOnInsert());
    EXPECT_EQ(expected, test.getFailsOnFind());
    EXPECT_EQ(expected, test.getFailsOnMissing());
}

TEST(HashMap_IntegralPair, multithread)
{
    typedef int key_type;
    typedef int value_type;

    typedef lfds::hash_map<key_type, value_type> map_type;
    typedef lfds::testing::map_insert_erase<map_type, MapSize, NumRepetitions> test_type;

    map_type map;
    test_type test(map);

    test.run();

    static map_type::size_type expected = 0;

    EXPECT_TRUE(map_type::INTEGRAL_KEY);
    EXPECT_TRUE(map_type::INTEGRAL_KEYVALUE);

    EXPECT_EQ(expected, test.getFailsOnInsert());
    EXPECT_EQ(expected, test.getFailsOnFind());
    EXPECT_EQ(expected, test.getFailsOnMissing());
}
