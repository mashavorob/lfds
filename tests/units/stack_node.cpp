/*
 * stack_node.cpp
 *
 *  Created on: Apr 6, 2015
 *      Author: masha
 */

#include <xtomic/impl/stack_node.hpp>

#include <gtest/gtest.h>



TEST(Stack_Node, recover)
{
    typedef lfds::stack_node<int> node_type;

    node_type node;
    node_type* pnode = &node;
    int* pdata = pnode->getData();
    node_type* prnode = node_type::recover(pdata);
    EXPECT_EQ(pnode, prnode);
}


