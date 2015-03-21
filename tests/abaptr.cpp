/*
 * abaptr.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#include <aba_ptr.hpp>
#include "gtest/gtest.h"


TEST(abaptr, Negative)
{
    int val[] =
    { 0, 0 };

    lfds::abaptr<int> a(val);

    lfds::abaptr<int> expected = a;
    ++a.m_counter;

    lfds::abaptr<int> b(val + 1);
    EXPECT_FALSE(a.atomic_cas(expected, b));
    EXPECT_EQ(a.m_ptr, val);
}

TEST(abaptr, Positive)
{
    int val[] =
    { 0, 0 };

    lfds::abaptr<int> a(val);

    lfds::abaptr<int> expected = a;
    lfds::abaptr<int> b(val + 1);
    EXPECT_TRUE(a.atomic_cas(expected, b));
    EXPECT_EQ(a.m_ptr, val + 1);
}

