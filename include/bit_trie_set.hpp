/*
 * bit_trie_set.hpp
 *
 *  Created on: Feb 19, 2015
 *      Author: masha
 */

#ifndef INCLUDE_BIT_TRIE_SET_HPP_
#define INCLUDE_BIT_TRIE_SET_HPP_

#include "bit_string_table.hpp"
#include "bit_trie_set_node.hpp"
#include "pool_buffer.hpp"

#include <memory>
#include <atomic>
#include <algorithm>

namespace lfds
{

template<class Allocator = std::allocator<char> >
class bit_trie_set
{
public:
    typedef bit_trie_set<Allocator> this_type;

    typedef const char* key_type;
    typedef bit_trie_set_node node_type;
    typedef Allocator allocator_type;
    typedef pool_buffer<node_type, allocator_type> buffer_type;
    typedef typename buffer_type::size_type size_type;
    typedef typename bit_string_table::row_type bit_string_type;
    typedef typename bit_string_table::index_type bit_index_type;

    static constexpr int BIT_STRING_SIZE = bit_string_table::ROW_SIZE;

private:
    bit_trie_set(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

public:
    bit_trie_set(size_type initialCapacity = 0) :
            m_buffer(initialCapacity), m_head(), m_size(0)
    {
        node_type* node = m_buffer.allocate();
        m_head.store(node, std::memory_order_relaxed);
    }

    bool find(const char* str) const
    {
        const node_type* node = m_head.load(std::memory_order_relaxed);
        const bit_string_table & table = bit_string_table::instance();
        while (node && *str)
        {
            const bit_index_type* bit_row = table[*str];
            for (int i = 0; (i < BIT_STRING_SIZE) && node; ++i)
            {
                const bit_index_type bit = bit_row[i];
                node = node->m_next[bit].load(std::memory_order_relaxed);
            }
            ++str;
        }
        return node && node->m_terminal.load(std::memory_order_relaxed);;
    }

    bool erase(const char* str)
    {
        node_type* node = m_head.load(std::memory_order_relaxed);
        const bit_string_table & table = bit_string_table::instance();
        while (node && *str)
        {
            const bit_index_type* bit_row = table[*str];
            for (int i = 0; (i < BIT_STRING_SIZE) && node; ++i)
            {
                const bit_index_type bit = bit_row[i];
                node = node->m_next[bit].load(std::memory_order_relaxed);
            }
            ++str;
        }

        if (!node)
        {
            return false;
        }
        bool t = true;
        static constexpr bool f = false;
        if (!node->m_terminal.compare_exchange_strong(t, f,
                std::memory_order_release, std::memory_order_relaxed))
        {
            return false;
        }
        m_size.fetch_sub(1, std::memory_order_relaxed);
        return true;
    }

    bool insert(const char* str)
    {
        node_type* new_node = nullptr;
        node_type* node = m_head.load(std::memory_order_relaxed);
        const bit_string_table & table = bit_string_table::instance();
        while (*str)
        {
            const bit_index_type* bit_row = table[*str];
            for (int i = 0; i < BIT_STRING_SIZE; ++i)
            {
                const bit_index_type bit = bit_row[i];
                node_type* next = node->m_next[bit].load(std::memory_order_relaxed);
                if (!next)
                {
                    if (!new_node)
                    {
                        new_node = m_buffer.allocate();
                    }
                    if (node->m_next[bit].compare_exchange_strong(next, new_node,
                            std::memory_order_relaxed, std::memory_order_relaxed))
                    {
                        next = new_node;
                        new_node = nullptr;
                    }
                    else
                    {
                        // another concurrent insertion allocated the node
                        next = node->m_next[bit].load(std::memory_order_relaxed);
                    }
                }
                node = next;
                assert(node);
            }
            ++str;
        }
        if (new_node)
        {
            m_buffer.deallocate(new_node);
        }
        if (node->m_terminal.load(std::memory_order_relaxed))
        {
            // the string is already here
            return false;
        }
        bool f = false;
        static constexpr bool t = true;
        if (!node->m_terminal.compare_exchange_strong(f, t,
                std::memory_order_release, std::memory_order_relaxed))
        {
            return false;
        }

        m_size.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    size_type size() const
    {
        return m_size.load(std::memory_order_relaxed);
    }

private:
    buffer_type m_buffer;
    std::atomic<node_type*> m_head;
    std::atomic<size_type> m_size;
};

}

#endif /* INCLUDE_BIT_TRIE_SET_HPP_ */
