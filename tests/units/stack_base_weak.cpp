/*
 * stack_base_weak.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: masha
 */

#include <xtomic/impl/stack_base_weak.hpp>
#include "gtest/gtest.h"

TEST(stack_base_weak, pop)
{
    typedef xtomic::stack_base_weak<int> stack_type;
    typedef stack_type::node_type node_type;

    stack_type stack;
    EXPECT_EQ(stack.pop(), static_cast<node_type*>(0));
}

TEST(stack_base_weak, push)
{
    typedef xtomic::stack_base_weak<int> stack_type;
    typedef stack_type::node_type node_type;

    stack_type stack;
    node_type a;
    stack.push(&a);
    EXPECT_EQ(stack.pop(), &a);
    EXPECT_EQ(stack.pop(), static_cast<node_type*>(0));
}

TEST(stack_base_weak, push3)
{
    typedef xtomic::stack_base_weak<int> stack_type;
    typedef stack_type::node_type node_type;

    stack_type stack;
    node_type a[3];
    stack.push(&a[0]);
    stack.push(&a[1]);
    stack.push(&a[2]);
    EXPECT_EQ(stack.pop(), &a[2]);
    EXPECT_EQ(stack.pop(), &a[1]);
    EXPECT_EQ(stack.pop(), &a[0]);
    EXPECT_EQ(stack.pop(), static_cast<node_type*>(0));
}

TEST(stack_base_weak, atomic_pop)
{
    typedef xtomic::stack_base_weak<int> stack_type;
    typedef stack_type::node_type node_type;

    stack_type stack;
    EXPECT_EQ(stack.atomic_pop(), static_cast<node_type*>(0));
}

TEST(stack_base_weak, atomic_push)
{
    typedef xtomic::stack_base_weak<int> stack_type;
    typedef stack_type::node_type node_type;

    stack_type stack;
    node_type a;
    stack.atomic_push(&a);
    EXPECT_EQ(stack.atomic_pop(), &a);
    EXPECT_EQ(stack.atomic_pop(), static_cast<node_type*>(0));
}

TEST(stack_base_weak, atomic_push3)
{
    typedef xtomic::stack_base_weak<int> stack_type;
    typedef stack_type::node_type node_type;

    stack_type stack;
    node_type a[3];
    stack.atomic_push(&a[0]);
    stack.atomic_push(&a[1]);
    stack.atomic_push(&a[2]);
    EXPECT_EQ(stack.atomic_pop(), &a[2]);
    EXPECT_EQ(stack.atomic_pop(), &a[1]);
    EXPECT_EQ(stack.atomic_pop(), &a[0]);
    EXPECT_EQ(stack.atomic_pop(), static_cast<node_type*>(0));
}

