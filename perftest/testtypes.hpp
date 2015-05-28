/*
 * testtypes.hpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTTYPES_HPP_
#define PERFTEST_TESTTYPES_HPP_

#include <set>
#include <map>
#include <cstdlib>
#include <string>

namespace xtomic
{
namespace perftest
{

typedef std::size_t id_type;
typedef std::set<id_type> ids_type;
typedef std::map<std::string, ids_type> group_type;
typedef std::map<std::string, group_type> groups_type;

}
}

#endif /* PERFTEST_TESTTYPES_HPP_ */
