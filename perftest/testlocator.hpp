/*
 * testlocator.hpp
 *
 *  Created on: Mar 23, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTLOCATOR_HPP_
#define PERFTEST_TESTLOCATOR_HPP_

#include "performancetest.hpp"
#include "testtypes.hpp"

namespace xtomic
{
namespace perftest
{

class IPerfTestFactory;

struct PerfTestInfo
{
    const char* m_group;
    const char* m_name;
    const char* m_param;
    const char* m_units;
    IPerfTestFactory* m_factory;

    id_type m_id;
    PerfTestInfo* m_link;
};

class PerfTestLocator
{
private:
    PerfTestLocator();
public:

    typedef std::size_t size_type;

    struct get_test_name
    {
        std::string operator()(const id_type id)
        {
            return PerfTestLocator::getInstance().getTestName(id);
        }
    };

    struct get_test_group
    {
        std::string operator()(const id_type id)
        {
            return PerfTestLocator::getInstance().getTestGroup(id);
        }
    };

    struct get_test_param
    {
        std::string operator()(const id_type id)
        {
            return PerfTestLocator::getInstance().getTestParam(id);
        }
    };

    struct get_test_full
    {
        std::string operator()(const id_type id)
        {
            const PerfTestLocator & locator = PerfTestLocator::getInstance();

            return std::string(locator.getTestGroup(id)) + "."
                    + locator.getTestName(id) + "." + locator.getTestParam(id);
        }
    };

public:
    static const PerfTestLocator& getInstance();

    static void registerTest(PerfTestInfo* info);

    static size_type getSize()
    {
        return m_link ? m_link->m_id + 1 : 0;
    }

    const char* getTestName(const id_type id) const
    {
        return at(id)->m_name;
    }
    const char* getTestParam(const id_type id) const
    {
        return at(id)->m_param;
    }
    const char* getTestGroup(const id_type id) const
    {
        return at(id)->m_group;
    }
    const char* getTestUnits(const id_type id) const
    {
        return at(id)->m_units;
    }
    void getTest(const id_type id, PerformanceTest & test) const;

    const PerfTestInfo* at(const id_type id) const;
private:
    static PerfTestInfo* m_link;
};

}
}

#endif /* PERFTEST_TESTLOCATOR_HPP_ */
