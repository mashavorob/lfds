/*
 * uniform_hash_set_test.hpp
 *
 *  Created on: Apr 28, 2015
 *      Author: masha
 */

#ifndef TESTS_UNITS_UNIFORM_HASH_SET_TEST_HPP_
#define TESTS_UNITS_UNIFORM_HASH_SET_TEST_HPP_

#include <gtest/gtest.h>

#include "set_insert_erase.hpp"

#include <utils/my-int-wrapper.hpp>
#include <xtomic/hash_set.hpp>

#include <ctime>
#include <cstdlib>

template<typename Key, typename Hash, lfds::memory_model::type model>
struct make_hash_set_table;

template<typename Key, typename Hash>
struct make_hash_set_table<Key, Hash, lfds::memory_model::wise>
{
    typedef typename lfds::make_wise_hash_set<Key, Hash>::type type;
};

template<typename Key, typename Hash>
struct make_hash_set_table<Key, Hash, lfds::memory_model::greedy>
{
    typedef typename lfds::make_greedy_hash_set<Key, Hash>::type type;
};

template<typename Key, lfds::memory_model::type model, bool IsIntegral>
struct unform_hash_set_tester
{
    static constexpr int MapSize = static_cast<int>(1e5);
    static constexpr int NumRepetitions = MapSize * 10;

    typedef Key key_type;
    typedef typename lfds::my::make_hash<Key>::type hash_type;
    typedef typename make_hash_set_table<key_type, hash_type, model>::type set_type;
    typedef typename set_type::size_type size_type;
    typedef typename set_type::snapshot_type snapshot_type;
    typedef lfds::testing::set_insert_erase<set_type, MapSize, NumRepetitions> mt_test_type;

    static constexpr bool INTEGRAL = set_type::INTEGRAL;
    static constexpr lfds::memory_model::type MEMORY_MODEL =
            set_type::MEMORY_MODEL;

    static void testTypeTraits()
    {

#define check_flag(flag, constant) \
    do { \
    if ( flag ) EXPECT_TRUE(constant); \
    else  EXPECT_FALSE(constant); \
    } while ( false )

        check_flag(IsIntegral, INTEGRAL);

#undef check_flag

        const lfds::memory_model::type memoryModel = MEMORY_MODEL;
        EXPECT_EQ(model, memoryModel);
    }

    static void testEmpty()
    {
        set_type hm;

        bool res = false;
        size_type size = 0;

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(0));

        res = hm.find(0);
        EXPECT_FALSE(res);

        res = hm.erase(0);
        EXPECT_FALSE(res);
    }

    static void testInsert()
    {
        set_type hm;

        bool res = false;
        size_type size = 0;

        res = hm.insert(0);
        EXPECT_TRUE(res);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(1));
    }

    static void testFind()
    {
        set_type hm;

        bool res = false;
        size_type size = 0;

        res = hm.find(1);
        EXPECT_FALSE(res);

        hm.insert(1);
        res = hm.find(1);
        EXPECT_TRUE(res);

        res = hm.find(123);
        EXPECT_FALSE(res);

        res = hm.find(-1);
        EXPECT_FALSE(res);

        res = hm.find(0);
        EXPECT_FALSE(res);
    }

    static void testErase()
    {
        set_type hm;

        bool res = false;
        size_type size = 0;

        res = hm.erase(1);
        EXPECT_FALSE(res);

        hm.insert(1);

        res = hm.erase(2);
        EXPECT_FALSE(res);

        res = hm.erase(-1);
        EXPECT_FALSE(res);

        res = hm.erase(0);
        EXPECT_FALSE(res);

        res = hm.find(1);
        EXPECT_TRUE(res);

        res = hm.find(-1);
        EXPECT_FALSE(res);

        res = hm.erase(1);
        EXPECT_TRUE(res);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(0));

        res = hm.erase(2);
        EXPECT_FALSE(res);

        res = hm.erase(1);
        EXPECT_FALSE(res);

        res = hm.erase(1);
        EXPECT_FALSE(res);
    }

    static void testCollision()
    {
        set_type hm;

        bool res = false;
        size_type size = 0;

        hm.insert(1);
        hm.insert(2);
        hm.insert(3);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(3));

        res = hm.find(1);
        EXPECT_TRUE(res);

        res = hm.find(2);
        EXPECT_TRUE(res);

        res = hm.find(3);
        EXPECT_TRUE(res);

        res = hm.find(0);
        EXPECT_FALSE(res);

        res = hm.erase(2);
        EXPECT_TRUE(res);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(2));

        res = hm.find(1);
        EXPECT_TRUE(res);

        res = hm.find(2);
        EXPECT_FALSE(res);

        res = hm.find(3);
        EXPECT_TRUE(res);

        res = hm.find(0);
        EXPECT_FALSE(res);
    }

    static void testReuseKey()
    {
        set_type hm;

        bool res = false;
        size_type size = 0;

        hm.insert(1);
        hm.insert(2);
        hm.insert(3);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(3));

        res = hm.find(1);
        EXPECT_TRUE(res);

        res = hm.find(2);
        EXPECT_TRUE(res);

        res = hm.find(3);
        EXPECT_TRUE(res);

        res = hm.find(0);
        EXPECT_FALSE(res);

        res = hm.erase(2);
        EXPECT_TRUE(res);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(2));

        res = hm.find(1);
        EXPECT_TRUE(res);

        res = hm.find(2);
        EXPECT_FALSE(res);

        res = hm.find(3);
        EXPECT_TRUE(res);

        res = hm.find(0);
        EXPECT_FALSE(res);

        hm.insert(2);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(3));

        res = hm.find(1);
        EXPECT_TRUE(res);

        res = hm.find(2);
        EXPECT_TRUE(res);

        res = hm.find(3);
        EXPECT_TRUE(res);

        res = hm.find(0);
        EXPECT_FALSE(res);
    }

    static void testRehash()
    {
        set_type hm;

        size_type size = 0;
        size_type count = 0;
        bool result = false;

        size = hm.getCapacity() * 2 + 1;

        for (int i = 1; i <= size; ++i)
        {
            result = hm.insert(i);
            EXPECT_TRUE(result);
            result = hm.find(i);
            EXPECT_TRUE(result);
        }

        count = hm.getCapacity();

        EXPECT_GT(count, size);
        count = hm.size();
        EXPECT_EQ(count, size);

        for (int i = 1; i <= size; ++i)
        {
            result = hm.find(i);
            EXPECT_TRUE(result);
            result = hm.find(-i);
            EXPECT_FALSE(result);
        }
    }

    static void testRandom()
    {
        srand(time(nullptr));

        set_type hm;

        static const int maxval = 10000;
        std::size_t count_inserted = 0;
        for (int i = 0; i < 1000; ++i)
        {
            int k = rand() % maxval;
            if (hm.insert(k))
            {
                ++count_inserted;
            }
        }

        std::size_t count_found = 0;
        for (int i = 0; i <= maxval; ++i)
        {
            key_type k = i;
            if (hm.find(k))
            {
                ++count_found;
            }
        }
        EXPECT_EQ(count_inserted, count_found);
    }

    // just unsure that it is compilable
    template<typename InnerKey>
    struct compile_tester
    {
        typedef typename lfds::my::make_value_type<key_type, InnerKey>::type sized_key_type;
        typedef typename lfds::my::make_hash<sized_key_type>::type inner_hash_type;

        typedef std::equal_to<sized_key_type> equal_func_type;
        typedef std::allocator<key_type> allocator_type;

        typedef lfds::hash_set<sized_key_type, inner_hash_type, equal_func_type,
                allocator_type, set_type::MEMORY_MODEL> inner_hash_set_type;
        typedef typename inner_hash_set_type::size_type size_type;

        void operator()() const
        {
            // components
            bool expected = IsIntegral;
            bool res = lfds::is_integral<sized_key_type>::value;
            EXPECT_EQ(expected, res);

            inner_hash_set_type hm;

            size_type size = 0;

            hm.insert(1);
            hm.find(1);
            hm.erase(1);
            hm.size();

            const bool isIntegral = inner_hash_set_type::INTEGRAL;
            const lfds::memory_model::type memModel =
                    inner_hash_set_type::MEMORY_MODEL;
            const lfds::memory_model::type expectedModel = MEMORY_MODEL;

            EXPECT_EQ(expected, isIntegral);
            //EXPECT_EQ(model, memModel);
        }
    };

    static void testDataTypes()
    {
        compile_tester<int8_t>()();
        compile_tester<int16_t>()();
        compile_tester<int32_t>()();
        compile_tester<int64_t>()();
    }

    static void testSnapshotEmpty()
    {
        snapshot_type snapshot;
        snapshot.push_back(1);

        set_type hm;

        hm.getSnapshot(snapshot);

        EXPECT_EQ(snapshot.size(), 0);

    }

    static void testSnapshot()
    {
        snapshot_type snapshot;

        set_type hm;

        hm.insert(1);

        hm.getSnapshot(snapshot);

        EXPECT_EQ(snapshot.size(), 1);

        EXPECT_EQ(snapshot.front(), static_cast<key_type>(1));
    }
    static void testMultithread()
    {
        set_type map;
        mt_test_type test(map);

        test.run();

        size_type expected = 0;

        EXPECT_EQ(expected, test.getFailsOnInsert());
        EXPECT_EQ(expected, test.getFailsOnFind());
        EXPECT_EQ(expected, test.getFailsOnMissing());
        EXPECT_EQ(expected, test.getFailsOnErase());
    }
};

template<typename Key, bool integralKey = lfds::is_integral<Key>::value >
struct make_set_uniform_tests
{
    typedef unform_hash_set_tester<Key, lfds::memory_model::wise, integralKey> wise_test_type;
    typedef unform_hash_set_tester<Key, lfds::memory_model::greedy, integralKey> greedy_test_type;
};

#define MAKE_SET_UNIT_TEST(test_maker, suite, testFunc) \
        TEST(GreedyHashSet_##suite, testFunc) \
        { test_maker::greedy_test_type::test##testFunc(); } \
        TEST(WiseHashSet_##suite, testFunc) \
        { test_maker::wise_test_type::test##testFunc(); }

#define MAKE_ALL_TESTS_FOR_SET(key_type, suite) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, TypeTraits) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, Empty) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, Insert) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, Find) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, Erase) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, Collision) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, ReuseKey) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, Rehash) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, Random) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, DataTypes) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, SnapshotEmpty) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, Snapshot) \
        MAKE_SET_UNIT_TEST(make_set_uniform_tests<key_type>, suite, Multithread)

#endif /* TESTS_UNITS_UNIFORM_HASH_SET_TEST_HPP_ */
