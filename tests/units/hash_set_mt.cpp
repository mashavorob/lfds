/*
 * hash_set_mt.cpp
 *
 *  Created on: Apr 17, 2015
 *      Author: masha
 */

#include <gtest/gtest.h>

#include "set_insert_erase.hpp"

#include <utils/my-int-wrapper.hpp>
#include <hash_set.hpp>


static const int MapSize = static_cast<int>(1e5);
static const int NumRepetitions = MapSize*10;

TEST(HashSet_Generic, multithread)
{
    typedef lfds::my::int_wrapper<int> key_type;
    typedef lfds::my::int_wrapper_hash<key_type::type> hash_type;

    typedef lfds::hash_set<key_type, hash_type> set_type;

    typedef lfds::testing::set_insert_erase<set_type, MapSize, NumRepetitions> test_type;


    set_type map;
    test_type test(map);
    test.run();

    static set_type::size_type expected = 0;

    EXPECT_FALSE(set_type::INTEGRAL);

    EXPECT_EQ(expected, test.getFailsOnInsert());
    EXPECT_EQ(expected, test.getFailsOnFind());
    EXPECT_EQ(expected, test.getFailsOnMissing());
}

TEST(HashSet_Integral, multithread)
{
    typedef long long key_type;

    typedef lfds::hash_set<key_type> set_type;
    typedef lfds::testing::set_insert_erase<set_type, MapSize, NumRepetitions> test_type;

    set_type map;
    test_type test(map);
    test.run();

    static set_type::size_type expected = 0;

    EXPECT_TRUE(set_type::INTEGRAL);

    EXPECT_EQ(expected, test.getFailsOnInsert());
    EXPECT_EQ(expected, test.getFailsOnFind());
    EXPECT_EQ(expected, test.getFailsOnMissing());
}
