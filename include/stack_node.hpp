/*
 * stack_node.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef STACK_NODE_HPP_
#define STACK_NODE_HPP_

#include "cppbasics.hpp"
#include <cstddef>

namespace lfds
{

template<typename T>
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
    T* getData()
    {
        return reinterpret_cast<T*>(m_data);
    }

    static this_type* recover(T* p)
    {
        // workaround for offsetof bug in old GCC versions
        static const this_type dummy;
        static const int offset = &dummy.m_data[0]
                - reinterpret_cast<const char*>(&dummy);

        return reinterpret_cast<this_type*>(reinterpret_cast<char*>(p) - offset);
    }

private:
    char m_data[sizeof(T)] align_as(T);
};

}

#endif /* STACK_NODE_HPP_ */
