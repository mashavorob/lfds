/*
 * queue.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#include "gtest/gtest.h"

#include <xtomic/queue.hpp>

TEST(FixedSizeQueue, pop)
{
    int val;
    typedef xtomic::queue<int, xtomic::Queue::FixedSize> queue_type;

    queue_type q(1);

    EXPECT_FALSE(q.pop(val));
}

TEST(FixedSizeQueue, pushpoppop)
{
    int val = 0;
    typedef xtomic::queue<int, xtomic::Queue::FixedSize> queue_type;

    queue_type q(1);

    EXPECT_TRUE(q.push(1));
    EXPECT_FALSE(q.push(2));
    EXPECT_TRUE(q.pop(val));
    EXPECT_FALSE(q.pop(val));
    EXPECT_EQ(val, 1);
}

TEST(FixedSizeQueue, sequence)
{
    int val = 0;
    volatile bool res = false;
    typedef xtomic::queue<int, xtomic::Queue::FixedSize> queue_type;

    queue_type q(3);

    res = q.push(1);
    EXPECT_TRUE(res);
    res = q.push(2);
    EXPECT_TRUE(res);
    res = q.push(3);
    EXPECT_TRUE(res);
    res = q.push(4);
    EXPECT_FALSE(res);


    // multithreaded queue provides odd sequence
    res = q.pop(val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, 3);
    res = q.pop(val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, 2);

    res = q.pop(val);
    EXPECT_TRUE(res);
    EXPECT_EQ(val, 1);

    res = q.pop(val);
    EXPECT_FALSE(res);
    EXPECT_EQ(val, 1);
}

TEST(DynamicSizeQueue, pop)
{
    int val;
    typedef xtomic::queue<int, xtomic::Queue::DynamicSize> queue_type;

    queue_type q(1);

    EXPECT_FALSE(q.pop(val));
}

TEST(DynamicSizeQueue, pushpoppop)
{
    int val = 0;
    typedef xtomic::queue<int, xtomic::Queue::DynamicSize> queue_type;

    queue_type q(1);

    EXPECT_TRUE(q.push(1));
    EXPECT_TRUE(q.pop(val));
    EXPECT_FALSE(q.pop(val));
    EXPECT_EQ(val, 1);
}

TEST(DynamicSizeQueue, sequence)
{
    int val = 0;
    typedef xtomic::queue<int, xtomic::Queue::DynamicSize> queue_type;

    queue_type q(3);

    EXPECT_TRUE(q.push(1));
    EXPECT_TRUE(q.push(2));
    EXPECT_TRUE(q.push(3));
    EXPECT_TRUE(q.push(4));

    // multithreaded queue does not guarantyy items order
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(val, 4);
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(val, 3);
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(val, 1);

    EXPECT_FALSE(q.pop(val));
}

TEST(WaitFreeQueue, traits)
{
    int val;
    typedef xtomic::queue<int, xtomic::Queue::FixedSize, xtomic::Queue::OneProducer, xtomic::Queue::OneConsumer> queue_type;

    EXPECT_TRUE(queue_type::fixed_size);
    EXPECT_FALSE(queue_type::many_producers);
    EXPECT_FALSE(queue_type::many_consumers);
    EXPECT_TRUE(queue_type::wait_free);
}

TEST(WaitFreeQueue, pop)
{
    int val;
    typedef xtomic::queue<int, xtomic::Queue::FixedSize, xtomic::Queue::OneProducer, xtomic::Queue::OneConsumer> queue_type;

    queue_type q(1);

    EXPECT_FALSE(q.pop(val));
}

TEST(WaitFreeQueue, pushpoppop)
{
    int val = 0;
    typedef xtomic::queue<int, xtomic::Queue::FixedSize, xtomic::Queue::OneProducer, xtomic::Queue::OneConsumer> queue_type;

    queue_type q(1);

    EXPECT_TRUE(q.push(1));
    EXPECT_FALSE(q.push(1));
    EXPECT_TRUE(q.pop(val));
    EXPECT_FALSE(q.pop(val));
    EXPECT_EQ(val, 1);
}

TEST(WaitFreeQueue, sequence)
{
    int val = 0;
    typedef xtomic::queue<int, xtomic::Queue::FixedSize, xtomic::Queue::OneProducer, xtomic::Queue::OneConsumer> queue_type;

    queue_type q(3);

    EXPECT_TRUE(q.push(1));
    EXPECT_TRUE(q.push(2));
    EXPECT_TRUE(q.push(3));
    EXPECT_FALSE(q.push(4));

    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(val, 1);
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(q.pop(val));
    EXPECT_EQ(val, 3);
    EXPECT_FALSE(q.pop(val));
}
