/*
 * testfilter.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#include "testfilter.hpp"
#include "testlocator.hpp"
#include "wildcard.hpp"

#include <cppbasics.hpp>

#include <algorithm>

namespace lfds
{
namespace perftest
{

namespace
{

template<typename Accessor, bool byWildCard>
struct remove_items
{
    typedef Filter::strs_type strs_type;
    typedef typename get_str_match<byWildCard>::type match_type;

    typedef typename ids_type::iterator iterator;

    remove_items(bool invertFilter) :
            m_negFilter(invertFilter)
    {

    }

    void operator()(ids_type & ids, const strs_type & fields)
    {
        if (fields.empty())
        {
            return;
        }

        Accessor get;

        typename ids_type::iterator i = ids.begin();
        typename ids_type::iterator end = ids.end();
        typename strs_type::const_iterator fbeg = fields.begin();
        typename strs_type::const_iterator fend = fields.end();

        while (i != end)
        {
            const id_type id = *i;
            const std::string val = get(id);
            typename strs_type::const_iterator fpos = std::find_if(fbeg, fend,
                    match_type(val));
            bool res = (fpos != fend);
            if (m_negFilter)
            {
                res = !res;
            }

            if (!res)
            {
                ids.erase(i++);
            }
            else
            {
                ++i;
            }
        }
    }
private:
    bool m_negFilter;
};

}

ids_type Filter::getAllTests()
{
    ids_type result;
    const id_type end = PerfTestLocator::getInstance().getSize();

    for (id_type i = 0; i != end; ++i)
    {
        result.insert(i);
    }
    return result;
}

void Filter::byName(ids_type & ids, const strs_type & names, bool invertFilter)
{
    remove_items<PerfTestLocator::get_test_name, true> remove(invertFilter);

    remove(ids, names);
}

void Filter::byGroup(ids_type & ids,
                     const strs_type & groups,
                     bool invertFilter)
{
    remove_items<PerfTestLocator::get_test_group, true> remove(invertFilter);

    remove(ids, groups);
}

void Filter::byFull(ids_type & ids, const strs_type & groups, bool invertFilter)
{
    remove_items<PerfTestLocator::get_test_full, true> remove(invertFilter);

    remove(ids, groups);
}

}
}

