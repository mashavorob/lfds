/*
 * data_adapter.hpp
 *
 *  Created on: Jan 30, 2015
 *      Author: masha
 */

#ifndef DEMO_DATA_ADAPTER_HPP_
#define DEMO_DATA_ADAPTER_HPP_

#include <string>
#include <functional>
#include <utility>
#include <cstdint>

//
// the only purpose of the class is prevent xtomic::hash_map<>
// from using optimized model for integral types and strings
//
template<typename T>
struct data_adapter
{
    typedef data_adapter<T> this_class;

    data_adapter() :
            m_t()
    {

    }
    data_adapter(T t) :
            m_t(t)
    {

    }
    data_adapter(const this_class& other) :
            m_t(other.m_t)
    {

    }
    this_class & operator=(T t)
    {
        m_t = t;
        return *this;
    }
    this_class & operator=(const this_class& other)
    {
        m_t = other.m_t;
        return *this;
    }
    bool operator==(const this_class& other) const
    {
        return m_t == other.m_t;
    }
    bool operator<(const this_class& other) const
    {
        return m_t < other.m_t;
    }

    T m_t;
};

template<>
struct data_adapter<std::string>
{
    typedef data_adapter<std::string> this_class;

    data_adapter() :
            m_t()
    {

    }
    data_adapter(const std::string & t) :
            m_t(t)
    {

    }
    data_adapter(std::string && t) :
            m_t(std::forward<std::string>(t))
    {

    }
    data_adapter(const this_class& other) :
            m_t(other.m_t)
    {

    }
    data_adapter(this_class&& other) :
            m_t(std::forward<std::string>(other.m_t))
    {

    }
    this_class & operator=(const std::string & t)
    {
        m_t = t;
        return *this;
    }
    this_class & operator=(std::string && t)
    {
        m_t = std::forward<std::string>(t);
        return *this;
    }
    this_class & operator=(const this_class& other)
    {
        m_t = other.m_t;
        return *this;
    }
    this_class & operator=(this_class&& other)
    {
        m_t = std::forward<std::string>(other.m_t);
        return *this;
    }
    bool operator==(const this_class& other) const
    {
        return m_t == other.m_t;
    }
    bool operator<(const this_class& other) const
    {
        return m_t < other.m_t;
    }
    std::string m_t;
};

namespace std
{
template<>
struct hash<data_adapter<int16_t>>
{
    std::size_t operator()(data_adapter<int16_t> a) const
    {
        return std::hash<int16_t>()(a.m_t);
    }
};
template<>
struct hash<data_adapter<int32_t>>
{
    std::size_t operator()(data_adapter<int32_t> a) const
    {
        return std::hash<int32_t>()(a.m_t);
    }
};
template<>
struct hash<data_adapter<int64_t>>
{
    std::size_t operator()(data_adapter<int64_t> a) const
    {
        return std::hash<int64_t>()(a.m_t);
    }
};
template<>
struct hash<data_adapter<uint16_t>>
{
    std::size_t operator()(data_adapter<int16_t> a) const
    {
        return std::hash<uint16_t>()(a.m_t);
    }
};
template<>
struct hash<data_adapter<uint32_t>>
{
    std::size_t operator()(data_adapter<int32_t> a) const
    {
        return std::hash<uint32_t>()(a.m_t);
    }
};
template<>
struct hash<data_adapter<uint64_t>>
{
    std::size_t operator()(data_adapter<uint64_t> a) const
    {
        return std::hash<uint64_t>()(a.m_t);
    }
};
template<>
struct hash<data_adapter<std::string>>
{
    std::size_t operator()(const data_adapter<std::string> & a) const
    {
        return std::hash<std::string>()(a.m_t);
    }
};
}

#endif /* DEMO_DATA_ADAPTER_HPP_ */
