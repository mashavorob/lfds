/*
 * raw_hash_table.hpp
 *
 *  Created on: Mar 3, 2015
 *      Author: masha
 */

#ifndef INCLUDE_RAW_HASH_TABLE_HPP_
#define INCLUDE_RAW_HASH_TABLE_HPP_

#include "xtomic.hpp"

#include <cstddef>

namespace lfds {

template<class Node>
struct hash_data_table
{
    typedef Node node_type;
    typedef std::size_t size_type;

    node_type* m_table;
    size_type m_capacity;
    size_type m_highWatermark;
    xtomic<size_type> m_size;
    xtomic<size_type> m_used;
};

}


#endif /* INCLUDE_RAW_HASH_TABLE_HPP_ */
