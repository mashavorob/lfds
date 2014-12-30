/*
 * stack_base.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#include "stack_base_aba.hpp"
#include "gtest/gtest.h"

TEST(stack_base_aba, pop)
{
	typedef lfds::stack_base_aba<int> stack_type;

	stack_type stack;
	EXPECT_EQ(stack.pop(), nullptr);
}

TEST(stack_base_aba, push)
{
	typedef lfds::stack_base_aba<int>	stack_type;
	typedef stack_type::node_type	node_type;

	stack_type stack;
	node_type	a;
	stack.push(&a);
	EXPECT_EQ(stack.pop(), &a);
	EXPECT_EQ(stack.pop(), nullptr);
}

TEST(stack_base_aba, push3)
{
	typedef lfds::stack_base_aba<int>	stack_type;
	typedef stack_type::node_type	node_type;

	stack_type stack;
	node_type	a[3];
	stack.push(&a[0]);
	stack.push(&a[1]);
	stack.push(&a[2]);
	EXPECT_EQ(stack.pop(), &a[2]);
	EXPECT_EQ(stack.pop(), &a[1]);
	EXPECT_EQ(stack.pop(), &a[0]);
	EXPECT_EQ(stack.pop(), nullptr);
}

TEST(stack_base_aba, atomic_pop)
{
	typedef lfds::stack_base_aba<int> stack_type;

	stack_type stack;
	EXPECT_EQ(stack.atomic_pop(), nullptr);
}

TEST(stack_base_aba, atomic_push)
{
	typedef lfds::stack_base_aba<int>	stack_type;
	typedef stack_type::node_type	node_type;

	stack_type stack;
	node_type	a;
	stack.atomic_push(&a);
	EXPECT_EQ(stack.atomic_pop(), &a);
	EXPECT_EQ(stack.atomic_pop(), nullptr);
}

TEST(stack_base_aba, atomic_push3)
{
	typedef lfds::stack_base_aba<int>	stack_type;
	typedef stack_type::node_type	node_type;

	stack_type stack;
	node_type	a[3];
	stack.atomic_push(&a[0]);
	stack.atomic_push(&a[1]);
	stack.atomic_push(&a[2]);
	EXPECT_EQ(stack.atomic_pop(), &a[2]);
	EXPECT_EQ(stack.atomic_pop(), &a[1]);
	EXPECT_EQ(stack.atomic_pop(), &a[0]);
	EXPECT_EQ(stack.atomic_pop(), nullptr);
}

TEST(stack_base_aba, copy_upsidedown)
{
	typedef lfds::stack_base_aba<int>	stack_type;
	typedef stack_type::node_type	node_type;

	stack_type stack1;
	node_type	a[3];
	stack1.atomic_push(&a[0]);
	stack1.atomic_push(&a[1]);
	stack1.atomic_push(&a[2]);

	stack_type stack2;
	stack_type::copy_upsidedown(stack1, stack2);
	EXPECT_EQ(stack1.atomic_pop(), nullptr);
	EXPECT_EQ(stack2.atomic_pop(), &a[0]);
	EXPECT_EQ(stack2.atomic_pop(), &a[1]);
	EXPECT_EQ(stack2.atomic_pop(), &a[2]);
	EXPECT_EQ(stack2.atomic_pop(), nullptr);
}
