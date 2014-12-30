/*
 * abptr.hpp
 *
 *  Created on: Dec 12, 2014
 *      Author: masha
 */

#ifndef INCLUDE_ABAPTR_HPP_
#define INCLUDE_ABAPTR_HPP_

#include <cstddef>
#include "cas.hpp"

namespace lfds {

// pointer that protects CAS operations with pointers from
// ABA problem:
//
//    -- some variable
//    T* ptr;
//    ...
//    bool result;
//    T* oldVal;
//    do
//    {
//       oldVal = ptr;
//       newVal->next = oldVal->next;
//       -- some one else deletes ptr
//       -- other one allocates new memory block by the same address
//		 -- (just usual coincidence) and updates ptr again with te same value
//       result = CAS(&ptr, oldVal, newVal);
//       -- bah: CAS() returned true and ptr-> points to incorrect node
//     }
//     while (!result);
//
//     x86 cmxchgNb requires N bytes alignment for its operand

// workaround for Eclipse CDT C++11 support
#define ALIGN_ABA_PTR  alignas(sizeof(void*)*2)

template<class T>
class ALIGN_ABA_PTR abaptr
{
public:
	typedef std::size_t counter_type;

	abaptr(T* ptr = nullptr) : m_ptr(ptr), m_counter(0)
	{ }
	abaptr(const abaptr<T>& other)
	{
		*this = other;
	}
	abaptr(const volatile abaptr<T>& other)
	{
		*this = other;
	}
	abaptr<T>& operator=(const volatile abaptr<T>& other)
	{
		m_ptr = other.m_ptr;
		m_counter = other.m_counter;
		return *this;
	}
	bool atomic_cas(const abaptr<T>& expected, const abaptr<T>& val) volatile
	{
		return lfds::atomic_cas(*this, expected, val);
	}
public:
	T*				m_ptr;
	counter_type	m_counter;
};

}

#endif /* INCLUDE_ABAPTR_HPP_ */
