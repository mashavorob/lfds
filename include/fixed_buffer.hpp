/*
 * fixed_buffer.hpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#ifndef INCLUDE_FIXED_BUFFER_HPP_
#define INCLUDE_FIXED_BUFFER_HPP_

#include "buffer_base.hpp"

namespace lfds {

template<class T, class Allocator>
class fixed_buffer : public buffer_base<T, Allocator>
{
public:
	typedef fixed_buffer<T, Allocator>				this_class;
	typedef buffer_base<T, Allocator>				base_class;
	typedef typename base_class::collection_type	collection_type;
	typedef typename base_class::node_type			node_type;
	typedef typename base_class::size_type			size_type;
public:
	static const bool fixed_size = true;

public:
	fixed_buffer(size_type capacity)
		: m_capacity(capacity)
		, m_reserved(nullptr)
	{
		m_reserved = base_class::allocate_nodes(m_capacity);
		for ( auto i = 0; i < m_capacity; ++i )
		{
			base_class::m_freeNodes.push(m_reserved + i);
		}
	}

	~fixed_buffer()
	{
		base_class::deallocate_nodes(m_reserved, m_capacity);
	}

	template<class... Args>
	node_type* new_node(Args&&... data)
	{
		node_type* p = base_class::m_freeNodes.atomic_pop();
		if ( p )
		{
			base_class::construct_data(p, std::forward<Args>(data)...);
		}
		return p;
	}

	size_type capacity() const
	{
		return m_capacity;
	}

private:
	size_type	m_capacity;
	node_type*	m_reserved;
};

}


#endif /* INCLUDE_FIXED_BUFFER_HPP_ */
