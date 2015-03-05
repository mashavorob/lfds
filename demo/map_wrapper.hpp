/*
 * map_wrapper.hpp
 *
 *  Created on: Jan 30, 2015
 *      Author: masha
 */

#ifndef DEMO_MAP_WRAPPER_HPP_
#define DEMO_MAP_WRAPPER_HPP_

#include <map>
#include <unordered_map>
#include <mutex>

template<class Map>
class map_wrapper
{
public:
    typedef map_wrapper<Map> this_type;
    typedef Map map_type;
    typedef typename map_type::key_type key_type;
    typedef typename map_type::mapped_type mapped_type;
    typedef typename map_type::size_type size_type;
    typedef typename map_type::iterator iterator;
    typedef typename map_type::const_iterator const_iterator;
private:
    typedef std::lock_guard<std::mutex> lock_type;

    map_wrapper(const this_type &);
    this_type & operator=(const this_type &);

public:
    map_wrapper(size_type = 0)
    {

    }
    bool find(const key_type & key, mapped_type & value) const
    {
        lock_type lock(m_mutex);
        const_iterator i = m_map.find(key);
        if (i != m_map.end())
        {
            const typename map_type::value_type & pair = *i;
            value = pair.second;
            return true;
        }
        return false;
    }
    template<class ... Args>
    bool insert(const key_type & key, Args&&... val)
    {
        lock_type lock(m_mutex);
        return m_map.insert(
                std::make_pair(key, mapped_type(std::forward<Args>(val)...))).second;
    }
    bool erase(const key_type & key)
    {
        lock_type lock(m_mutex);
        return m_map.erase(key) != 0;
    }
    size_type size() const
    {
        lock_type lock(m_mutex);
        return m_map.size();
    }

private:
    map_type m_map;
    mutable std::mutex m_mutex;
};

template<class Key, class T, class Less = std::less<Key>,
        class Allocator = std::allocator<std::pair<const Key, T> > >
class std_map_wrapper
{
public:
    typedef std::map<Key, T, Less, Allocator> map_type;
    typedef map_wrapper<map_type> wrapped_map_type;
    typedef typename wrapped_map_type::key_type key_type;
    typedef typename wrapped_map_type::mapped_type mapped_type;
    typedef typename wrapped_map_type::size_type size_type;
public:
    std_map_wrapper(size_type = 0)
    {

    }
    bool find(const key_type & key, mapped_type & value) const
    {
        return m_map.find(key, value);
    }
    template<class ... Args>
    bool insert(const key_type & key, Args&&... val)
    {
        return m_map.insert(key, std::forward<Args>(val)...);
    }
    bool erase(const key_type & key)
    {
        return m_map.erase(key);
    }
    size_type size() const
    {
        return m_map.size();
    }

private:
    wrapped_map_type m_map;
};

template<class Key, class T, class Hash = std::hash<Key>,
        class Pred = std::equal_to<Key>, class Allocator = std::allocator<
                std::pair<const Key, T> > >
class std_unordered_map_wrapper
{
public:
    typedef std::unordered_map<Key, T, Hash, Pred, Allocator> set_type;
    typedef map_wrapper<set_type> wrapped_set_type;
    typedef typename wrapped_set_type::key_type key_type;
    typedef typename wrapped_set_type::mapped_type mapped_type;
    typedef typename wrapped_set_type::size_type size_type;
public:
    std_unordered_map_wrapper(size_type = 0)
    {

    }
    bool find(const key_type & key, mapped_type & value) const
    {
        return m_map.find(key, value);
    }
    template<class ... Args>
    bool insert(const key_type & key, Args&&... val)
    {
        return m_map.insert(key, std::forward<Args>(val)...);
    }
    bool erase(const key_type & key)
    {
        return m_map.erase(key);
    }
    size_type size() const
    {
        return m_map.size();
    }

private:
    wrapped_set_type m_map;
};

#endif /* DEMO_MAP_WRAPPER_HPP_ */
