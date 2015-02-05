/*
 * hash_map.cpp
 *
 *  Created on: Jan 28, 2015
 *      Author: masha
 */

#include "gtest/gtest.h"

#include "hash_map.hpp"

#include <string>
#include <functional>
#include <ctime>
#include <cstdlib>

namespace lfds
{
namespace testing
{

template<class T>
class adapter
{
public:
    adapter() :
            m_t()
    {

    }
    adapter(T t) :
            m_t(t)
    {

    }
    adapter(const adapter<T> & other) :
            m_t(other.m_t)
    {

    }
    bool operator==(const adapter<T> & other) const
    {
        return m_t == other.m_t;
    }
    std::size_t hash() const
    {
        static const std::hash<T> hasher;
        return hasher(m_t);
    }
public:
    T m_t;
};
}
}

namespace std
{
template<>
struct hash<lfds::testing::adapter<int> > : public unary_function<
        lfds::testing::adapter<int>, std::size_t>
{
    size_t operator()(const lfds::testing::adapter<int>& val) const
    {
        return val.hash();
    }
};
}

struct bad_hash: public std::unary_function<lfds::testing::adapter<int>,
        std::size_t>
{
    std::size_t operator()(const lfds::testing::adapter<int>& val) const
    {
        return 0;
    }
};

template<class T>
inline bool operator==(const lfds::testing::adapter<T> & a, T b)
{
    return a.m_t == b;
}

template<class T>
inline bool operator==(T a, const lfds::testing::adapter<T> & b)
{
    return a == b.m_t;
}

TEST(HashMap, empty)
{
    typedef lfds::testing::adapter<int> value_type; // prevent optimization
    typedef lfds::hash_map<value_type, value_type> hash_map;
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

TEST(HashMap, insert)
{
    typedef lfds::testing::adapter<int> value_type; // prevent optimization
    typedef lfds::hash_map<value_type, value_type> hash_map;
    typedef hash_map::size_type size_type;

    hash_map hm;

    bool res = false;
    size_type size = 0;

    res = hm.insert(0, -1);
    EXPECT_TRUE(res);

    size = hm.size();
    EXPECT_EQ(size, 1);
}

TEST(HashMap, find)
{
    typedef lfds::testing::adapter<int> value_type; // prevent optimization
    typedef lfds::hash_map<value_type, value_type> hash_map;
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

TEST(HashMap, erase)
{
    typedef lfds::testing::adapter<int> value_type; // prevent optimization
    typedef lfds::hash_map<value_type, value_type> hash_map;
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

TEST(HashMap, collision)
{
    typedef lfds::testing::adapter<int> value_type; // prevent optimization
    typedef lfds::hash_map<value_type, value_type, bad_hash> hash_map; // same hash for all
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

TEST(HashMap, reusekey)
{
    typedef lfds::testing::adapter<int> value_type; // prevent optimization
    typedef lfds::hash_map<value_type, value_type> hash_map;
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

TEST(HashMap, rehash)
{
    typedef lfds::testing::adapter<int> value_type; // prevent optimization
    typedef lfds::hash_map<value_type, value_type> hash_map;
    typedef hash_map::size_type size_type;

    hash_map hm;

    size_type size = 0;
    size_type count = 0;
    bool result = false;
    value_type val;

    size = hm.capacity()*2 + 1;

    for (int i = 1; i <= size; ++i)
    {
        result = hm.insert(i, i + 100);
        EXPECT_TRUE(result);
        result = hm.find(i, val);
        EXPECT_TRUE(result);
        EXPECT_EQ(i + 100, val.m_t);
    }

    count = hm.capacity();

    EXPECT_GT(count, size);
    count = hm.size();
    EXPECT_EQ(count, size);

    for (int i = 1; i <= size; ++i)
    {
        result = hm.find(i, val);
        EXPECT_TRUE(result);
        if ( result )
        {
            EXPECT_EQ(i + 100, val.m_t);
        }
        result = hm.find(-i, val);
        EXPECT_FALSE(result);
    }
}

TEST(HashMap, random)
{
    typedef lfds::testing::adapter<int> value_type; // prevent optimization
    typedef lfds::hash_map<value_type, value_type> hash_map;
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
        value_type k = i;
        if (hm.find(k, v))
        {
            ++count_found;
            EXPECT_EQ(v.m_t, -k.m_t);
        }
    }
    EXPECT_EQ(count_inserted, count_found);
}
