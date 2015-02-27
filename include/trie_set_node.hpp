/*
 * trie_set_node.hpp
 *
 *  Created on: Feb 17, 2015
 *      Author: masha
 */

#ifndef INCLUDE_TRIE_SET_NODE_HPP_
#define INCLUDE_TRIE_SET_NODE_HPP_

#include "ref_ptr.hpp"

#include <algorithm>
#include <climits>
#include <atomic>

namespace lfds
{
struct trie_set_node;

struct trie_set_node
{
    static constexpr int SIZE = UCHAR_MAX + 1;

    typedef std::atomic<bool> atomic_flag_type;

    typedef typename std::size_t size_type;
    typedef typename std::atomic<trie_set_node*> atomic_ptr_type;
    typedef typename std::atomic<int> atomic_counter_type;

    trie_set_node* m_parent;
    atomic_ptr_type m_children[SIZE];
    atomic_counter_type m_refCount;
    unsigned int m_char;
    atomic_flag_type m_terminal;

    trie_set_node() :
            m_parent(nullptr), m_refCount(0), m_char(0), m_terminal(false)
    {
        std::fill(m_children, m_children + SIZE, nullptr);
    }
};
}

#endif /* INCLUDE_TRIE_SET_NODE_HPP_ */
