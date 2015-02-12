/*
 * ref_lock.hpp
 *
 *  Created on: Feb 7, 2015
 *      Author: masha
 */

#ifndef INCLUDE_REF_LOCK_HPP_
#define INCLUDE_REF_LOCK_HPP_

namespace lfds {

template<class T>
class ref_lock
{
public:
    ref_lock(const T& obj) : m_obj(obj)
    {
        m_obj.add_ref();
    }
    ~ref_lock()
    {
        m_obj.release();
    }
private:
    typedef ref_lock<T> this_type;

    ref_lock(const this_type&);
    this_type& operator=(const this_type&);

private:
    const T & m_obj;
};

}



#endif /* INCLUDE_REF_LOCK_HPP_ */
