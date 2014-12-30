/*
 * queue.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef INCLUDE_QUEUE_HPP_
#define INCLUDE_QUEUE_HPP_

#include <utility>

#include "queue_base.hpp"
#include "queue_spscfs.hpp"
#include "buffer_traits.hpp"

namespace lfds {

template<class T, bool FixedSize = true, bool ManyProducers = true,
		bool ManyConsumers = true, class Allocator = std::allocator<T> >
class queue {
public:
	typedef T value_type;
	typedef buffer_traits<T, Allocator, FixedSize> traits_type;
	typedef typename traits_type::buffer_type buffer_type;
	typedef queue_base<T, ManyProducers, ManyConsumers> queue_type;
	typedef typename buffer_type::size_type size_type;

public:
	static const bool fixed_size = buffer_type::fixed_size;
	static const bool many_producers = queue_type::many_producers;
	static const bool many_consumers = queue_type::many_consumers;
	static const bool wait_free = false;
private:
	typedef queue<T, FixedSize, ManyProducers, ManyConsumers, Allocator> this_class;
	typedef typename buffer_type::node_type node_type;

private:
	queue(const this_class&);
	this_class& operator=(const this_class&);

public:

	queue(size_type capacity) :
			m_buff(capacity), m_size(0) {
	}

	~queue() {
		node_type* p = m_queue.pop();
		while (p) {
			m_buff.free_node(p);
			p = m_queue.pop();
		}
	}

	template<class ... Args>
	bool push(Args&&... val) {
		node_type* p = m_buff.new_node(std::forward<Args>(val)...);
		if (!p) {
			return false;
		}
		m_queue.atomic_push(p);
		++m_size;
		return true;
	}

	bool pop(T & val) {
		node_type* p = m_queue.atomic_pop();
		if (!p) {
			return false;
		}
		val = std::move(*p->data());
		m_buff.free_node(p);
		--m_size;
		return true;
	}

	size_type capacity() const {
		return m_buff.capacity();
	}

	size_type size() const {
		return m_size.load(std::memory_order_relaxed);
	}
private:
	buffer_type m_buff;
	queue_type m_queue;
	std::atomic<size_type> m_size;
};

template<class T, class Allocator >
class queue<T, true, false, false, Allocator> {
public:
	typedef queue<T, true, false, false, Allocator> this_class;
	typedef queue_spscfs<T, Allocator> queue_type;
	typedef typename queue_type::value_type value_type;
	typedef typename queue_type::size_type size_type;
private:
	queue(const this_class&);
	this_class& operator=(const this_class&);
public:
	static const bool fixed_size = true;
	static const bool many_producers = false;
	static const bool many_consumers = false;
	static const bool wait_free = true;
public:
	queue(size_type sz) :
			m_queue(sz) {
	}

	template<class ... Args>
	bool push(Args&&... args) {
		return m_queue.push(std::forward<Args>(args)...);
	}
	bool pop(T & val) {
		return m_queue.pop(val);
	}
	size_type size() const {
		return m_queue.size();
	}
private:
	queue_type m_queue;
};

}

#endif /* INCLUDE_QUEUE_HPP_ */
