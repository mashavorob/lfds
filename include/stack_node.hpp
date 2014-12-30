/*
 * stack_node.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: masha
 */

#ifndef STACK_NODE_HPP_
#define STACK_NODE_HPP_


namespace lfds {

template<class T>
class stack_node
{
public:
	typedef stack_node<T>	this_class;

public:
	this_class*	m_next;
	T*			data()
	{
		return reinterpret_cast<T*>(m_data);
	}
private:
	char 	m_data[sizeof(T)];
};

}



#endif /* STACK_NODE_HPP_ */
