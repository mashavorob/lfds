#include "gtest/gtest.h"

#include <utility>

#include "cas.hpp"

TEST(CAS1b, Negative)
{
    typedef char value_type;
    value_type a = 1;

    EXPECT_FALSE(lfds::atomic_cas(a, static_cast<value_type>(0), static_cast<value_type>(1)));
    EXPECT_EQ(static_cast<value_type>(1), a);
}

TEST(CAS1b, Positive)
{
    typedef char value_type;
    value_type a = 1;

    EXPECT_TRUE(lfds::atomic_cas(a, static_cast<value_type>(1), static_cast<value_type>(2)));
    EXPECT_EQ(static_cast<value_type>(2), a);
}

TEST(CAS2b, Negative)
{
    typedef short value_type;
    value_type a = 1;

    EXPECT_FALSE(lfds::atomic_cas(a, static_cast<value_type>(0), static_cast<value_type>(1)));
    EXPECT_EQ(static_cast<value_type>(1), a);
}

TEST(CAS2b, Positive)
{
    typedef short value_type;
    value_type a = 1;

    EXPECT_TRUE(lfds::atomic_cas(a, static_cast<value_type>(1), static_cast<value_type>(2)));
    EXPECT_EQ(static_cast<value_type>(2), a);
}

TEST(CAS4b, Negative)
{
    typedef int value_type;
    value_type a = 1;

    EXPECT_FALSE(lfds::atomic_cas(a, static_cast<value_type>(0), static_cast<value_type>(1)));
    EXPECT_EQ(static_cast<value_type>(1), a);
}

TEST(CAS4b, Positive)
{
    typedef int value_type;
    value_type a = 1;

    EXPECT_TRUE(lfds::atomic_cas(a, static_cast<value_type>(1), static_cast<value_type>(2)));
    EXPECT_EQ(static_cast<value_type>(2), a);
}

TEST(CAS8b, Negative)
{
    typedef long long value_type;
    value_type a(1);

    EXPECT_FALSE(lfds::atomic_cas(a, value_type(0), value_type(1)));
    EXPECT_EQ(a, 1);
}

TEST(CAS8b, Positive)
{
    typedef std::pair<int, int> value_type;
    value_type a(1, 2);

    EXPECT_TRUE(lfds::atomic_cas(a, value_type(1, 2), value_type(3, 4)));
    EXPECT_EQ(a, value_type(3, 4));

    long long var = (static_cast<long long>(1) << 32) | static_cast<long long>(2);
    long long e = var;
    long long n = (static_cast<long long>(3) << 32) | static_cast<long long>(4);

    EXPECT_TRUE(lfds::atomic_cas(var, e, n));
    EXPECT_EQ(var, n);

}

TEST(CAS16b, Negative)
{
    typedef std::pair<long long, long long> value_type;
    value_type a(1, 0);

    EXPECT_FALSE(lfds::atomic_cas(a, value_type(0, 1), value_type(1, 1)));
    EXPECT_EQ(a, value_type(1, 0));
}

TEST(CAS16b, Positive)
{
    typedef std::pair<long long, long long> value_type;
    value_type a(1, 2);

    EXPECT_TRUE(lfds::atomic_cas(a, value_type(1, 2), value_type(3, 4)));
    EXPECT_EQ(value_type(3, 4), a);
}
