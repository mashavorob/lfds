/*
 * ref_lock.hpp
 *
 *  Created on: Feb 7, 2015
 *      Author: masha
 */

#ifndef INCLUDE_REF_LOCK_HPP_
#define INCLUDE_REF_LOCK_HPP_

#include <cassert>

namespace lfds
{

template<class T>
class ref_lock
{
public:
    typedef ref_lock<T> this_type;

    ref_lock(T& obj) :
            m_obj(&obj)
    {
        m_obj->add_ref();
    }
    ref_lock(T& obj, bool initialLock) :
            m_obj(&obj)
    {
        assert(!initialLock);
    }
    ~ref_lock()
    {
        m_obj->release();
    }

    void swap(this_type & other)
    {
        T* obj = m_obj;
        m_obj = other.get();
        other.put(obj);
    }

    void swap(T & other)
    {
        this_type dummy(other);
        swap(dummy);
    }

    void swap()
    {
        static T sentinelObj;
        this_type dummy(sentinelObj);
        dummy.swap(*this);
    }

    T* get()
    {
        return m_obj;
    }

    T& operator*()
    {
        return *m_obj;
    }
    T* operator->()
    {
        return m_obj;
    }
private:
    void put(T* obj)
    {
        m_obj = obj;
    }
private:

    ref_lock(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;

private:
    T *m_obj;
};

}

#endif /* INCLUDE_REF_LOCK_HPP_ */
