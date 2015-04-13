/*
 * testfilter.hpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTFILTER_HPP_
#define PERFTEST_TESTFILTER_HPP_

#include "testtypes.hpp"

namespace lfds
{
namespace perftest
{

class Filter
{
public:
    typedef std::set<std::string> strs_type;

    static ids_type getAllTests();

    static void byName(ids_type & ids, const strs_type & names, bool invertFilter);

    static void byGroup(ids_type & ids, const strs_type & groups, bool invertFilter);

    static void byFull(ids_type & ids, const strs_type & groups, bool invertFilter);
};

struct filter_by_name
{
    typedef Filter::strs_type strs_type;

    void operator()(ids_type & ids, const strs_type & filters, bool invertFilters) const
    {
        Filter::byName(ids, filters, invertFilters);
    }
};
struct filter_by_group
{
    typedef Filter::strs_type strs_type;

    void operator()(ids_type & ids, const strs_type & filters, bool invertFilters) const
    {
        Filter::byGroup(ids, filters, invertFilters);
    }
};
struct filter_by_full
{
    typedef Filter::strs_type strs_type;

    void operator()(ids_type & ids, const strs_type & filters, bool invertFilters) const
    {
        Filter::byFull(ids, filters, invertFilters);
    }
};

}
}


#endif /* PERFTEST_TESTFILTER_HPP_ */
