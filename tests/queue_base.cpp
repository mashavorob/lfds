/*
 * queue_base.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#include "gtest/gtest.h"

#include "queue_base.hpp"

TEST(queue_base, pop)
{
	typedef lfds::queue_base<int, false, false> queue_type;

	queue_type queue;
	EXPECT_EQ(queue.pop(), nullptr);
}

TEST(queue_base, push)
{
	typedef lfds::queue_base<int, false, false> queue_type;
	typedef queue_type::node_type	node_type;

	queue_type queue;
	node_type	a;
	queue.push(&a);
	EXPECT_EQ(queue.pop(), &a);
	EXPECT_EQ(queue.pop(), nullptr);
}

TEST(queue_base, push3)
{
	typedef lfds::queue_base<int, false, false> queue_type;
	typedef queue_type::node_type	node_type;

	queue_type queue;
	node_type	a[3];
	queue.push(&a[0]);
	queue.push(&a[1]);
	queue.push(&a[2]);
	EXPECT_EQ(queue.pop(), &a[0]);
	EXPECT_EQ(queue.pop(), &a[1]);
	EXPECT_EQ(queue.pop(), &a[2]);
	EXPECT_EQ(queue.pop(), nullptr);
}

TEST(queue_base, atomic_pop)
{
	typedef lfds::queue_base<int, false, false> queue_type;

	queue_type queue;
	EXPECT_EQ(queue.atomic_pop(), nullptr);
}

TEST(queue_base, atomic_push)
{
	typedef lfds::queue_base<int, false, false> queue_type;
	typedef queue_type::node_type	node_type;

	queue_type queue;
	EXPECT_EQ(queue.atomic_pop(), nullptr);
	node_type	a;
	queue.atomic_push(&a);
	EXPECT_EQ(queue.atomic_pop(), &a);
	EXPECT_EQ(queue.atomic_pop(), nullptr);
}

TEST(queue_base, atomic_push3)
{
	typedef lfds::queue_base<int, false, false> queue_type;
	typedef queue_type::node_type	node_type;

	queue_type queue;
	EXPECT_EQ(queue.atomic_pop(), nullptr);

	node_type	a[3];
	queue.atomic_push(&a[0]);
	queue.atomic_push(&a[1]);
	queue.atomic_push(&a[2]);
	EXPECT_EQ(queue.atomic_pop(), &a[0]);
	EXPECT_EQ(queue.atomic_pop(), &a[1]);
	EXPECT_EQ(queue.atomic_pop(), &a[2]);
	EXPECT_EQ(queue.atomic_pop(), nullptr);
}
