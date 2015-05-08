/*
 * averagetime.cpp
 *
 *  Created on: Mar 26, 2015
 *      Author: masha
 */

#include "maptests.hpp"
#include <hash_map.hpp>
#include <hash_trie.hpp>
#include <maps/stdmaps.hpp>
#include "lfmaps.hpp"

#include <testaveragetime.hpp>
#include <testmaxtime.hpp>
#include <performancetest.hpp>
#include <testfactory.hpp>

namespace lfds
{
namespace perftest
{
namespace maps
{

typedef dummy_wrapper<long long> slow_int_type;

struct slow_hash
{
    typedef slow_int_type::type value_type;
    std::size_t operator()(const slow_int_type & val) const
    {
        return static_cast<std::size_t>(static_cast<value_type>(val));
    }
    ;
};

static const unsigned int NANOSECONDS_PER_SEC = static_cast<unsigned int>(1e9);
static const unsigned int MICROSECONDS_PER_SEC = static_cast<unsigned int>(1e6);

class void_registrar
{
public:
    void_registrar(const char*, const char*)
    {

    }
};

template<typename Map>
class ir_registrar
{
public:
    typedef Map map_type;
    typedef MaxInsertTester<map_type, true> tester_type;
    typedef MaximumOpTimeTest<tester_type, MICROSECONDS_PER_SEC> test_type;
    typedef PerfTestFactoryImpl<test_type> factory_type;

    ir_registrar(const char* group, const char* name) :
            m_factory(group, name, "maximum insert time (with initial reserve)",
                    "μs/op")
    {

    }

private:
    factory_type m_factory;
};

template<typename Map>
class mem_registrar
{
public:
    typedef typename Map::counted_map_type map_type;
    typedef AvgInsertTester<map_type, false> tester_type;
    typedef MemConsumptionTestImpl<tester_type, 1> mem_test_type;
    typedef PerfTestFactoryImpl<mem_test_type> factory_type;

    mem_registrar(const char* group, const char* name) :
            m_factory(group, name, "memory consumption", "Mb")
    {

    }

private:
    factory_type m_factory;
};

template<typename Map, bool = Map::RESERVE_IMPLEMENTED>
struct get_ir_registrar;

template<typename Map>
struct get_ir_registrar<Map, false>
{
    typedef void_registrar type;
};

template<typename Map>
struct get_ir_registrar<Map, true>
{
    typedef ir_registrar<Map> type;
};

template<typename Map, bool = Map::ALLOCATOR_IMPLEMENTED>
struct get_mem_registrar;

template<typename Map>
struct get_mem_registrar<Map, false>
{
    typedef void_registrar type;
};

template<typename Map>
struct get_mem_registrar<Map, true>
{
    typedef mem_registrar<Map> type;
};

template<typename Map>
class registrar
{
public:

    typedef Map map_type;
    typedef AvgInsertTester<map_type, true> avg_insert_tester_type;
    typedef MaxInsertTester<map_type, false> max_insert_tester_type;
    typedef AvgEraseTester<map_type> avg_erase_tester_type;
    typedef AvgFindTester<map_type> avg_find_tester_type;

    typedef AverageOpTimeTest<avg_insert_tester_type, NANOSECONDS_PER_SEC> avg_insert_test_type;
    typedef MaximumOpTimeTest<max_insert_tester_type, MICROSECONDS_PER_SEC> max_insert_test_type;
    typedef AverageOpTimeTest<avg_erase_tester_type, NANOSECONDS_PER_SEC> avg_erase_test_type;
    typedef AverageOpTimeTest<avg_find_tester_type, NANOSECONDS_PER_SEC> avg_find_test_type;

    typedef MtInsertNoiser<map_type> noiser_type;
    typedef MtAvgFindWorker<map_type> avg_worker_type;
    typedef MtMaxFindWorker<map_type> max_worker_type;

    typedef MtAvgAggregator avg_agg_type;
    typedef MtMaxAggregator max_agg_type;

    typedef MtTestImpl<map_type, noiser_type, avg_worker_type, avg_agg_type,
            NANOSECONDS_PER_SEC, TYPICAL_SIZE> mt_avg_test;
    typedef MtTestImpl<map_type, noiser_type, max_worker_type, max_agg_type,
            MICROSECONDS_PER_SEC, TYPICAL_SIZE> mt_max_test;

    typedef PerfTestFactoryImpl<avg_insert_test_type> avg_insert_factory_type;
    typedef PerfTestFactoryImpl<max_insert_test_type> max_insert_factory_type;
    typedef PerfTestFactoryImpl<avg_erase_test_type> avg_erase_factory_type;
    typedef PerfTestFactoryImpl<avg_find_test_type> avg_find_factory_type;
    typedef PerfTestFactoryImpl<mt_avg_test> mt_avg_find_factory_type;
    typedef PerfTestFactoryImpl<mt_max_test> mt_max_find_factory_type;
    typedef PerfTestFactoryImpl<mt_max_test> mem_tester_factory_type;

    typedef typename get_ir_registrar<Map>::type ir_registrar_type;
    typedef typename get_mem_registrar<Map>::type mem_registrar_type;

public:
    registrar(const char* group, const char* name) :
            m_avg_insert(group, name, "average insert time", "ns/op"),
            m_max_insert(group, name, "maximum insert time", "μs/op"),
            m_avg_erase(group, name, "average erase time", "ns/op"),
            m_avg_find(group, name, "average find time", "ns/op"),
            m_mt_avg_find(group, name, "mt average find time", "ns/op"),
            m_mt_max_find(group, name, "mt maximum find time", "μs/op"),
            m_ir_registrar(group, name),
            m_mem_registrar(group, name)
    {

    }
private:
    avg_insert_factory_type m_avg_insert;
    max_insert_factory_type m_max_insert;
    avg_erase_factory_type m_avg_erase;
    avg_find_factory_type m_avg_find;
    mt_avg_find_factory_type m_mt_avg_find;
    mt_max_find_factory_type m_mt_max_find;
    ir_registrar_type m_ir_registrar;
    mem_registrar_type m_mem_registrar;
};

namespace wise
{
typedef adapter::make_wise_hash_map<slow_int_type, slow_int_type,
        dummy_hash<slow_int_type::type> >::type generic_hash_map_type;
typedef adapter::make_wise_hash_map<long long, slow_int_type>::type ikey_hash_map_type;
typedef adapter::make_wise_hash_map<long long, long long>::type ival_hash_map_type;
typedef adapter::make_wise_hash_map<long long, int>::type ipair_hash_map_type;
}
namespace greedy
{
typedef adapter::make_greedy_hash_map<slow_int_type, slow_int_type,
        dummy_hash<slow_int_type::type> >::type generic_hash_map_type;
typedef adapter::make_greedy_hash_map<long long, slow_int_type>::type ikey_hash_map_type;
typedef adapter::make_greedy_hash_map<long long, long long>::type ival_hash_map_type;
typedef adapter::make_greedy_hash_map<long long, int>::type ipair_hash_map_type;
}
namespace simplified
{
typedef adapter::make_greedy_hash_map<long long, long long>::type hash_map_type;
}

typedef adapter::hash_trie<int, int, 16> hash_trie_type;
typedef adapter::stdmap<int, int, false> map_type;
typedef adapter::stdmap<int, int, true> unorderd_map_type;

namespace experimental
{
static registrar<hash_trie_type> r1("experimental", "hash_trie");
}

namespace wise
{
static registrar<generic_hash_map_type> r1("memory_model::wise", "hash_map<generic, generic>");
static registrar<ikey_hash_map_type> r2("memory_model::wise",
        "hash_map<int64_t, generic>");
static registrar<ival_hash_map_type> r3("memory_model::wise",
        "hash_map<int64_t, int64_t>");
static registrar<ipair_hash_map_type> r4("memory_model::wise",
        "hash_map<int64_t, int>");
}

namespace greedy
{
static registrar<generic_hash_map_type> r1("memory_model::greedy", "hash_map<generic, generic>");
static registrar<ikey_hash_map_type> r2("memory_model::greedy",
        "hash_map<int64_t, generic>");
static registrar<ival_hash_map_type> r3("memory_model::greedy",
        "hash_map<int64_t, int64_t>");
static registrar<ipair_hash_map_type> r4("memory_model::greedy",
        "hash_map<int64_t, int>");
}
namespace simplified
{
static registrar<hash_map_type> r1("memory_model::simplified", "hash_map<int64_t, int64_t>");
}
namespace reference
{
static registrar<map_type> r1("std", "map");
static registrar<unorderd_map_type> r2("std", "unordered_map");
}

}
}
}

