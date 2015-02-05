/*
 * buffers.cpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#include "gtest/gtest.h"

#include "fixed_buffer.hpp"
#include "dynamic_buffer.hpp"

TEST(fixed_buffer, allocfree)
{
    typedef lfds::fixed_buffer<int, std::allocator<int> > buffer_type;
    typedef buffer_type::node_type node_type;

    enum
    {
        code_1 = 12345,
    };

    buffer_type buff(1);

    node_type* node = buff.new_node(code_1);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(*node->data(), code_1);
    EXPECT_EQ(buff.new_node(), nullptr);
    buff.free_node(node);
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

    node_type* node1 = buff.new_node(code_1);
    EXPECT_NE(node1, nullptr);
    EXPECT_EQ(*node1->data(), code_1);

    node_type* node2 = buff.new_node(code_2);
    EXPECT_NE(node2, nullptr);
    EXPECT_EQ(*node2->data(), code_2);

    node_type* node3 = buff.new_node(code_3);
    EXPECT_NE(node3, nullptr);
    EXPECT_EQ(*(node3->data()), code_3);

    EXPECT_EQ(buff.new_node(), nullptr);

    buff.free_node(node1);

    node_type* node4 = buff.new_node(code_4);
    EXPECT_NE(node4, nullptr);
    EXPECT_EQ(*node4->data(), code_4);

    EXPECT_EQ(buff.new_node(), nullptr);

    buff.free_node(node2);
    buff.free_node(node3);
    buff.free_node(node4);

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

    node_type* node = buff.new_node(code_1);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(*node->data(), code_1);

    buff.free_node(node);
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

    node_type* node1 = buff.new_node(code_1);
    EXPECT_NE(node1, nullptr);
    EXPECT_EQ(*node1->data(), code_1);

    node_type* node2 = buff.new_node(code_2);
    EXPECT_NE(node2, nullptr);
    EXPECT_EQ(*node2->data(), code_2);

    buff.free_node(node1);

    node_type* node3 = buff.new_node(code_3);
    EXPECT_NE(node3, nullptr);
    EXPECT_EQ(*node3->data(), code_3);
    EXPECT_EQ(node1, node3); // node should be reused

    buff.free_node(node2);
    buff.free_node(node3);
}
