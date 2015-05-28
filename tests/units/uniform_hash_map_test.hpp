/*
 * universal_hash_map_test.hpp
 *
 *  Created on: Apr 28, 2015
 *      Author: masha
 */

#ifndef TESTS_UNITS_UNIFORM_HASH_MAP_TEST_HPP_
#define TESTS_UNITS_UNIFORM_HASH_MAP_TEST_HPP_

#include <gtest/gtest.h>

#include "map_insert_erase.hpp"

#include <utils/my-int-wrapper.hpp>
#include <xtomic/hash_map.hpp>

#include <ctime>
#include <cstdlib>

template<typename Key, typename Value, typename Hash,
        xtomic::memory_model::type model>
struct make_hash_table;

template<typename Key, typename Value, typename Hash>
struct make_hash_table<Key, Value, Hash, xtomic::memory_model::wise>
{
    typedef typename xtomic::make_wise_hash_map<Key, Value, Hash>::type type;
};

template<typename Key, typename Value, typename Hash>
struct make_hash_table<Key, Value, Hash, xtomic::memory_model::greedy>
{
    typedef typename xtomic::make_greedy_hash_map<Key, Value, Hash>::type type;
};

struct map_type
{
    enum type
    {
        Generic, IntegralKey, IntegralValue, IntegralPair,
    };
};
template<map_type::type MapType>
struct map_traits;

template<>
struct map_traits<map_type::Generic>
{
    static constexpr bool INTEGRAL_KEY = false;
    static constexpr bool INTEGRAL_VALUE = false;
    static constexpr bool INTEGRAL_KEYVALUE = false;
};

template<>
struct map_traits<map_type::IntegralKey>
{
    static constexpr bool INTEGRAL_KEY = true;
    static constexpr bool INTEGRAL_VALUE = false;
    static constexpr bool INTEGRAL_KEYVALUE = false;
};

template<>
struct map_traits<map_type::IntegralValue>
{
    static constexpr bool INTEGRAL_KEY = false;
    static constexpr bool INTEGRAL_VALUE = true;
    static constexpr bool INTEGRAL_KEYVALUE = false;
};

template<>
struct map_traits<map_type::IntegralPair>
{
    static constexpr bool INTEGRAL_KEY = true;
    static constexpr bool INTEGRAL_VALUE = true;
    static constexpr bool INTEGRAL_KEYVALUE = true;
};

template<typename Key, typename Value, xtomic::memory_model::type MemoryModel,
        map_type::type MapType>
struct unform_hash_map_tester
{
    static constexpr int MapSize = static_cast<int>(1e5);
    static constexpr int NumRepetitions = MapSize * 10;

    typedef Key key_type;
    typedef Value mapped_type;
    typedef typename xtomic::my::make_hash<Key>::type hash_type;
    typedef typename make_hash_table<key_type, mapped_type, hash_type,
            MemoryModel>::type map_type;
    typedef typename map_type::size_type size_type;
    typedef typename map_type::snapshot_type snapshot_type;
    typedef xtomic::testing::map_insert_erase<map_type, MapSize, NumRepetitions> mt_test_type;

    typedef map_traits<MapType> expected_map_traits_type;

    static constexpr bool INTEGRAL_KEY = map_type::INTEGRAL_KEY;
    static constexpr bool INTEGRAL_VALUE = map_type::INTEGRAL_VALUE;
    static constexpr bool INTEGRAL_KEYVALUE = map_type::INTEGRAL_KEYVALUE;
    static constexpr xtomic::memory_model::type MEMORY_MODEL =
            map_type::MEMORY_MODEL;

    static void testTypeTraits()
    {

#define check_traits_flag(constant) \
    do { \
    if ( expected_map_traits_type::constant ) EXPECT_TRUE(constant); \
    else  EXPECT_FALSE(constant); \
    } while ( false )

        check_traits_flag(INTEGRAL_KEY);
        check_traits_flag(INTEGRAL_VALUE);
        check_traits_flag(INTEGRAL_KEYVALUE);

#undef check_traits_flag

        const xtomic::memory_model::type memoryModel = MEMORY_MODEL;
        EXPECT_EQ(memoryModel, MemoryModel);
    }

    static void testEmpty()
    {
        map_type hm;

        bool res = false;
        size_type size = 0;

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(0));

        mapped_type val;
        res = hm.find(0, val);
        EXPECT_FALSE(res);

        res = hm.erase(0);
        EXPECT_FALSE(res);
    }

    static void testInsert()
    {
        map_type hm;

        bool res = false;
        size_type size = 0;

        res = hm.insert(1, -1);
        EXPECT_TRUE(res);

        size = hm.size();
        EXPECT_EQ(static_cast<size_type>(1), size);

        res = hm.insert(1, -2);
        EXPECT_FALSE(res);

        size = hm.size();
        EXPECT_EQ(static_cast<size_type>(1), size);
    }

    static void testInsertOrUpdate()
    {
        map_type hm;

        bool res = false;
        size_type size = 0;

        hm.insertOrUpdate(1, -1);

        size = hm.size();
        EXPECT_EQ(static_cast<size_type>(1), size);

        hm.insertOrUpdate(1, -2);

        size = hm.size();
        EXPECT_EQ(static_cast<size_type>(1), size);
    }

    static void testFind()
    {
        map_type hm;

        bool res = false;
        size_type size = 0;
        mapped_type val;

        res = hm.find(1, val);
        EXPECT_FALSE(res);

        hm.insert(1, -1);
        res = hm.find(1, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(mapped_type(-1), val);

        hm.insert(1, -2);
        res = hm.find(1, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(mapped_type(-1), val);

        hm.insertOrUpdate(1, -2);
        res = hm.find(1, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(mapped_type(-2), val);

        res = hm.find(123, val);
        EXPECT_FALSE(res);

        res = hm.find(-1, val);
        EXPECT_FALSE(res);

        res = hm.find(321, val);
        EXPECT_FALSE(res);
    }

    static void testErase()
    {
        map_type hm;

        bool res = false;
        size_type size = 0;
        mapped_type val;

        res = hm.erase(1);
        EXPECT_FALSE(res);

        hm.insert(1, -1);

        res = hm.erase(2);
        EXPECT_FALSE(res);

        res = hm.erase(-1);
        EXPECT_FALSE(res);

        res = hm.erase(123);
        EXPECT_FALSE(res);

        res = hm.find(1, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-1));

        res = hm.find(-1, val);
        EXPECT_FALSE(res);

        res = hm.erase(1);
        EXPECT_TRUE(res);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(0));

        res = hm.find(1, val);
        EXPECT_FALSE(res);

        res = hm.erase(2);
        EXPECT_FALSE(res);

        res = hm.erase(1);
        EXPECT_FALSE(res);

        res = hm.erase(1);
        EXPECT_FALSE(res);
    }

    static void testCollision()
    {
        map_type hm;

        bool res = false;
        size_type size = 0;
        mapped_type val;

        hm.insert(1, -1);
        hm.insert(2, -2);
        hm.insert(3, -3);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(3));

        res = hm.find(1, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-1));

        res = hm.find(2, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-2));

        res = hm.find(3, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-3));

        res = hm.find(0, val);
        EXPECT_FALSE(res);

        res = hm.erase(2);
        EXPECT_TRUE(res);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(2));

        res = hm.find(1, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-1));

        res = hm.find(2, val);
        EXPECT_FALSE(res);

        res = hm.find(3, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-3));

        res = hm.find(0, val);
        EXPECT_FALSE(res);
    }

    static void testReuseKey()
    {
        map_type hm;

        bool res = false;
        size_type size = 0;
        mapped_type val;

        hm.insert(1, -1);
        hm.insert(2, -2);
        hm.insert(3, -3);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(3));

        res = hm.find(1, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-1));

        res = hm.find(2, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-2));

        res = hm.find(3, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-3));

        res = hm.find(0, val);
        EXPECT_FALSE(res);

        res = hm.erase(2);
        EXPECT_TRUE(res);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(2));

        res = hm.find(1, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-1));

        res = hm.find(2, val);
        EXPECT_FALSE(res);

        res = hm.find(3, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-3));

        res = hm.find(0, val);
        EXPECT_FALSE(res);

        hm.insert(2, -2);

        size = hm.size();
        EXPECT_EQ(size, static_cast<size_type>(3));

        res = hm.find(1, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-1));

        res = hm.find(2, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-2));

        res = hm.find(3, val);
        EXPECT_TRUE(res);
        EXPECT_EQ(val, static_cast<mapped_type>(-3));

        res = hm.find(0, val);
        EXPECT_FALSE(res);
    }

    static void testRehash()
    {
        map_type hm;

        size_type size = 0;
        size_type count = 0;
        bool result = false;
        mapped_type val;

        size = hm.getCapacity() * 2 + 1;

        for (int i = 1; i <= size; ++i)
        {
            result = hm.insert(i, i + 100);
            EXPECT_TRUE(result);
            result = hm.find(i, val);
            EXPECT_TRUE(result);
            EXPECT_EQ(static_cast<mapped_type>(i + 100), val);
        }

        count = hm.getCapacity();

        EXPECT_GT(count, size);
        count = hm.size();
        EXPECT_EQ(count, size);

        for (int i = 1; i <= size; ++i)
        {
            result = hm.find(i, val);
            EXPECT_TRUE(result);
            if (result)
            {
                EXPECT_EQ(i + 100, val);
            }
            result = hm.find(-i, val);
            EXPECT_FALSE(result);
        }
    }

    static void testRandom()
    {
        srand(time(nullptr));

        map_type hm;

        static const int maxval = 10000;
        std::size_t count_inserted = 0;
        for (int i = 0; i < 1000; ++i)
        {
            int k = rand() % maxval;
            int v = -k;
            if (hm.insert(k, v))
            {
                ++count_inserted;
            }
        }

        std::size_t count_found = 0;
        mapped_type v;
        for (int i = 0; i <= maxval; ++i)
        {
            key_type k = i;
            if (hm.find(k, v))
            {
                ++count_found;

                typedef typename xtomic::my::remove_wrapper<key_type>::type key_int_type;
                typedef typename xtomic::my::remove_wrapper<mapped_type>::type mapped_int_type;

                EXPECT_EQ(static_cast<mapped_int_type>(v),
                        -static_cast<key_int_type>(k));
            }
        }
        EXPECT_EQ(count_inserted, count_found);
    }

    // just unsure that it is compilable
    template<typename InnerKey, typename InnerValue>
    struct compile_tester
    {
        typedef typename xtomic::my::make_value_type<key_type, InnerKey>::type sized_key_type;
        typedef typename xtomic::my::make_value_type<mapped_type, InnerValue>::type sized_mapped_type;
        typedef typename xtomic::my::make_hash<sized_key_type>::type inner_hash_type;

        typedef std::equal_to<sized_key_type> equal_func_type;
        typedef std::allocator<mapped_type> allocator_type;

        typedef xtomic::hash_map<sized_key_type, sized_mapped_type,
                inner_hash_type, equal_func_type, allocator_type,
                map_type::MEMORY_MODEL> inner_hash_map_type;
        typedef typename inner_hash_map_type::size_type size_type;

        static constexpr bool INTEGRAL_KEY = inner_hash_map_type::INTEGRAL_KEY;
        static constexpr bool INTEGRAL_VALUE =
                inner_hash_map_type::INTEGRAL_VALUE;
        static constexpr bool INTEGRAL_KEYVALUE =
                inner_hash_map_type::INTEGRAL_KEYVALUE;
        static constexpr xtomic::memory_model::type MEMORY_MODEL =
                inner_hash_map_type::MEMORY_MODEL;

        void operator()() const
        {
            inner_hash_map_type hm;

            size_type size = 0;
            sized_mapped_type val;

            hm.insert(1, -1);
            hm.find(1, val);
            hm.erase(1);
            hm.size();

#define check_traits_flag(constant) \
                do { \
                if ( expected_map_traits_type::constant ) EXPECT_TRUE(constant); \
                else  EXPECT_FALSE(constant); \
                } while ( false )

            check_traits_flag(INTEGRAL_KEY);
            check_traits_flag(INTEGRAL_VALUE);
            check_traits_flag(INTEGRAL_KEYVALUE);

#undef check_traits_flag

            const xtomic::memory_model::type memModel = MEMORY_MODEL;
            EXPECT_EQ(MemoryModel, memModel);
        }
    };

    static void testDataTypes()
    {

        compile_tester<int8_t, int8_t>()();
        compile_tester<int8_t, int16_t>()();
        compile_tester<int8_t, int32_t>()();
        compile_tester<int8_t, int64_t>()();
        compile_tester<int16_t, int8_t>()();
        compile_tester<int16_t, int16_t>()();
        compile_tester<int16_t, int32_t>()();
        compile_tester<int16_t, int64_t>()();
        compile_tester<int32_t, int8_t>()();
        compile_tester<int32_t, int16_t>()();
        compile_tester<int32_t, int32_t>()();
        compile_tester<int32_t, int64_t>()();
        compile_tester<int64_t, int8_t>()();
        compile_tester<int64_t, int16_t>()();
        compile_tester<int64_t, int32_t>()();
    }

    static void testSnapshotEmpty()
    {
        snapshot_type snapshot;
        snapshot.push_back(std::make_pair(1, 1));

        map_type hm;

        hm.getSnapshot(snapshot);

        EXPECT_EQ(snapshot.size(), 0);

    }

    static void testSnapshot()
    {
        snapshot_type snapshot;

        map_type hm;

        hm.insert(1, 1);

        hm.getSnapshot(snapshot);

        EXPECT_EQ(snapshot.size(), 1);

        EXPECT_EQ(snapshot.front().first, static_cast<key_type>(1));
        EXPECT_EQ(snapshot.front().second, static_cast<mapped_type>(1));
    }
    static void testMultithread()
    {
        map_type map;
        mt_test_type test(map);

        test.run();

        size_type expected = 0;

        EXPECT_EQ(expected, test.getFailsOnInsert());
        EXPECT_EQ(expected, test.getFailsOnFind());
        EXPECT_EQ(expected, test.getFailsOnMissing());
        EXPECT_EQ(expected, test.getFailsOnErase());
    }
};

template<typename Key, typename Value, map_type::type MapType>
struct make_map_uniform_tests
{
    typedef unform_hash_map_tester<Key, Value, xtomic::memory_model::greedy,
            MapType> greedy_test_type;
    typedef unform_hash_map_tester<Key, Value, xtomic::memory_model::wise,
            MapType> wise_test_type;
};

#define MAKE_MAP_UNIT_TEST(test_maker, suite, testFunc) \
        TEST(GreedyHashMap_##suite, testFunc) \
        { test_maker::greedy_test_type::test##testFunc(); } \
        TEST(WiseHashMap_##suite, testFunc) \
        { test_maker::wise_test_type::test##testFunc(); }

#define MAKE_ALL_TESTS_FOR_MAP2(test_maker, suite) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, TypeTraits)

#define MAKE_ALL_TESTS_FOR_MAP(test_maker, suite) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, TypeTraits) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, Empty) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, Insert) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, InsertOrUpdate) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, Find) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, Erase) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, Collision) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, ReuseKey) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, Rehash) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, Random) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, DataTypes) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, SnapshotEmpty) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, Snapshot) \
        MAKE_MAP_UNIT_TEST(test_maker, suite, Multithread)

#endif /* TESTS_UNITS_UNIFORM_HASH_MAP_TEST_HPP_ */
