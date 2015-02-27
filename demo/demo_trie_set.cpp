/*
 * demo_try_set.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: masha
 */

#include "set_wrapper.hpp"
#include "random_strings.hpp"

#include <queue.hpp>
#include <trie_set.hpp>
#include <bit_trie_set.hpp>
#include <hash_set.hpp>

#include <atomic>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <cassert>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <iostream>
#include <vector>
#include <limits>

static const unsigned TRIE_TEST_SIZE = 4000;
static const unsigned TRIE_TEST_NUM_THREADS = 2;
static const bool ALLOW_CONCURENT_MODIFICATIONS = true;

static const char* arbitrary_text =
        "An array is a series of elements of the same type placed in contiguous memory"
                " locations that can be individually referenced by adding an index to a unique"
                " identifier. That means that, for example, five values of type int can be "
                "declared as an array without having to declare 5 different variables (each with "
                "its own identifier). Instead, using an array, the five int values are stored in "
                "contiguous memory locations, and all five can be accessed using the same identifier, "
                "with the proper index. For example, an array containing 5 integer values of type "
                "int called foo could be represented as: where each blank panel represents an element"
                " of the array. In this case, these are values of type int. These elements are numbered "
                "from 0 to 4, being 0 the first and 4 the last; In C++, the first element in an array is "
                "always numbered with a zero (not a one), no matter its length. Like a regular variable,"
                " an array must be declared before it is used. A typical declaration for an array in C++ is: "
                "type name [elements]; where type is a valid type (such as int, float...), name "
                "is a valid identifier and the elements field (which is always enclosed in square "
                "brackets []), specifies the length of the array in terms of the number of elements. "
                "Therefore, the foo array, with five elements of type int, can be declared as:";

static std::vector<std::string> words;
static std::vector<std::string> words2;
static std::vector<const char*> wordKeys;

template<class Queue>
struct fill_queue
{
    typedef Queue queue_type;

    void operator()(queue_type & q)
    {
        for (const std::string & w : words)
        {
            const char* sz = w.c_str();
            q.push(sz);
        }
    }
};

template<class Set>
class BenchmarkBase
{
public:
    // wait free data queue to pass inserted data to deleter thread
    typedef lfds::queue<const char*, true, true, true> queue_type;
    typedef Set set_type;
    typedef typename set_type::key_type key_type;
    typedef fill_queue<queue_type> filler_type;
    typedef std::size_t size_type;

    const static unsigned int SIZE_HIGH = TRIE_TEST_SIZE;
    const static unsigned int SIZE_LOW = SIZE_HIGH * 17 / 20; // (85%=85/100=17/20)

    static void reset()
    {
        getStartFlag().store(false, std::memory_order_relaxed);
        getStopFlag().store(false, std::memory_order_relaxed);
    }
    static void start()
    {
        getStartFlag().store(true, std::memory_order_release);
    }
    static void stop()
    {
        getStopFlag().store(true, std::memory_order_release);
    }
protected:
    static queue_type & getErasedQueue()
    {
        static queue_type q(SIZE_HIGH);
        static std::once_flag flag;
        static filler_type filler;

        std::call_once(flag, filler, std::ref(q));

        return q;
    }
    static queue_type & getInsertedQueue()
    {
        static queue_type q(SIZE_HIGH);
        return q;
    }
    static bool isStarted()
    {
        return getStartFlag().load(std::memory_order_relaxed);
    }
    static bool isStopped()
    {
        return getStopFlag().load(std::memory_order_relaxed);
    }
private:
    static std::atomic_bool & getStartFlag()
    {
        static std::atomic_bool val(0);
        return val;
    }
    static std::atomic_bool & getStopFlag()
    {
        static std::atomic_bool val(0);
        return val;
    }
};

struct result_type
{
    typedef std::size_t size_type;

    size_type m_result;
};

template<class Set>
class Inserter: public BenchmarkBase<Set>, public result_type
{
public:
    typedef BenchmarkBase<Set> base_type;
    typedef typename base_type::set_type set_type;
    typedef typename base_type::queue_type queue_type;
    typedef typename result_type::size_type size_type;

    static const unsigned SIZE_HIGH = base_type::SIZE_HIGH;

    void operator()(set_type & m)
    {
        queue_type & erased = base_type::getErasedQueue();
        queue_type & inserted = base_type::getInsertedQueue();
        size_type count = 0;

        while (!base_type::isStopped())
        {
            const char* data;
            if (m.size() < SIZE_HIGH && erased.pop(data))
            {
                bool set_insert_res = m.insert(data);
                assert(set_insert_res);
                bool queue_insert_res = inserted.push(data);
                assert(queue_insert_res);
                ++count;
            }
        }
        m_result = count;
    }
};

template<class Set>
class Deleter: public BenchmarkBase<Set>, public result_type
{
public:
    typedef BenchmarkBase<Set> base_type;
    typedef typename base_type::set_type set_type;
    typedef typename base_type::queue_type queue_type;
    typedef typename result_type::size_type size_type;

    static const unsigned SIZE_LOW = base_type::SIZE_LOW;

    void operator()(set_type & m)
    {
        queue_type & erased = base_type::getErasedQueue();
        queue_type & inserted = base_type::getInsertedQueue();
        size_type count = 0;
        size_type count2 = 0;
        std::string trap = "all declaration float are";

        while (!base_type::isStarted())
            ;
        while (!base_type::isStopped())
        {
            const char* data;
            if (m.size() > SIZE_LOW && inserted.pop(data))
            {
                bool trap_triggered = false;
                if ( trap == data )
                {
                    trap_triggered = true;
                    ++count2;
                }
                bool set_find_res = m.find(data);
                bool set_erase_res = m.erase(data);
                assert(set_erase_res);
                bool queue_erase_res = erased.push(data);
                assert(queue_erase_res);
                ++count;
            }
        }
        m_result = count;
    }
};

template<class Set, class Key>
class Finder: public BenchmarkBase<Set>, public result_type
{
public:
    typedef BenchmarkBase<Set> base_type;
    typedef Key key_type;
    typedef std::vector<key_type> collection_type;
    typedef typename base_type::set_type set_type;
    typedef typename collection_type::const_iterator const_iterator;
    typedef typename result_type::size_type size_type;

    static const size_type SIZE = 10000;
public:

    Finder()
    {
        m_keys.assign(wordKeys.begin(), wordKeys.end());
    }

    void operator()(set_type & m)
    {

        size_type count = 0;
        const_iterator beg = m_keys.begin();
        const_iterator end = m_keys.end();
        const_iterator iter = beg;

        while (!base_type::isStarted())
            ;
        while (!base_type::isStopped())
        {
            const key_type & key = *iter;
            m.find(key);
            ++count;
            if (++iter == end)
            {
                iter = beg;
            }
        }
        m_result = count;
    }
private:
    collection_type m_keys;
};

template<class Set, class Key = typename Set::key_type>
class Benchmark: public BenchmarkBase<Set>
{
public:
    typedef BenchmarkBase<Set> base_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::set_type set_type;
    typedef Deleter<Set> deleter_type;
    typedef Inserter<Set> inserter_type;
    typedef Finder<Set, Key> finder_type;

    static const int DURATION_SECONDS = 10;

public:
    double operator()(size_type thread_count) const
    {
        typedef std::chrono::duration<double> seconds_type;
        typedef std::chrono::high_resolution_clock clock_type;
        typedef clock_type::duration duration_type;
        typedef clock_type::time_point time_point_type;

        deleter_type deleter;
        inserter_type inserter;
        std::vector<finder_type> finders(thread_count, finder_type());

        base_type::reset();

        set_type m;

        std::vector<std::thread> threads;

        if (ALLOW_CONCURENT_MODIFICATIONS)
        {
            threads.push_back(std::thread(std::ref(deleter), std::ref(m)));
        }
        threads.push_back(std::thread(std::ref(inserter), std::ref(m)));

        for (finder_type & finder : finders)
        {
            threads.push_back(std::thread(std::ref(finder), std::ref(m)));
        }

        const seconds_type duration(DURATION_SECONDS);

        const seconds_type quiet(1); // quiet time 1 sec just for fun
        std::this_thread::sleep_for(quiet);

        base_type::start();

        time_point_type start_point = clock_type::now();
        std::this_thread::sleep_for(duration);
        base_type::stop();
        time_point_type end_point = clock_type::now();

        // stop all threads
        for (auto & t : threads)
        {
            t.join();
        }

        size_type count = 0;
        for (finder_type & finder : finders)
        {
            count += finder.m_result;
        }

        // return average time
        seconds_type duration_seconds =
                std::chrono::duration_cast<seconds_type>(
                        end_point - start_point);

        double res = static_cast<double>(duration_seconds.count())
                / static_cast<double>(count);
        res = res / static_cast<double>(thread_count);
        return res;
    }
};

template<class BM>
double DoBenchmark(BM & bm, const std::size_t num_threads, const char* title,
        const double ref_perf = 0.)
{
    std::cout << title;
    std::cout.flush();
    double perf = bm(num_threads);
    std::cout << static_cast<int>(perf * 1e9) << " ns per find";
    if (ref_perf > std::numeric_limits<double>::epsilon())
    {
        std::cout << " (" << static_cast<int>(perf / ref_perf)
                << " times slower)";
    }
    std::cout << std::endl;
    return perf;
}

void DemoTrieSet()
{
    if (words.empty())
    {
        MakeUniqueStrings(arbitrary_text, TRIE_TEST_SIZE, words);
        MakeUniqueStrings(arbitrary_text, TRIE_TEST_SIZE * 2, words2);

        srand(time(NULL));
        wordKeys.resize(words2.size());
        int size = static_cast<int>(words2.size());
        for (int i = 0; i < size; ++i)
        {
            int index = i + (rand() % (size - i));
            wordKeys[i] = words2[index].c_str();
        }
    }

    typedef lfds::trie_set<> lf_trie_set;
    typedef lfds::bit_trie_set<> lf_bit_trie_set;
    typedef lfds::hash_set<std::string> lf_hash_set;
    typedef std_set_wrapper<std::string> std_set_type;
    typedef std_unordered_set_wrapper<std::string> std_unordered_set_type;

    typedef Benchmark<lf_trie_set> lf_trie_benchmark_type;
    typedef Benchmark<lf_bit_trie_set> lf_bit_trie_benchmark_type;
    typedef Benchmark<lf_hash_set> lf_hash_set_benchmark_type;
    typedef Benchmark<lf_hash_set, const char*> lf_hash_set_c_benchmark_type;
    typedef Benchmark<std_set_type> std_set_benchmark_type;
    typedef Benchmark<std_unordered_set_type> std_unorderd_benchmark_type;
    typedef Benchmark<std_set_type, const char*> std_set_benchmark_type2;
    typedef Benchmark<std_unordered_set_type, const char*> std_unorderd_benchmark_type2;

    const std::size_t num_threads = TRIE_TEST_NUM_THREADS;

    lf_trie_benchmark_type bm_trie_set;
    lf_bit_trie_benchmark_type bm_bit_trie_set;
    lf_hash_set_benchmark_type bm0;
    lf_hash_set_c_benchmark_type bm1;
    std_set_benchmark_type bm2;
    std_set_benchmark_type2 bm3;
    std_unorderd_benchmark_type bm4;
    std_unorderd_benchmark_type2 bm5;

    const char* trie_title     = "                   benchmark lock free trie: ";
    const char* bit_trie_title = "               benchmark lock free bit trie: ";
    const char* hset_title     = "               benchmark lock free hash set: ";
    const char* hset_title_c   = "benchmark lock free hash set (char* as key): ";
    const char* set_title      = "                          benchmark std set: ";
    const char* uset_title     = "                benchmark std unordered_set: ";
    const char* set_title_c    = "           benchmark std set (char* as key): ";
    const char* uset_title_c   = " benchmark std unordered_set (char* as key): ";

    const double ref_perf = DoBenchmark(bm_trie_set, num_threads, trie_title);
    DoBenchmark(bm_bit_trie_set, num_threads, bit_trie_title, ref_perf);
    DoBenchmark(bm0, num_threads, hset_title, ref_perf);
    DoBenchmark(bm1, num_threads, hset_title_c, ref_perf);
    DoBenchmark(bm2, num_threads, set_title, ref_perf);
    DoBenchmark(bm3, num_threads, set_title_c, ref_perf);
    DoBenchmark(bm4, num_threads, uset_title, ref_perf);
    DoBenchmark(bm5, num_threads, uset_title_c, ref_perf);
}

