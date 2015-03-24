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

    static void byName(ids_type & ids, const strs_type & names);

    static void byGroup(ids_type & ids, const strs_type & groups);

    static void byLabels(ids_type & ids, const strs_type & labels);
};

}
}


#endif /* PERFTEST_TESTFILTER_HPP_ */
