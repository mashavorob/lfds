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

typedef dummy_wrapper<int> slow_int_type;

struct slow_hash
{
    std::size_t operator()(const slow_int_type & val) const
    {
        return static_cast<std::size_t>(static_cast<int>(val));
    }
    ;
};

static const unsigned int NANOSECONDS_PER_SEC = static_cast<unsigned int>(1e9);
static const unsigned int MICROSECONDS_PER_SEC = static_cast<unsigned int>(1e6);

class noir_registrar
{
public:
    noir_registrar(const char*, const char*)
    {

    }
};

template<class Map>
class ir_registrar
{
public:
    typedef Map map_type;
    typedef MaxInsertTester<map_type, true> max_insert_ir_tester_type;
    typedef MaximumOpTimeTest<max_insert_ir_tester_type, MICROSECONDS_PER_SEC> max_insert_ir_test_type;
    typedef PerfTestFactoryImpl<max_insert_ir_test_type> max_insert_ir_factory_type;

    ir_registrar(const char* group, const char* name) :
            m_max_insert(group, name,
                    "maximum insert time (with initial reserve)",
                    getMaxInsertLabels(), "μs/op")
    {

    }

private:
    max_insert_ir_factory_type m_max_insert;

    static const char** getMaxInsertLabels()
    {
        static const char* labels[] =
        { "time", "max", "insert", "st", "initial-reserve", "nexus", nullptr };
        return labels;
    }
};

template<class Map, bool = Map::RESERVE_IMPLEMENTED>
struct get_ir_registrar;

template<class Map>
struct get_ir_registrar<Map, false>
{
    typedef noir_registrar type;
};

template<class Map>
struct get_ir_registrar<Map, true>
{
    typedef ir_registrar<Map> type;
};


template<class Map>
class registrar
{
public:

    typedef Map map_type;
    typedef AvgInsertTester<map_type> avg_insert_tester_type;
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

    typedef MemConsumptionTestImpl<max_insert_test_type, 1> mem_tester_type;

    typedef PerfTestFactoryImpl<avg_insert_test_type> avg_insert_factory_type;
    typedef PerfTestFactoryImpl<max_insert_test_type> max_insert_factory_type;
    typedef PerfTestFactoryImpl<avg_erase_test_type> avg_erase_factory_type;
    typedef PerfTestFactoryImpl<avg_find_test_type> avg_find_factory_type;
    typedef PerfTestFactoryImpl<mt_avg_test> mt_avg_find_factory_type;
    typedef PerfTestFactoryImpl<mt_max_test> mt_max_find_factory_type;
    typedef PerfTestFactoryImpl<mt_max_test> mem_tester_factory_type;

    typedef typename get_ir_registrar<Map>::type ir_registrar_type;

public:
    registrar(const char* group, const char* name) :
            m_avg_insert(group, name, "average insert time",
                    getAvgInsertLabels(), "ns/op"),
            m_max_insert(group, name, "maximum insert time",
                    getMaxInsertLabels(), "μs/op"),
            m_avg_erase(group, name, "average erase time", getAvgEraseLabels(),
                    "ns/op"),
            m_avg_find(group, name, "average find time", getAvgFindLabels(),
                    "ns/op"),
            m_mt_avg_find(group, name, "mt average find time",
                    getMtAvgFindLabels(), "ns/op"),
            m_mt_max_find(group, name, "mt maximum find time",
                    getMtMaxFindLabels(), "μs/op"),
            m_mem_test(group, name, "memory consumption", getMemLabels(), "Mb"),
            m_ir_registrar(group, name)
    {

    }
private:
    avg_insert_factory_type m_avg_insert;
    max_insert_factory_type m_max_insert;
    avg_erase_factory_type m_avg_erase;
    avg_find_factory_type m_avg_find;
    mt_avg_find_factory_type m_mt_avg_find;
    mt_max_find_factory_type m_mt_max_find;
    mem_tester_factory_type m_mem_test;
    ir_registrar_type m_ir_registrar;

    static const char** getAvgInsertLabels()
    {
        static const char* labels[] =
        { "time", "avg", "insert", "st", "nexus", nullptr };
        return labels;
    }
    static const char** getMaxInsertLabels()
    {
        static const char* labels[] =
        { "time", "max", "insert", "st", "no-reserve", "nexus", nullptr };
        return labels;
    }
    static const char** getAvgEraseLabels()
    {
        static const char* labels[] =
        { "time", "avg", "erase", "st", "nexus", nullptr };
        return labels;
    }
    static const char** getAvgFindLabels()
    {
        static const char* labels[] =
        { "time", "avg", "find", "st", "nexus", nullptr };
        return labels;
    }
    static const char** getMtAvgFindLabels()
    {
        static const char* labels[] =
        { "time", "avg", "find", "mt", "nexus", nullptr };
        return labels;
    }
    static const char** getMtMaxFindLabels()
    {
        static const char* labels[] =
        { "time", "max", "find", "mt", "nexus", nullptr };
        return labels;
    }
    static const char** getMemLabels()
    {
        static const char* labels[] =
        { "mem", "nexus", nullptr };
        return labels;
    }
};

typedef adapter::hash_map<slow_int_type, slow_int_type, dummy_hash<slow_int_type::type> > generic_hash_map_type;
typedef adapter::hash_map<int, slow_int_type> ikey_hash_map_type;
typedef adapter::hash_map<int, int> ipair_hash_map_type;
typedef adapter::hash_trie<int, int, 16> hash_trie16_type;
typedef adapter::hash_trie<int, int, 256> hash_trie256_type;
typedef adapter::stdmap<int, int, false> map_type;
typedef adapter::stdmap<int, int, true> unorderd_map_type;

static registrar<hash_trie16_type> r0_16("lock-free",
        "hash_trie (b-factor=16)");
static registrar<hash_trie256_type> r0_256("lock-free",
        "hash_trie (b-factor=256)");
static registrar<generic_hash_map_type> r1("lock-free", "hash_map");
static registrar<ikey_hash_map_type> r2("lock-free", "hash_map (integral key)");
static registrar<ipair_hash_map_type> r3("lock-free",
        "hash_map (integral key-value pair)");
static registrar<map_type> r4("std", "map");
static registrar<unorderd_map_type> r5("std", "unordered_map");

}
}
}

