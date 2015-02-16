/*
 * .hpp
 *
 *  Created on: Feb 13, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_SET_INTEGRAL_KEY_HPP_
#define INCLUDE_HASH_SET_INTEGRAL_KEY_HPP_

#include "meta_utils.hpp"

namespace lfds
{

// Eclipse Luna does not recognize alignas keyword regardless of any settings
// It seems Eclipse's developers do not admit the bug so the only way
// to avoid annoying error marks these macro are used
template<class Key>
struct __attribute__((aligned(2*sizeof(Key)))) hash_set_integral_key
{
    typedef hash_set_integral_key<Key> this_type;
    typedef typename get_int_by_size<sizeof(Key)>::type state_type;
    // states of the item (m_state field)
    enum
    {
        unused,     // initial state
        touched,    // key is valid
        allocated,  // key is valid
    };

    // normal life cycle of an item in the hash table is:
    //
    //  <after initialization>
    // 1. unused  - initial state. Fields key and value have not been initialized
    //
    //  <first insert finished, the insert is atomic >
    // 2. allocated - ready to use, all fields are valid
    //
    //  <first erase finished, the operation is atomic>
    // 3. touched - erase operation finished.
    //              key is valid
    //              value is destroyed
    //
    //  <subsequent insert finished, the operation is atomic>
    // 4. allocated - exactly same as 2.

    hash_set_integral_key() :
            m_key(), m_state()
    {

    }
    hash_set_integral_key(const Key key, const state_type state) :
            m_key(key), m_state(state)
    {

    }
    hash_set_integral_key(const this_type & other) :
            m_key(other.m_key), m_state(other.m_state)
    {

    }
    hash_set_integral_key(const volatile this_type & other) :
            m_key(other.m_key), m_state(other.m_state)
    {

    }

    Key m_key;
    state_type m_state;
};

}

#endif /* INCLUDE_HASH_SET_INTEGRAL_KEY_HPP_ */
