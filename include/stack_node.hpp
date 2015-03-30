/*
 * stack_node.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef STACK_NODE_HPP_
#define STACK_NODE_HPP_

#include <cstddef>

namespace lfds
{

template<class T>
class stack_node
{
public:
    typedef stack_node<T> this_type;
private:
    stack_node(const this_type&);
    this_type& operator=(const this_type&);
public:
    this_type* m_next;
    stack_node() :
            m_next(nullptr)
    {

    }
    T* data()
    {
        return reinterpret_cast<T*>(m_data);
    }

    static this_type* recover(T* p)
    {
        const int offset = offsetof(this_type, m_data);
        return reinterpret_cast<this_type*>(reinterpret_cast<char*>(p) - offset);
    }

private:
    char m_data[sizeof(T)] __attribute__((aligned(__alignof(T))));
};

}

#endif /* STACK_NODE_HPP_ */
