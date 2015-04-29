/*
 * casbenchmark.cpp
 *
 *  Created on: Dec 23, 2014
 *      Author: masha
 */

#include "casbenchmark.hpp"

#include "cas.hpp"
#include <utility>
#include <atomic>
#include <chrono>
#include <iostream>

template<typename Value, bool builtin>
struct Runner
{
    static const bool usebuiltin = builtin;
    void operator()(std::size_t repetitions, const Value tag,
            const Value expected, const Value newVal)
    {
        do
        {
            Value tagVal = tag;
            lfds::atomic_cas(tagVal, expected, newVal);
        } while (--repetitions);
    }
};

template<typename Value>
struct Runner<Value, true>
{
    static const bool usebuiltin = true;

    void operator()(std::size_t repetitions, const Value tag,
            const Value expected, const Value newVal)
    {
        Value expectedVal = expected;
        do
        {
            std::atomic<Value> tagval(tag);
            std::atomic_compare_exchange_weak(&tagval, &expectedVal, newVal);
        } while (--repetitions);
    }
};

template<typename Value, bool usebuiltin, std::size_t = sizeof(Value)>
struct RunnerTraits
{
    static const char* name()
    {
        return nullptr;
    }

    static void run(std::size_t repetitions, const Value tag,
            const Value expected, const Value newVal)
    {

    }
};

template<typename Value>
struct RunnerTraits<Value, true, 8>
{
    static const char* name()
    {
        return "std::cas 8b";
    }
    static void run(std::size_t repetitions, const Value tag,
            const Value expected, const Value newVal)
    {
        Runner<Value, true>()(repetitions, tag, expected, newVal);
    }
};

template<typename Value>
struct RunnerTraits<Value, false, 8>
{
    static const char* name()
    {
        return "lfds::cas 8b";
    }
    static void run(std::size_t repetitions, const Value tag,
            const Value expected, const Value newVal)
    {
        Runner<Value, false>()(repetitions, tag, expected, newVal);
    }
};

template<typename Value>
struct RunnerTraits<Value, false, 16>
{
    static const char* name()
    {
        return "lfds::cas 16b";
    }
    static void run(std::size_t repetitions, const Value tag,
            const Value expected, const Value newVal)
    {
        Runner<Value, false>()(repetitions, tag, expected, newVal);
    }
};

template<typename Value, bool builtin>
struct Benchmark
{
    static const std::size_t repcount = 100000000;

    typedef std::chrono::high_resolution_clock clock_type;
    typedef std::chrono::duration<double> seconds_type;
    typedef clock_type::duration duration_type;
    typedef RunnerTraits<Value, builtin> runner_traits_type;

    void operator()(const Value tag, const Value exp, const Value val)
    {
        std::cout << "Benchmark for " << runner_traits_type::name() << std::endl
                << "---------------------------------" << std::endl;
        runtest(tag, exp, val);
        runtest(tag, tag, val);
        std::cout << "Benchmark for " << runner_traits_type::name() << std::endl
                << "---------------------------------" << std::endl
                << std::endl;
    }

    void runtest(const Value tag, const Value exp, const Value val)
    {
        seconds_type duration;

        std::cout << "running " << runner_traits_type::name()
                << (tag == exp ? " (success)" : " (fail)") << std::endl;
        auto start_point = clock_type::now();
        runner_traits_type::run(repcount, tag, exp, val);
        auto hiresDuration = clock_type::now() - start_point;
        duration = std::chrono::duration_cast < seconds_type > (hiresDuration);
        std::cout << "duration: " << duration.count() << " secs" << std::endl;
        std::cout << duration.count() * 1.0e9 / static_cast<double>(repcount)
                << " ns per operation" << std::endl;
    }
};

template<typename Value>
void doBenchmark(const Value tag, const Value exp, const Value val,
        bool builtin)
{
    if (builtin)
    {
        Benchmark<Value, true>()(tag, exp, val);
    }
    else
    {
        Benchmark<Value, false>()(tag, exp, val);
    }
}

void CASBenchMark()
{
    std::size_t tag8b = 0;
    std::size_t exp8b = 1;
    std::size_t val8b = 2;

    typedef std::pair<std::size_t, std::size_t> val16_type;

    val16_type tag16b(0, 0);
    val16_type exp16b(1, 1);
    val16_type val16b(2, 2);

    doBenchmark(tag8b, exp8b, val8b, true);
    doBenchmark(tag8b, exp8b, val8b, false);
    doBenchmark(tag16b, exp16b, val16b, false);
}
