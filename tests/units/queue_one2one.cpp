/*
 * test_one2one.cpp
 *
 *  Created on: Apr 14, 2015
 *      Author: masha
 */

#include "queue_one2one.hpp"

#include "gtest/gtest.h"

#include <xtomic/queue.hpp>

TEST(MT_WaitFreeQueue, multithread)
{
    enum {
        Size = 1000000,
        QueueSize = 100,
    };

    typedef xtomic::queue<int, xtomic::Queue::FixedSize, xtomic::Queue::OneProducer, xtomic::Queue::OneConsumer> queue_type;
    typedef xtomic::testing::queue_one2one<queue_type, Size> test_type;


    queue_type q(QueueSize);
    test_type test(q);

    test.run();

    EXPECT_EQ(0, test.getNumOfIncorrectValues());
    EXPECT_TRUE(test.isDataComplete());
    EXPECT_LE(test.getMaxSequenceDiff(), 1);
}

TEST(MT_LockFreeQueue, fixedSize_oneProducer_OneConsumer)
{
    enum {
        Size = 1000000,
        QueueSize = 100,
    };

    typedef xtomic::queue<int, xtomic::Queue::DynamicSize, xtomic::Queue::OneProducer, xtomic::Queue::OneConsumer> queue_type;
    typedef xtomic::testing::queue_one2one<queue_type, Size> test_type;


    queue_type q(QueueSize);
    test_type test(q);

    test.run();

    EXPECT_EQ(0, test.getNumOfIncorrectValues());
    EXPECT_TRUE(test.isDataComplete());
    EXPECT_LE(test.getMaxSequenceDiff(), Size/2);
}

