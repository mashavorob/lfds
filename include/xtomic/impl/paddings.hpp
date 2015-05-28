/*
 * paddings.hpp
 *
 *  Created on: Feb 11, 2015
 *      Author: masha
 */

#ifndef INCLUDE_PADDINGS_HPP_
#define INCLUDE_PADDINGS_HPP_

#include <xtomic/aux/inttypes.hpp>

#pragma pack(push)
#pragma pack(1)

namespace xtomic
{
namespace
{

// paddings of different size
template<int size>
class padding_type
{
public:
    typedef padding_type<size> this_type;
public:
    padding_type() :
            m_dummy(0),
            m_padding()
    {
    }
private:
    padding_type(const this_type &); // = delete;
    padding_type(const volatile this_type &); // = delete;
    this_type & operator=(const this_type &); // = delete;
    this_type & operator=(const volatile this_type &); // = delete;
private:
    typedef padding_type<size - 1> nested_padding_type;
private:
    int8_t m_dummy;
    nested_padding_type m_padding;
};

// does not depend on nesting
template<>
class padding_type<0>
{
public:
    typedef padding_type<0> this_type;
public:
    padding_type()
    {
    }
private:
    padding_type(const this_type &); // = delete;
    padding_type(const volatile this_type &); // = delete;
    this_type & operator=(const this_type &); // = delete;
    this_type & operator=(const volatile this_type &); // = delete;
};

// does not depend on nesting
template<>
class padding_type<1>
{
public:
    typedef padding_type<1> this_type;
public:
    padding_type() :
            m_dummy(0)
    {
    }
private:
    padding_type(const this_type &); // = delete;
    padding_type(const volatile this_type &); // = delete;
    this_type & operator=(const this_type &); // = delete;
    this_type & operator=(const volatile this_type &); // = delete;
private:
    int8_t m_dummy;
};
template<>
class padding_type<2>
{
public:
    typedef padding_type<2> this_type;
public:
    padding_type() :
            m_dummy(0)
    {
    }
private:
    padding_type(const this_type &); // = delete;
    padding_type(const volatile this_type &); // = delete;
    this_type & operator=(const this_type &); // = delete;
    this_type & operator=(const volatile this_type &); // = delete;
private:
    int16_t m_dummy;
};

template<>
class padding_type<4>
{
public:
    typedef padding_type<4> this_type;
public:
    padding_type() :
            m_dummy(0)
    {
    }
private:
    padding_type(const this_type &); // = delete;
    padding_type(const volatile this_type &); // = delete;
    this_type & operator=(const this_type &); // = delete;
    this_type & operator=(const volatile this_type &); // = delete;
private:
    int32_t m_dummy;
};

template<>
class padding_type<6>
{
public:
    typedef padding_type<6> this_type;
public:
    padding_type() :
            m_dummy(0)
    {
    }
private:
    padding_type(const this_type &); // = delete;
    padding_type(const volatile this_type &); // = delete;
    this_type & operator=(const this_type &); // = delete;
    this_type & operator=(const volatile this_type &); // = delete;
private:
    typedef padding_type<4> nested_padding_type;
private:
    int16_t m_dummy;
    nested_padding_type m_padding;
};
}
}
#pragma pack(pop)

#endif /* INCLUDE_PADDINGS_HPP_ */
