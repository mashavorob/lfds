/*
 * buffer_base.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_BUFFER_BASE_HPP_
#define INCLUDE_BUFFER_BASE_HPP_

namespace lfds
{

template<class Buffer>
class buffer_base
{
public:
    typedef buffer_base<Buffer> this_class;
    typedef Buffer buffer_type;
    typedef typename buffer_type::node_type node_type;
    typedef typename buffer_type::allocator_type data_allocator_type;
    typedef typename buffer_type::size_type size_type;
    typedef typename data_allocator_type::template rebind<node_type>::other node_allocator_type;

    // non copyable
private:
    buffer_base(const this_class &);
    this_class& operator=(const this_class&);

public:
    buffer_base()
    {
    }

public:
    node_type* allocate_nodes(size_type count)
    {
        return m_nodeAllocator.allocate(count);
    }
    void deallocate_nodes(node_type* p, size_type count)
    {
        m_nodeAllocator.deallocate(p, count);
    }

    template<class ... Args>
    void construct_data(node_type* p, Args&&... data)
    {
        m_dataAllocator.construct(p->data(), std::forward<Args>(data)...);
    }
    void destroy_data(node_type* p)
    {
        m_dataAllocator.destroy(p->data());
    }

private:
    data_allocator_type m_dataAllocator;
    node_allocator_type m_nodeAllocator;
};

}

#endif /* INCLUDE_BUFFER_BASE_HPP_ */
