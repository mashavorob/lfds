/*
 * testallocator.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTALLOCATOR_HPP_
#define PERFTEST_TESTALLOCATOR_HPP_

#include <xtomic/aux/cppbasics.hpp>

#include <memory>

namespace xtomic
{
namespace perftest
{

class allocator_base
{
public:
    static std::size_t getAllocated()
    {
        return m_cbAllocated;
    }
protected:
    static void onAlloc(std::size_t cb)
    {
        m_cbAllocated += cb;
    }
    static void onFree(std::size_t cb)
    {
        m_cbAllocated -= cb;
    }
private:
    static std::size_t m_cbAllocated;
};

template<typename Allocator>
class counted_allocator: private allocator_base
{
public:

    typedef counted_allocator<Allocator> this_type;

    typedef Allocator nested_allocator_type;

    typedef typename nested_allocator_type::value_type value_type;
    typedef typename nested_allocator_type::pointer pointer;
    typedef typename nested_allocator_type::reference reference;
    typedef typename nested_allocator_type::const_pointer const_pointer;
    typedef typename nested_allocator_type::const_reference const_reference;
    typedef typename nested_allocator_type::size_type size_type;
    typedef typename nested_allocator_type::difference_type difference_type;

    typedef std::allocator<value_type> t;
    template<typename Type>
    struct rebind
    {
        typedef typename nested_allocator_type::template rebind<Type>::other other_nested_allocator_type;
        typedef counted_allocator<other_nested_allocator_type> other;
    };

    static constexpr unsigned int VALUE_SIZE = sizeof(value_type);
public:
    counted_allocator()
    {

    }

    counted_allocator(const this_type& other) :
            m_nestedAlloc(other.getNested())
    {

    }

    template<typename Alloc>
    counted_allocator(const counted_allocator<Alloc>& other) :
            m_nestedAlloc(other.getNested())
    {

    }

    const nested_allocator_type& getNested() const
    {
        return m_nestedAlloc;
    }

    pointer address(reference r) const
    {
        return m_nestedAlloc.address(r);
    }
    const_pointer address(const_reference r) const
    {
        return m_nestedAlloc.address(r);
    }
    pointer allocate(size_type n)
    {
        allocator_base::onAlloc(n * VALUE_SIZE);
        return m_nestedAlloc.allocate(n);
    }
    void deallocate(pointer p, size_type n)
    {
        allocator_base::onFree(n * VALUE_SIZE);
        m_nestedAlloc.deallocate(p, n);
    }
    size_type max_size() const
    {
        return m_nestedAlloc.max_size();
    }
#if LFDS_USE_CPP11
    template<typename ... Args>
    void construct(pointer p, Args&&... val)
#else
    void construct(pointer p, const value_type & val)
#endif
    {
        m_nestedAlloc.construct(p, std_forward(Args, val));
    }
    void destroy(pointer p)
    {
        m_nestedAlloc.destroy(p);
    }
private:
    nested_allocator_type m_nestedAlloc;
};

}
}

#endif /* PERFTEST_TESTALLOCATOR_HPP_ */
