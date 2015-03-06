/*
 * trie_buffer.hpp
 *
 *  Created on: Feb 17, 2015
 *      Author: masha
 */

#ifndef INCLUDE_POOL_BUFFER_HPP_
#define INCLUDE_POOL_BUFFER_HPP_

#include "abaptr.hpp"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <algorithm>

namespace lfds
{
namespace
{
template<class T>
struct pool_node
{
    typedef pool_node<T> this_type;
    this_type* m_next;
    T m_data;

    static this_type* recover(T* p)
    {
        static constexpr int offset = offsetof(this_type, m_data);
        return reinterpret_cast<this_type*>(reinterpret_cast<char*>(p) - offset);
    }
};

template<class T, class Allocator>
struct pool_chunk
{
    typedef pool_chunk<T, Allocator> this_type;
    typedef pool_node<T> node_type;
    typedef std::size_t size_type;
    typedef T value_type;
    typedef typename Allocator::template rebind<value_type>::other allocator_type;

    this_type* m_next;
    size_type m_size;
    node_type m_data[1];  //-- trailing array of nodes

    pool_chunk(size_type size) :
            m_next(), m_size(size)
    {
    }

    void construct(allocator_type & a)
    {
        for (size_type i = 0; i < m_size; ++i)
        {
            a.construct(&m_data[i]);
        }
    }
    void destroy(allocator_type & a)
    {
        for (size_type i = 0; i < m_size; ++i)
        {
            a.destroy(&m_data[i]);
        }
    }

    static size_type getByteSize(size_type chunkSize)
    {
        return sizeof(this_type) + sizeof(node_type) * (chunkSize - 1);
    }
};

template<class T>
inline void atomic_push(std::atomic<T*> & list, T* head, T* tail)
{
    bool success;
    do
    {
        tail->m_next = list.load(std::memory_order_relaxed);
        success = list.compare_exchange_weak(tail->m_next, tail->m_next,
                std::memory_order_relaxed, std::memory_order_relaxed);
    } while (!success);
}

template<class T>
inline void atomic_push(std::atomic<T*> & list, T* val)
{
    atomic_push(list, val, val);
}

template<class T>
inline void atomic_push(volatile abaptr<T> & list, T* head, T* tail)
{
    bool success;
    do
    {
        abaptr<T> old_ptr = list;
        tail->m_next = old_ptr.m_ptr;
        abaptr<T> new_ptr =
        { head, old_ptr.m_counter + 1 };
        success = list.atomic_cas(old_ptr, new_ptr);
    } while (!success);
}

template<class T>
inline void atomic_push(volatile abaptr<T> & list, T* val)
{
    atomic_push(list, val, val);
}

}

template<class T, class Allocator>
class pool_buffer
{
public:
    typedef pool_buffer<T, Allocator> this_type;
    typedef T value_type;

    typedef pool_chunk<value_type, Allocator> chunk_type;

    typedef typename Allocator::template rebind<char>::other byte_allocator_type;
    typedef typename Allocator::template rebind<chunk_type>::other chunk_allocator_type;
    typedef typename chunk_type::node_type node_type;
    typedef typename chunk_type::allocator_type node_allocator_type;
    typedef std::size_t size_type;

    static constexpr unsigned int MIN_SIZE = 32;

private:
    pool_buffer(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

public:
    pool_buffer(size_type initialCapacity) :
            m_reserved(0), m_freeNodes(nullptr), m_chunks(nullptr), m_byte_allocator(), m_node_allocator(), m_chunk_allocator()
    {
        m_freeNodes.m_ptr = reserve(initialCapacity).first;
    }
    ~pool_buffer()
    {
        chunk_type* chunk = m_chunks.load(std::memory_order_relaxed);
        while (chunk)
        {
            chunk_type* next = chunk->m_next;
            deallocate_chunk(chunk);
            chunk = next;
        }
    }

    T* allocate()
    {
        // pop node
        bool success = false;
        node_type* node;

        do
        {
            abaptr<node_type> old_val = m_freeNodes;
            node = old_val.m_ptr;
            if (!node)
            {
                auto headtail = reserve(size());

                node = headtail.first;
                node_type* head = node->m_next;
                node_type* tail = headtail.second;
                atomic_push(m_freeNodes, head, tail);
                break;
            }
            else
            {
                abaptr<node_type> new_val =
                { node->m_next, old_val.m_counter + 1 };
                success = m_freeNodes.atomic_cas(old_val, new_val);
            }
        } while (!success);
        return &node->m_data;
    }
    void deallocate(T* p)
    {
        node_type* node = node_type::recover(p);
        atomic_push(m_freeNodes, node);
    }
    size_type size() const
    {
        return m_reserved.load(std::memory_order_relaxed);
    }
private:
    std::pair<node_type*, node_type*> reserve(size_type size)
    {
        static constexpr size_type min_size = MIN_SIZE;
        size_type chunk_size = std::max(size, min_size);
        chunk_type* chunk = allocate_chunk(chunk_size);

        node_type* data = chunk->m_data;
        node_type* head = data;
        head->m_next = nullptr;
        for (size_type i = 1; i < chunk->m_size; ++i)
        {
            node_type* node = &data[i];
            node->m_next = head;
            head = node;
        }
        atomic_push(m_chunks, chunk);
        m_reserved.fetch_add(chunk_size, std::memory_order_relaxed);
        return std::make_pair(head, data);
    }

    chunk_type* allocate_chunk(size_type chunk_size)
    {
        size_type byte_size = chunk_type::getByteSize(chunk_size);
        char* raw_ptr = m_byte_allocator.allocate(byte_size);
        chunk_type* chunk = reinterpret_cast<chunk_type*>(raw_ptr);
        m_chunk_allocator.construct(chunk, chunk_size);
        chunk->construct(m_node_allocator);
        return chunk;
    }

    void deallocate_chunk(chunk_type* chunk)
    {
        size_type byte_size = chunk_type::getByteSize(chunk->m_size);
        char* raw_ptr = reinterpret_cast<char*>(chunk);

        chunk->destroy(m_node_allocator);
        m_chunk_allocator.destroy(chunk);

        m_byte_allocator.deallocate(raw_ptr, byte_size);
    }
private:
    std::atomic<size_type> m_reserved;
    volatile abaptr<node_type> m_freeNodes;
    std::atomic<chunk_type*> m_chunks;
    byte_allocator_type m_byte_allocator;
    node_allocator_type m_node_allocator;
    chunk_allocator_type m_chunk_allocator;
};
}

#endif /* INCLUDE_POOL_BUFFER_HPP_ */
