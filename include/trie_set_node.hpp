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

    typedef std::size_t size_type;
    typedef std::atomic<size_type> atomic_counter_type;
    typedef ref_ptr<trie_set_node> ref_ptr_type;

    trie_set_node* m_parent;
    ref_ptr_type m_children[SIZE];
    atomic_counter_type m_refCount;

    atomic_flag_type m_terminal;

    trie_set_node() :
            m_parent(nullptr), m_refCount(0), m_terminal(false)
    {
    }
};
}

#endif /* INCLUDE_TRIE_SET_NODE_HPP_ */
