/*
 * trie_set.hpp
 *
 *  Created on: Feb 17, 2015
 *      Author: masha
 */

#ifndef INCLUDE_TRIE_SET_HPP_
#define INCLUDE_TRIE_SET_HPP_

#include "trie_set_node.hpp"
#include "pool_buffer.hpp"

#include <memory>
#include <atomic>
#include <algorithm>

namespace lfds
{

template<class Allocator = std::allocator<char> >
class trie_set
{
public:
    typedef trie_set<Allocator> this_type;

    typedef const char* key_type;
    typedef trie_set_node node_type;
    typedef Allocator allocator_type;
    typedef pool_buffer<node_type, allocator_type> buffer_type;
    typedef typename buffer_type::size_type size_type;
    typedef node_type::atomic_ptr_type node_ptr_type;

private:
    trie_set(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

public:
    trie_set(size_type initialCapacity = 0) :
            m_buffer(initialCapacity), m_head(), m_size(0)
    {
        m_head = m_buffer.allocate();
    }

    bool find(const char* str)
    {
        node_type* node = m_head;
        unsigned int c = static_cast<unsigned char>(*str);
        while (c && node)
        {
            node = node->m_children[c].load(std::memory_order_relaxed);
            c = static_cast<unsigned char>(*(++str));
        }

        return node && node->m_terminal.load(std::memory_order_relaxed);
    }

    bool erase(const char* str)
    {
        node_type* node = m_head;

        unsigned int c = static_cast<unsigned char>(*str);
        while (c && node)
        {
            node = node->m_children[c].load(std::memory_order_relaxed);
            c = static_cast<unsigned char>(*(++str));
        }

        if (!node)
        {
            return false;
        }

        bool result = node->m_terminal.load(std::memory_order_relaxed);

        if (result)
        {
            bool t = true;
            static constexpr bool f = false;
            result = node->m_terminal.compare_exchange_strong(t, f,
                    std::memory_order_release, std::memory_order_relaxed);

            if (result)
            {
                // release reference because the node is not a terminal any more
                node->m_refCount.fetch_sub(1, std::memory_order_acquire);
                m_size.fetch_sub(1, std::memory_order_relaxed);
            }
        }
        return result;
    }

    bool insert(const char* str)
    {
        node_type* new_node = nullptr;

        node_type* node = m_head;
        unsigned int c = static_cast<unsigned char>(*str);
        while (c && node)
        {
            node_ptr_type & link = node->m_children[c];
            node_type* next_node = link.load(std::memory_order_relaxed);
            if (!next_node)
            {
                if (!new_node)
                {
                    new_node = m_buffer.allocate();
                }
                new_node->m_parent = node;
                new_node->m_char = c;

                // extra reference to prevent deleting in concurrent operation
                bool result = link.compare_exchange_strong(next_node, new_node,
                        std::memory_order_relaxed, std::memory_order_relaxed);
                if (result)
                {
                    next_node = new_node;
                    new_node = nullptr;
                    ++node->m_refCount;
                }
                else
                {
                    // insertion in another concurent operation
                    next_node = link.load(std::memory_order_relaxed);
                }
            }
            node = next_node;
            c = static_cast<unsigned char>(*(++str));
        }

        if ( new_node )
        {
            m_buffer.deallocate(new_node);
        }

        if (node->m_terminal.load(std::memory_order_relaxed))
        {
            // the key is already here
            return false;
        }

        bool f = false;
        static constexpr bool t = true;
        if (!node->m_terminal.compare_exchange_strong(f, t,
                std::memory_order_release, std::memory_order_relaxed))
        {
            // the node has been updated in another concurrent operation
            return false;
        }
        // the node already has an extra reference so leave it unchanged
        m_size.fetch_add(1, std::memory_order_relaxed);

        // do not deref node because terminal node needs an extra reference
        return true;
    }

    size_type size() const
    {
        return m_size.load(std::memory_order_relaxed);
    }

private:
    buffer_type m_buffer;
    node_type* m_head;
    std::atomic<size_type> m_size;
};

}

#endif /* INCLUDE_TRIE_SET_HPP_ */
