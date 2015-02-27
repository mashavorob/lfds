/*
 * bit_trie_set_node.hpp
 *
 *  Created on: Feb 19, 2015
 *      Author: masha
 */

#ifndef INCLUDE_BIT_TRIE_SET_NODE_HPP_
#define INCLUDE_BIT_TRIE_SET_NODE_HPP_

#include <atomic>
#include <algorithm>

namespace lfds
{
struct bit_trie_set_node
{
    static constexpr int SIZE = 2;

    typedef std::atomic<bit_trie_set_node*> atomic_ptr;
    typedef std::atomic<bool> atomic_flag;

    atomic_ptr m_next[SIZE];
    atomic_flag m_terminal;

    bit_trie_set_node() :
            m_terminal(false)
    {
        std::for_each(m_next, m_next + SIZE, [](atomic_ptr& val)
        {   val.store(nullptr, std::memory_order_relaxed);});
    }
};
}

#endif /* INCLUDE_BIT_TRIE_SET_NODE_HPP_ */
