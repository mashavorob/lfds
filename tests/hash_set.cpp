/*
 * hash_set.cpp
 *
 *  Created on: Feb 12, 2015
 *      Author: masha
 */

#include <gtest/gtest.h>

#include <hash_set.hpp>

#include "hash_map_data_adapter.hpp"

TEST(HashSet, type_traits)
{
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::hash_set<key_type> hash_set;

    EXPECT_FALSE(hash_set::INTEGRAL);
}

TEST(HashSet, empty)
{
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::hash_set<key_type> hash_set;
    typedef hash_set::size_type size_type;

    hash_set hs;

    bool res = false;
    size_type size = 0;

    size = hs.size();
    EXPECT_EQ(size, 0);

    res = hs.find(0);
    EXPECT_FALSE(res);

    res = hs.erase(0);
    EXPECT_FALSE(res);
}

TEST(HashSet, insert)
{
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::hash_set<key_type> hash_set;
    typedef hash_set::size_type size_type;

    hash_set hs;

    bool res = false;
    size_type size = 0;

    res = hs.insert(0);
    EXPECT_TRUE(res);

    size = hs.size();
    EXPECT_EQ(size, 1);
}

TEST(HashSet, find)
{
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::hash_set<key_type> hash_set;
    typedef hash_set::size_type size_type;

    hash_set hs;

    bool res = false;
    size_type size = 0;

    res = hs.find(1);
    EXPECT_FALSE(res);

    hs.insert(1);
    res = hs.find(1);
    EXPECT_TRUE(res);

    res = hs.find(123);
    EXPECT_FALSE(res);

    res = hs.find(-1);
    EXPECT_FALSE(res);

    res = hs.find(0);
    EXPECT_FALSE(res);
}

TEST(HashSet, erase)
{
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::hash_set<key_type> hash_set;
    typedef hash_set::size_type size_type;

    hash_set hs;

    bool res = false;
    size_type size = 0;

    res = hs.erase(1);
    EXPECT_FALSE(res);

    hs.insert(1);

    res = hs.erase(2);
    EXPECT_FALSE(res);

    res = hs.erase(-1);
    EXPECT_FALSE(res);

    res = hs.erase(0);
    EXPECT_FALSE(res);

    res = hs.find(1);
    EXPECT_TRUE(res);

    res = hs.find(-1);
    EXPECT_FALSE(res);

    res = hs.erase(1);
    EXPECT_TRUE(res);

    size = hs.size();
    EXPECT_EQ(size, 0);

    res = hs.find(1);
    EXPECT_FALSE(res);

    res = hs.erase(2);
    EXPECT_FALSE(res);

    res = hs.erase(1);
    EXPECT_FALSE(res);

    res = hs.erase(1);
    EXPECT_FALSE(res);
}

TEST(HashSet, collision)
{
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::hash_set<key_type, bad_hash<key_type>> hash_set; // same hash for all
    typedef hash_set::size_type size_type;

    hash_set hs;

    bool res = false;
    size_type size = 0;

    hs.insert(1);
    hs.insert(2);
    hs.insert(3);

    size = hs.size();
    EXPECT_EQ(size, 3);

    res = hs.find(1);
    EXPECT_TRUE(res);

    res = hs.find(2);
    EXPECT_TRUE(res);

    res = hs.find(3);
    EXPECT_TRUE(res);

    res = hs.find(0);
    EXPECT_FALSE(res);

    res = hs.erase(2);
    EXPECT_TRUE(res);

    size = hs.size();
    EXPECT_EQ(size, 2);

    res = hs.find(1);
    EXPECT_TRUE(res);

    res = hs.find(2);
    EXPECT_FALSE(res);

    res = hs.find(3);
    EXPECT_TRUE(res);

    res = hs.find(0);
    EXPECT_FALSE(res);
}

TEST(HashSet, reusekey)
{
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::hash_set<key_type> hash_set;
    typedef hash_set::size_type size_type;

    hash_set hs;

    bool res = false;
    size_type size = 0;

    hs.insert(1);
    hs.insert(2);
    hs.insert(3);

    size = hs.size();
    EXPECT_EQ(size, 3);

    res = hs.find(1);
    EXPECT_TRUE(res);

    res = hs.find(2);
    EXPECT_TRUE(res);

    res = hs.find(3);
    EXPECT_TRUE(res);

    res = hs.find(0);
    EXPECT_FALSE(res);

    res = hs.erase(2);
    EXPECT_TRUE(res);

    size = hs.size();
    EXPECT_EQ(size, 2);

    res = hs.find(1);
    EXPECT_TRUE(res);

    res = hs.find(2);
    EXPECT_FALSE(res);

    res = hs.find(3);
    EXPECT_TRUE(res);

    res = hs.find(0);
    EXPECT_FALSE(res);

    hs.insert(2);

    size = hs.size();
    EXPECT_EQ(size, 3);

    res = hs.find(1);
    EXPECT_TRUE(res);

    res = hs.find(2);
    EXPECT_TRUE(res);

    res = hs.find(3);
    EXPECT_TRUE(res);

    res = hs.find(0);
    EXPECT_FALSE(res);
}

TEST(HashSet, rehash)
{
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::hash_set<key_type> hash_set;
    typedef hash_set::size_type size_type;

    hash_set hs;

    size_type size = 0;
    size_type count = 0;
    bool result = false;

    size = hs.capacity()*2 + 1;

    for (int i = 1; i <= size; ++i)
    {
        result = hs.insert(i);
        EXPECT_TRUE(result);
        result = hs.find(i);
        EXPECT_TRUE(result);
    }

    count = hs.capacity();

    EXPECT_GT(count, size);
    count = hs.size();
    EXPECT_EQ(count, size);

    for (int i = 1; i <= size; ++i)
    {
        result = hs.find(i);
        EXPECT_TRUE(result);
        result = hs.find(-i);
        EXPECT_FALSE(result);
    }
}

TEST(HashSet, random)
{
    typedef lfds::testing::adapter<int> key_type; // prevent optimization
    typedef lfds::hash_set<key_type> hash_set;
    typedef hash_set::size_type size_type;

    srand(time(nullptr));

    hash_set hs;

    static const int maxval = 10000;
    std::size_t count_inserted = 0;
    for (int i = 0; i < 1000; ++i)
    {
        int k = rand() % maxval;
        if (hs.insert(k))
        {
            ++count_inserted;
        }
    }
    std::size_t count_found = 0;
    for (int i = 0; i <= maxval; ++i)
    {
        key_type k = i;
        if (hs.find(k))
        {
            ++count_found;
        }
    }
    EXPECT_EQ(count_inserted, count_found);
}
