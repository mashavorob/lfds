/*
 * test_many2many.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: masha
 */

#include "queue_many2many.hpp"

#include "gtest/gtest.h"

#include <xtomic/queue.hpp>

TEST(MT_LockFreeQueue, fixedSize_manyProducers_singleConsumer)
{
    enum {
        Size = 1000000,
        QueueSize = 100,
    };

    typedef xtomic::queue<int, xtomic::Queue::FixedSize, xtomic::Queue::ManyProducers, xtomic::Queue::OneConsumer> queue_type;
    typedef xtomic::testing::queue_many2many<queue_type, Size, 3, 1> test_type;

    queue_type q(QueueSize);
    test_type test(q);

    test.run();

    EXPECT_EQ(0, test.getNumOfIncorrectValues());
    EXPECT_TRUE(test.isDataComplete());
    EXPECT_LE(test.getMaxSequenceDiff(), Size/2);
}

TEST(MT_LockFreeQueue, dynamicSize_manyProducers_singleConsumer)
{
    enum {
        Size = 1000000,
        QueueSize = 100,
    };

    typedef xtomic::queue<int, xtomic::Queue::DynamicSize, xtomic::Queue::ManyProducers, xtomic::Queue::OneConsumer> queue_type;
    typedef xtomic::testing::queue_many2many<queue_type, Size, 3, 1> test_type;

    queue_type q(QueueSize);
    test_type test(q);

    test.run();

    EXPECT_EQ(0, test.getNumOfFailedPushes());
    EXPECT_EQ(0, test.getNumOfIncorrectValues());
    EXPECT_TRUE(test.isDataComplete());
    EXPECT_LE(test.getMaxSequenceDiff(), Size);
}

TEST(MT_LockFreeQueue, fixedSize_manyProducers_manyConsumers)
{
    enum {
        Size = 1000000,
        QueueSize = 100,
    };

    typedef xtomic::queue<int, xtomic::Queue::FixedSize, xtomic::Queue::ManyProducers, xtomic::Queue::ManyConsumers> queue_type;
    typedef xtomic::testing::queue_many2many<queue_type, Size, 2, 2> test_type;

    queue_type q(QueueSize);
    test_type test(q);

    test.run();

    EXPECT_EQ(0, test.getNumOfIncorrectValues());
    EXPECT_TRUE(test.isDataComplete());
    EXPECT_LE(test.getMaxSequenceDiff(), Size);
}

TEST(MT_LockFreeQueue, dynamicSize_manyProducers_manyConsumers)
{
    enum {
        Size = 1000000,
        QueueSize = 100,
    };

    typedef xtomic::queue<int, xtomic::Queue::DynamicSize, xtomic::Queue::ManyProducers, xtomic::Queue::ManyConsumers> queue_type;
    typedef xtomic::testing::queue_many2many<queue_type, Size, 2, 2> test_type;

    queue_type q(QueueSize);
    test_type test(q);

    test.run();

    EXPECT_EQ(0, test.getNumOfFailedPushes());
    EXPECT_EQ(0, test.getNumOfIncorrectValues());
    EXPECT_TRUE(test.isDataComplete());
    EXPECT_LE(test.getMaxSequenceDiff(), Size);
}
