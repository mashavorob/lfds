/*
 * set_wrapper.hpp
 *
 *  Created on: Feb 16, 2015
 *      Author: masha
 */

#ifndef DEMO_SET_WRAPPER_HPP_
#define DEMO_SET_WRAPPER_HPP_

#include <set>
#include <unordered_set>
#include <mutex>

template<class Set>
class set_wrapper
{
public:
    typedef set_wrapper<Set> this_type;
    typedef Set set_type;
    typedef typename set_type::key_type key_type;
    typedef typename set_type::size_type size_type;
    typedef typename set_type::iterator iterator;
    typedef typename set_type::const_iterator const_iterator;
private:
    typedef std::lock_guard<std::mutex> lock_type;

    set_wrapper(const this_type &);
    this_type & operator=(const this_type &);

public:
    set_wrapper(size_type = 0)
    {

    }
    bool find(const key_type & key) const
    {
        lock_type lock(m_mutex);
        const_iterator i = m_set.find(key);
        if (i != m_set.end())
        {
            return true;
        }
        return false;
    }
    bool insert(const key_type & key)
    {
        lock_type lock(m_mutex);
        return m_set.insert(key).second;
    }
    bool erase(const key_type & key)
    {
        lock_type lock(m_mutex);
        return m_set.erase(key) != 0;
    }
    size_type size() const
    {
        lock_type lock(m_mutex);
        return m_set.size();
    }

private:
    set_type m_set;
    mutable std::mutex m_mutex;
};

template<class Key, class Less = std::less<Key>,
        class Allocator = std::allocator<Key> >
class std_set_wrapper
{
public:
    typedef std::set<Key, Less, Allocator> set_type;
    typedef set_wrapper<set_type> wrapped_set_type;
    typedef typename wrapped_set_type::key_type key_type;
    typedef typename wrapped_set_type::size_type size_type;
public:
    std_set_wrapper(size_type = 0)
    {

    }
    bool find(const key_type & key) const
    {
        return m_set.find(key);
    }
    bool insert(const key_type & key)
    {
        return m_set.insert(key);
    }
    bool erase(const key_type & key)
    {
        return m_set.erase(key);
    }
    size_type size() const
    {
        return m_set.size();
    }

private:
    wrapped_set_type m_set;
};

template<class Key, class Hash = std::hash<Key>,
        class Pred = std::equal_to<Key>, class Allocator = std::allocator<Key > >
class std_unordered_set_wrapper
{
public:
    typedef std::unordered_set<Key, Hash, Pred, Allocator> set_type;
    typedef set_wrapper<set_type> wrapped_set_type;
    typedef typename wrapped_set_type::key_type key_type;
    typedef typename wrapped_set_type::size_type size_type;
public:
    std_unordered_set_wrapper(size_type = 0)
    {

    }
    bool find(const key_type & key) const
    {
        return m_set.find(key);
    }
    bool insert(const key_type & key)
    {
        return m_set.insert(key);
    }
    bool erase(const key_type & key)
    {
        return m_set.erase(key);
    }
    size_type size() const
    {
        return m_set.size();
    }

private:
    wrapped_set_type m_set;
};

#endif /* DEMO_SET_WRAPPER_HPP_ */
