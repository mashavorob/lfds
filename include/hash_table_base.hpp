/*
 * hash_table_base.hpp
 *
 *  Created on: Feb 6, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_TABLE_BASE_HPP_
#define INCLUDE_HASH_TABLE_BASE_HPP_

#include <algorithm>
#include <cassert>

namespace lfds
{
namespace
{
// basic constants for all hash tables
class hash_table_base
{
public:
    static const unsigned int HIGH_WATERMARK_MULT = 7;
    static const unsigned int HIGH_WATERMARK_DIV = 10;
    static const unsigned int MIN_CAPACITY = 20;

    typedef hash_table_base this_type;
    typedef std::size_t size_type;

    static size_type adjustCapacity(size_type capacity)
    {
        return std::max(capacity, static_cast<size_type>(MIN_CAPACITY));
    }
    static size_type calcWatermark(size_type capacity)
    {
        return adjustCapacity(capacity) * HIGH_WATERMARK_MULT
                / HIGH_WATERMARK_DIV;
    }
    //
    // template<class CompatibleKey, Key>
    // struct CompatiblePredicate : public binary_function<CompatibleKey, Key, bool>
    // {
    //      bool operator()(const CompatibleKey & key, const Key & value) const;
    //      ...
    // };

private:
    hash_table_base(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;
protected:
    hash_table_base(size_type reserve) :
            m_capacity(adjustCapacity(reserve)), m_high_watermark(
                    calcWatermark(reserve)), m_size(0), m_used(0)
    {

    }
    void advance_index(size_type & i) const
    {
        if (++i >= m_capacity)
        {
            i = 0;
        }
    }
    template<class Rehasher>
    void check_watermark(const Rehasher & rehash)
    {
        if (m_used >= m_high_watermark)
        {
            // prevent concurrent accessing
            typedef exclusive_lock lock_type;
            typedef lock_type::guard_type guard_type;

            guard_type guard = lock_type::create(m_lock);
            guard.lock();

            if (m_used >= m_high_watermark)
            {
                rehash(m_capacity*2);
            }
        }
    }
    void swap(this_type& other)
    {
#ifdef _DEBUG
        assert(m_size < m_capacity);
#endif
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_high_watermark, other.m_high_watermark);
        std::size_t dummy = m_size;
        m_size.store(other.m_size, std::memory_order_relaxed);
        other.m_size = dummy;
        dummy = m_used;
        m_used.store(other.m_used, std::memory_order_relaxed);
        other.m_used = dummy;
        assert(m_size < m_capacity);
    }

protected:
    size_type m_capacity;
    size_type m_high_watermark;
    std::atomic<size_type> m_size;
    std::atomic<size_type> m_used;
    mutable two_level_lock m_lock;
};
}
}

#endif /* INCLUDE_HASH_TABLE_BASE_HPP_ */
