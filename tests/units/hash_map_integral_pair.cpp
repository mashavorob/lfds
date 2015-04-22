/*
 * hash_map_integral_pair.cpp
 *
 *  Created on: Feb 11, 2015
 *      Author: masha
 */

#include <gtest/gtest.h>

#include "hash_map_data_adapter.hpp"

#include <hash_map.hpp>
#include <inttypes.hpp>

#include <ctime>
#include <cstdlib>

/////////////////////////////////////////////////////////////////
// integral key
TEST(HashMap_integral_pair, type_traits)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;

    EXPECT_TRUE(hash_map::INTEGRAL_KEY);
    EXPECT_TRUE(hash_map::INTEGRAL_KEYVALUE);
}

TEST(HashMap_integral_pair, empty)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;
    typedef hash_map::size_type size_type;

    hash_map hm;

    bool res = false;
    size_type size = 0;

    size = hm.size();
    EXPECT_EQ(size, 0);

    value_type val;
    res = hm.find(0, val);
    EXPECT_FALSE(res);

    res = hm.erase(0);
    EXPECT_FALSE(res);
}

// just unsure that it is compilable
template<class Key, class Value>
struct compile_tester
{
    typedef Key key_type;
    typedef Value value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map_type;
    typedef typename hash_map_type::size_type size_type;

    void operator()() const
    {
        hash_map_type hm;

        size_type size = 0;
        value_type val;

        hm.insert(1, -1);
        hm.find(1, val);
        hm.erase(1);
        hm.size();

        EXPECT_TRUE(hash_map_type::INTEGRAL_KEYVALUE);
    }
};

TEST(HashMap_integral_pair, DataTypes)
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
    EXPECT_TRUE(true);
}

TEST(HashMap_integral_pair, OutOfLimit)
{
    typedef compile_tester<int64_t, int64_t>::hash_map_type hash_map;
    EXPECT_FALSE(hash_map::INTEGRAL_KEY);
    EXPECT_TRUE(hash_map::INTEGRAL_VALUE);
    EXPECT_FALSE(hash_map::INTEGRAL_KEYVALUE);
}

TEST(HashMap_integral_pair, insert)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;
    typedef hash_map::size_type size_type;

    hash_map hm;

    bool res = false;
    size_type size = 0;

    res = hm.insert(0, -1);
    EXPECT_TRUE(res);

    size = hm.size();
    EXPECT_EQ(size, 1);
}

TEST(HashMap_integral_pair, find)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;
    typedef hash_map::size_type size_type;

    hash_map hm;

    bool res = false;
    size_type size = 0;
    value_type val;

    res = hm.find(1, val);
    EXPECT_FALSE(res);

    hm.insert(1, -1);
    res = hm.find(1, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -1);

    res = hm.find(123, val);
    EXPECT_FALSE(res);

    res = hm.find(-1, val);
    EXPECT_FALSE(res);

    res = hm.find(0, val);
    EXPECT_FALSE(res);
}

TEST(HashMap_integral_pair, erase)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;
    typedef hash_map::size_type size_type;

    hash_map hm;

    bool res = false;
    size_type size = 0;
    value_type val;

    res = hm.erase(1);
    EXPECT_FALSE(res);

    hm.insert(1, -1);

    res = hm.erase(2);
    EXPECT_FALSE(res);

    res = hm.erase(-1);
    EXPECT_FALSE(res);

    res = hm.erase(0);
    EXPECT_FALSE(res);

    res = hm.find(1, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -1);

    res = hm.find(-1, val);
    EXPECT_FALSE(res);

    res = hm.erase(1);
    EXPECT_TRUE(res);

    size = hm.size();
    EXPECT_EQ(size, 0);

    res = hm.find(1, val);
    EXPECT_FALSE(res);

    res = hm.erase(2);
    EXPECT_FALSE(res);

    res = hm.erase(1);
    EXPECT_FALSE(res);

    res = hm.erase(1);
    EXPECT_FALSE(res);
}

TEST(HashMap_integral_pair, collision)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type, bad_hash<key_type> > hash_map; // same hash for all
    typedef hash_map::size_type size_type;

    hash_map hm;

    bool res = false;
    size_type size = 0;
    value_type val;

    hm.insert(1, -1);
    hm.insert(2, -2);
    hm.insert(3, -3);

    size = hm.size();
    EXPECT_EQ(size, 3);

    res = hm.find(1, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -1);

    res = hm.find(2, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -2);

    res = hm.find(3, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -3);

    res = hm.find(0, val);
    EXPECT_FALSE(res);

    res = hm.erase(2);
    EXPECT_TRUE(res);

    size = hm.size();
    EXPECT_EQ(size, 2);

    res = hm.find(1, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -1);

    res = hm.find(2, val);
    EXPECT_FALSE(res);

    res = hm.find(3, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -3);

    res = hm.find(0, val);
    EXPECT_FALSE(res);
}

TEST(HashMap_integral_pair, reusekey)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;
    typedef hash_map::size_type size_type;

    hash_map hm;

    bool res = false;
    size_type size = 0;
    value_type val;

    hm.insert(1, -1);
    hm.insert(2, -2);
    hm.insert(3, -3);

    size = hm.size();
    EXPECT_EQ(size, 3);

    res = hm.find(1, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -1);

    res = hm.find(2, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -2);

    res = hm.find(3, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -3);

    res = hm.find(0, val);
    EXPECT_FALSE(res);

    res = hm.erase(2);
    EXPECT_TRUE(res);

    size = hm.size();
    EXPECT_EQ(size, 2);

    res = hm.find(1, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -1);

    res = hm.find(2, val);
    EXPECT_FALSE(res);

    res = hm.find(3, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -3);

    res = hm.find(0, val);
    EXPECT_FALSE(res);

    hm.insert(2, -2);

    size = hm.size();
    EXPECT_EQ(size, 3);

    res = hm.find(1, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -1);

    res = hm.find(2, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -2);

    res = hm.find(3, val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, -3);

    res = hm.find(0, val);
    EXPECT_FALSE(res);
}

TEST(HashMap_integral_pair, rehash)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;
    typedef hash_map::size_type size_type;

    hash_map hm;

    size_type size = 0;
    size_type count = 0;
    bool result = false;
    value_type val;

    size = hm.getCapacity() * 2 + 1;

    for (int i = 1; i <= size; ++i)
    {
        result = hm.insert(i, i + 100);
        EXPECT_TRUE(result);
        result = hm.find(i, val);
        EXPECT_TRUE(result);
        EXPECT_EQ(i + 100, val);
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

TEST(HashMap_integral_pair, random)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;
    typedef hash_map::size_type size_type;

    srand(time(nullptr));

    hash_map hm;

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
    value_type v;
    for (int i = 0; i <= maxval; ++i)
    {
        key_type k = i;
        if (hm.find(k, v))
        {
            ++count_found;
            EXPECT_EQ(v, -k);
        }
    }
    EXPECT_EQ(count_inserted, count_found);
}

TEST(HashMap_integral_pair, snapshot_empty)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;
    typedef hash_map::size_type size_type;
    typedef hash_map::snapshot_type snapshot_type;

    snapshot_type snapshot;
    snapshot.push_back(std::make_pair(1, 1));

    hash_map hm;

    hm.getSnapshot(snapshot);

    EXPECT_EQ(snapshot.size(), 0);

}

TEST(HashMap_integral_pair, snapshot)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_map<key_type, value_type> hash_map;
    typedef hash_map::size_type size_type;
    typedef hash_map::snapshot_type snapshot_type;

    snapshot_type snapshot;

    hash_map hm;

    hm.insert(1, 1);

    hm.getSnapshot(snapshot);

    EXPECT_EQ(snapshot.size(), 1);

    EXPECT_EQ(snapshot.front().first, 1);
    EXPECT_EQ(snapshot.front().second, 1);
}

