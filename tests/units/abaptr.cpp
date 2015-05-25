/*
 * abaptr.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#include <xtomic/impl/aba_ptr.hpp>
#include "gtest/gtest.h"


TEST(aba_ptr, Negative)
{
    int val[] =
    { 0, 0 };

    lfds::aba_ptr<int> a(val);

    lfds::aba_ptr<int> expected = a;
    ++a.m_counter;

    lfds::aba_ptr<int> b(val + 1);
    EXPECT_FALSE(a.atomic_cas(expected, b));
    EXPECT_EQ(a.m_ptr, val);
}

TEST(aba_ptr, Positive)
{
    int val[] =
    { 0, 0 };

    lfds::aba_ptr<int> a(val);

    lfds::aba_ptr<int> expected = a;
    lfds::aba_ptr<int> b(val + 1);
    EXPECT_TRUE(a.atomic_cas(expected, b));
    EXPECT_EQ(a.m_ptr, val + 1);
}

