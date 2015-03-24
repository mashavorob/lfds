/*
 * testtree.hpp
 *
 *  Created on: Mar 24, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTTREE_HPP_
#define PERFTEST_TESTTREE_HPP_

#include "testtypes.hpp"

namespace lfds
{
namespace perftest
{

class TestTree
{
public:

    TestTree(const ids_type & plainSet);

    const groups_type& get() const
    {
        return m_groups;
    }
private:
    groups_type m_groups;
};

}
}


#endif /* PERFTEST_TESTTREE_HPP_ */
