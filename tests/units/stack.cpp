/*
 * stack.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#include "gtest/gtest.h"

#include <xtomic/stack.hpp>

TEST(FixedSizeStack, pop)
{
    int val;
    typedef xtomic::stack<int, true> stack_type;

    stack_type s(1);

    EXPECT_FALSE(s.pop(val));
}

TEST(FixedSizeStack, pushpoppop)
{
    int val = 0;
    typedef xtomic::stack<int, true> stack_type;

    stack_type s(1);

    EXPECT_TRUE(s.push(1));
    EXPECT_FALSE(s.push(2));
    EXPECT_TRUE(s.pop(val));
    EXPECT_FALSE(s.pop(val));
    EXPECT_EQ(val, 1);
}

TEST(FixedSizeStack, sequence)
{
    int val = 0;
    typedef xtomic::stack<int, true> stack_type;

    stack_type s(3);

    EXPECT_TRUE(s.push(1));
    EXPECT_TRUE(s.push(2));
    EXPECT_TRUE(s.push(3));
    EXPECT_FALSE(s.push(4));

    EXPECT_TRUE(s.pop(val));
    EXPECT_EQ(val, 3);
    EXPECT_TRUE(s.pop(val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(s.pop(val));
    EXPECT_EQ(val, 1);

    EXPECT_FALSE(s.pop(val));
}

TEST(DynamicSizeStack, pop)
{
    int val;
    typedef xtomic::stack<int, false> stack_type;

    stack_type s(1);

    EXPECT_FALSE(s.pop(val));
}

TEST(DynamicSizeStack, pushpoppop)
{
    int val = 0;
    typedef xtomic::stack<int, false> stack_type;

    stack_type s(1);

    EXPECT_TRUE(s.push(1));
    EXPECT_TRUE(s.pop(val));
    EXPECT_FALSE(s.pop(val));
    EXPECT_EQ(val, 1);
}

TEST(DynamicSizeStack, sequence)
{
    int val = 0;
    typedef xtomic::stack<int, false> stack_type;

    stack_type s(3);

    EXPECT_TRUE(s.push(1));
    EXPECT_TRUE(s.push(2));
    EXPECT_TRUE(s.push(3));
    EXPECT_TRUE(s.push(4));

    EXPECT_TRUE(s.pop(val));
    EXPECT_EQ(val, 4);
    EXPECT_TRUE(s.pop(val));
    EXPECT_EQ(val, 3);
    EXPECT_TRUE(s.pop(val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(s.pop(val));
    EXPECT_EQ(val, 1);

    EXPECT_FALSE(s.pop(val));
}

