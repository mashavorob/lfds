#include "gtest/gtest.h"

#include <utility>

#include "cas.hpp"

TEST(CAS4b, Negative)
{
	int a = 1;

    EXPECT_FALSE(lfds::atomic_cas(a, 0, 1));
    EXPECT_EQ(a, 1);
}

TEST(CAS4b, Positive)
{
	int a = 1;

    EXPECT_TRUE(lfds::atomic_cas(a, 1, 2));
    EXPECT_EQ(a, 2);
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
	typedef long long value_type;
	value_type a(1);

    EXPECT_TRUE(lfds::atomic_cas(a, value_type(1), value_type(2)));
    EXPECT_EQ(a, 2);
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
	value_type a(1, 0);

    EXPECT_TRUE(lfds::atomic_cas(a, value_type(1, 0), value_type(1, 1)));
    EXPECT_EQ(a, value_type(1, 1));
}
