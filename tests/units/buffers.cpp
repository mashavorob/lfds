/*
 * buffers.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#include "gtest/gtest.h"

#include <xtomic/impl/fixed_buffer.hpp>
#include <xtomic/impl/dynamic_buffer.hpp>

TEST(fixed_buffer, allocfree)
{
    typedef lfds::fixed_buffer<int, std::allocator<int> > buffer_type;
    typedef buffer_type::node_type node_type;

    enum
    {
        code_1 = 12345,
    };

    buffer_type buff(1);

    node_type* node = buff.newNode(code_1);
    EXPECT_NE(node, static_cast<node_type*>(0));
    EXPECT_EQ(*node->getData(), code_1);
    EXPECT_EQ(buff.newNode(0), static_cast<node_type*>(0));
    buff.freeNode(node);
}

TEST(fixed_buffer, allocfree3)
{
    typedef lfds::fixed_buffer<int, std::allocator<int> > buffer_type;
    typedef buffer_type::node_type node_type;

    enum
    {
        code_1 = 12345, code_2 = 12346, code_3 = 12347, code_4 = 12348,
    };

    buffer_type buff(3);

    node_type* node1 = buff.newNode(code_1);
    EXPECT_NE(node1, static_cast<node_type*>(0));
    EXPECT_EQ(*node1->getData(), code_1);

    node_type* node2 = buff.newNode(code_2);
    EXPECT_NE(node2, static_cast<node_type*>(0));
    EXPECT_EQ(*node2->getData(), code_2);

    node_type* node3 = buff.newNode(code_3);
    EXPECT_NE(node3, static_cast<node_type*>(0));
    EXPECT_EQ(*(node3->getData()), code_3);

    EXPECT_EQ(buff.newNode(0), static_cast<node_type*>(0));

    buff.freeNode(node1);

    node_type* node4 = buff.newNode(code_4);
    EXPECT_NE(node4, static_cast<node_type*>(0));
    EXPECT_EQ(*node4->getData(), code_4);

    EXPECT_EQ(buff.newNode(0), static_cast<node_type*>(0));

    buff.freeNode(node2);
    buff.freeNode(node3);
    buff.freeNode(node4);

}

TEST(dynamic_buffer, allocfree)
{
    typedef lfds::dynamic_buffer<int, std::allocator<int> > buffer_type;
    typedef buffer_type::node_type node_type;

    enum
    {
        code_1 = 12345,
    };

    buffer_type buff(1);

    node_type* node = buff.newNode(code_1);
    EXPECT_NE(node, static_cast<node_type*>(0));
    EXPECT_EQ(*node->getData(), code_1);

    buff.freeNode(node);
}

TEST(dynamic_buffer, reuse_node)
{
    typedef lfds::dynamic_buffer<int, std::allocator<int> > buffer_type;
    typedef buffer_type::node_type node_type;

    enum
    {
        code_1 = 12345, code_2 = 54321, code_3 = 67890,
    };

    buffer_type buff(1);

    node_type* node1 = buff.newNode(code_1);
    EXPECT_NE(node1, static_cast<node_type*>(0));
    EXPECT_EQ(*node1->getData(), code_1);

    node_type* node2 = buff.newNode(code_2);
    EXPECT_NE(node2, static_cast<node_type*>(0));
    EXPECT_EQ(*node2->getData(), code_2);

    buff.freeNode(node1);

    node_type* node3 = buff.newNode(code_3);
    EXPECT_NE(node3, static_cast<node_type*>(0));
    EXPECT_EQ(*node3->getData(), code_3);
    EXPECT_EQ(node1, node3); // node should be reused

    buff.freeNode(node2);
    buff.freeNode(node3);
}
