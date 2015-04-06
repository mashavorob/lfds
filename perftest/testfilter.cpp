/*
 * testfilter.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#include "testfilter.hpp"
#include "testlocator.hpp"

#include <cppbasics.hpp>

#include <algorithm>

namespace lfds
{
namespace perftest
{

namespace {

template<class Accessor>
struct remove_items
{
    typedef Filter::strs_type strs_type;

    typedef typename ids_type::iterator iterator;

    void operator()(ids_type & ids, const strs_type & fields)
    {
        if ( fields.empty() )
        {
            return;
        }

        Accessor get;
        typename ids_type::iterator i = ids.begin();
        typename ids_type::iterator end = ids.end();
        typename strs_type::const_iterator fend = fields.end();

        while ( i != end )
        {
            const id_type id = *i;
            const char* val = get(id);
            if ( fields.find(val) == fend )
            {
                ids.erase(i++);
            }
            else
            {
                ++i;
            }
        }
    }
};

}

ids_type Filter::getAllTests()
{
    ids_type result;
    const id_type end = PerfTestLocator::getInstance().getSize();

    for ( id_type i = 0; i != end; ++i )
    {
        result.insert(i);
    }
    return result;
}

void Filter::byName(ids_type & ids, const strs_type & names)
{
    remove_items<PerfTestLocator::get_test_name> remove;

    remove(ids, names);
}

void Filter::byGroup(ids_type & ids, const strs_type & groups)
{
    remove_items<PerfTestLocator::get_test_group> remove;

    remove(ids, groups);
}


void Filter::byLabels(ids_type & ids, const strs_type & labels)
{
    if ( labels.empty() )
    {
        return;
    }

    const PerfTestLocator &locator = PerfTestLocator::getInstance();
    ids_type::iterator iter = ids.begin();
    ids_type::iterator end = ids.end();
    strs_type::const_iterator lend = labels.end();

    while ( iter != end )
    {
        const id_type id = *iter;
        const char** ll = locator.getTestLabels(id);

        bool found = false;
        for ( int i = 0; !found && ll[i]; ++ll )
        {
            const char* label = ll[i];
            found = labels.find(label) != lend;
        }
        if ( !found )
        {
            ids.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
}

}
}

