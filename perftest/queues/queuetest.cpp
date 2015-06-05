/*
 * queuetest.cpp
 *
 *  Created on: Jun 2, 2015
 *      Author: masha
 */


#include <xtomic/queue.hpp>
#include <xtomic/aux/inttypes.hpp>

#include <vector>

#include "testfactory.hpp"
#include "queuetest.hpp"
#include "stdqueue.hpp"


namespace xtomic
{
namespace perftest
{
namespace queues
{
typedef xtomic::uint64_t item_type;

typedef xtomic::queue<item_type, xtomic::Queue::FixedSize,
        xtomic::Queue::OneProducer, xtomic::Queue::OneConsumer> wait_free_queue_type;

typedef xtomic::queue<item_type, xtomic::Queue::FixedSize,
        xtomic::Queue::ManyProducers, xtomic::Queue::OneConsumer> lock_free_one_consumer_queue_type;

typedef xtomic::queue<item_type, xtomic::Queue::FixedSize,
        xtomic::Queue::ManyProducers, xtomic::Queue::ManyConsumers> lock_free_many_consumers_queue_type;

typedef adapter::stdqueue<item_type> stl_queue_type;

template<typename Queue>
class Registrar
{
private:
    typedef Queue queue_type;
    typedef BandwithTester<queue_type> tester_type;
    typedef PerfTestFactoryImpl<tester_type> factory_type;
public:
    Registrar(const char* queue_name) :
            m_factory("queues", queue_name, "bandwith", "MItems/sec")
    {

    }
private:
    factory_type m_factory;
};

static Registrar<wait_free_queue_type> s_wfq("wait free queue");
static Registrar<lock_free_one_consumer_queue_type> s_lfscq("lock free single consumer queue");
static Registrar<lock_free_many_consumers_queue_type> s_lfmcq("lock free many consumers queue");
static Registrar<stl_queue_type> s_stdq("std::queue");

}
}
}

