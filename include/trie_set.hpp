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
    typedef typename node_type::ref_ptr_type node_ptr_type;
    typedef typename node_ptr_type::atomic_ptr_type atomic_ptr_type;

private:
    trie_set(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

public:
    trie_set(size_type initialCapacity = 0) :
            m_buffer(initialCapacity), m_head(), m_size(0)
    {
        m_head = m_buffer.allocate();
        ++m_head->m_refCount;
    }

    bool find(const char* str)
    {
        node_type* node = m_head;
        ++node->m_refCount;
        unsigned int c = static_cast<unsigned char>(*str);
        while (c && node)
        {
            node_ptr_type* link = &node->m_children[c];
            node_type* next = acquireNode(link->m_ptr);
            --node->m_refCount;
            node = next;
            c = static_cast<unsigned char>(*(++str));
        }

        bool result = false;
        if (node)
        {
            result = node->m_terminal.load(std::memory_order_relaxed);
            --node->m_refCount;
        }

        return result;
    }

    bool erase(const char* str)
    {
        node_type* node = m_head;
        ++node->m_refCount;
        unsigned int c = static_cast<unsigned char>(*str);
        while (c && node)
        {
            node_ptr_type* link = &node->m_children[c];
            node_type* next = acquireNode(link->m_ptr);
            --node->m_refCount;
            node = next;
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
                unlinkNode(str, node);
                --m_size;
            }
        }
        if (!result)
        {
            --node->m_refCount;
        }
        return result;
    }

    bool insert(const char* str)
    {
        node_type* new_node = nullptr;
        node_type* node = m_head;
        node_ptr_type dummy;
        node_ptr_type* nodeLink = &dummy;
        ++node->m_refCount;
        unsigned int c = static_cast<unsigned char>(*str);
        while (c && node)
        {
            node_ptr_type* nextLink = &node->m_children[c];
            ++nextLink->m_refCount;
            node_type* next = acquireNode(nextLink->m_ptr);

            if (!next)
            {
                --nextLink->m_refCount;
                if (!new_node)
                {
                    try
                    {
                        new_node = m_buffer.allocate();
                    } catch (...)
                    {
                        --node->m_refCount;
                        throw;
                    }
                    new_node->m_refCount.store(1, std::memory_order_relaxed);
                }
                new_node->m_parent = node;

                node_ptr_type null_ptr =
                { nullptr, 0 };
                node_ptr_type new_ptr =
                { new_node, 1 };
                bool result = nextLink->atomic_cas(null_ptr, new_ptr);
                if (result)
                {
                    ++nodeLink->m_refCount;
                    next = new_node;
                    new_node = nullptr;
                }
                else
                {
                    // there was no luck so try all over again
                    continue;
                }
            }
            --nodeLink->m_refCount;
            --node->m_refCount;
            node = next;
            nodeLink = nextLink;
            c = static_cast<unsigned char>(*(++str));
        }

        if (new_node)
        {
            m_buffer.deallocate(new_node);
        }

        bool result = !node->m_terminal.load(std::memory_order_relaxed);
        if (result)
        {
            bool f = false;
            static constexpr bool t = true;
            result = node->m_terminal.compare_exchange_strong(f, t,
                    std::memory_order_release, std::memory_order_relaxed);
            if (result)
            {
                ++nodeLink->m_refCount;
                ++m_size;
            }
        }
        // there is a possibility that some concurrent erase is taking place so the last reference
        // must be released by means of unlinkNode
        unlinkNode(str, node);
        return result;
    }

    size_type size() const
    {
        return m_size.load(std::memory_order_relaxed);
    }
private:
    node_type* acquireNode(atomic_ptr_type& ptr)
    {
        node_type* node = ptr.load(std::memory_order_relaxed);

        // validate the node, zero is always valid
        while (node)
        {
            ++node->m_refCount;
            node_type* node2 = ptr.load(std::memory_order_relaxed);
            if (node2 == node)
            {
                break;
            }
            --node->m_refCount;
            node = ptr.load(std::memory_order_relaxed);
        }
        return node;
    }
    void unlinkNode(const char* str, node_type* node)
    {
        while (node != m_head)
        {
            unsigned char c = static_cast<unsigned char>(*(--str));

            node_type* parent = node->m_parent;
            node_ptr_type& link = parent->m_children[c];

            if (--link.m_refCount)
            {
                --node->m_refCount;
                return;
            }

            node_ptr_type expected_ptr =
            { node, 0 };
            node_ptr_type null_ptr =
            { nullptr, 0 };

            if (!link.atomic_cas(expected_ptr, null_ptr))
            {
                --node->m_refCount;
                return;
            }

            // wait for final release
            --node->m_refCount;
            while (node->m_refCount.load(std::memory_order_relaxed) > 0)
                ;

            m_buffer.deallocate(node);

            ++parent->m_refCount;
            node = parent;
        }
        --node->m_refCount;
    }
private:
    buffer_type m_buffer;
    node_type* m_head;
    std::atomic<size_type> m_size;
};

}

#endif /* INCLUDE_TRIE_SET_HPP_ */
