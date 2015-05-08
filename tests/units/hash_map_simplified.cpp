/*
 * hash_map_simplified.cpp
 *
 *  Created on: May 8, 2015
 *      Author: masha
 */
#include <gtest/gtest.h>

#include <hash_map.hpp>
#include <utils/my-int-wrapper.hpp>
#include "uniform_hash_map_test.hpp"

typedef int key_type;
typedef int mapped_type;



template<typename Key, typename Value>
struct make_simplified_map_uniform_tests
{
    typedef unform_hash_map_tester<Key, Value, lfds::memory_model::simplified,
            map_type::IntegralPair> type;
};


#define MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, testFunc) \
        TEST(suite, testFunc) \
        { test_maker::type::test##testFunc(); }

#define MAKE_ALL_TESTS_FOR_SIMPLIFIED_MAP(test_maker, suite) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, TypeTraits) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, Empty) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, Insert) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, InsertOrUpdate) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, Find) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, Erase) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, Collision) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, ReuseKey) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, Rehash) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, Random) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, DataTypes) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, SnapshotEmpty) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, Snapshot) \
        MAKE_SIMPLIFIED_MAP_UNIT_TEST(test_maker, suite, Multithread)


typedef make_simplified_map_uniform_tests<key_type, mapped_type> test_maker_type;

MAKE_ALL_TESTS_FOR_SIMPLIFIED_MAP(test_maker_type, SimplifiedHashMap)
