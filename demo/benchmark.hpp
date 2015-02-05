/*
 * benchmark.hpp
 *
 *  Created on: Dec 16, 2014
 *      Author: masha
 */

#ifndef DEMO_BENCHMARK_HPP_
#define DEMO_BENCHMARK_HPP_

#include <thread>
#include <chrono>
#include <list>
#include <iostream>
#include "test_events.hpp"
#include "test_producer.hpp"
#include "test_consumer.hpp"

template<class Collection, class Duration>
static double benchmark(Collection & coll, Duration d, unsigned producers,
        unsigned consumers)
{

    typedef test_consumer<Collection> consumer_type;
    typedef test_producer<Collection> producer_type;

    typedef std::chrono::duration<double> seconds_type;
    typedef std::chrono::high_resolution_clock clock_type;
    typedef clock_type::duration duration_type;
    typedef clock_type::time_point time_point_type;

    test_events evs;

    if (!Collection::many_producers)
    {
        producers = 1;
    }
    if (!Collection::many_consumers)
    {
        consumers = 1;
    }
    std::list<std::thread> threads;

    consumer_type::reset_perf_counter();

    for (auto i = 0; i < producers; ++i)
    {
        producer_type producer(coll, evs.get_start_ev(), evs.get_stop_ev());
        threads.push_back(std::thread(producer));
    }
    for (auto i = 0; i < consumers; ++i)
    {
        consumer_type consumer(coll, evs.get_stop_ev());
        threads.push_back(std::thread(consumer));
    }
    evs.go();

    time_point_type start_point = clock_type::now();
    std::this_thread::sleep_for(d);
    evs.stop();
    while (!threads.empty())
    {
        threads.front().join();
        threads.pop_front();
    }
    time_point_type end_point = clock_type::now();
    seconds_type duration_seconds = std::chrono::duration_cast < seconds_type
            > (end_point - start_point);
    std::size_t processedItems = consumer_type::get_perf_counter();

    return processedItems / duration_seconds.count();
}

#endif /* DEMO_BENCHMARK_HPP_ */
