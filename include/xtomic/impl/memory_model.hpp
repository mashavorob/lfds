/*
 * memory_model.hpp
 *
 *  Created on: Apr 24, 2015
 *      Author: masha
 */

#ifndef INCLUDE_MEMORY_MODEL_HPP_
#define INCLUDE_MEMORY_MODEL_HPP_

#include "hash_map_table_base.hpp"
#include "hash_set_table_base.hpp"
#include "xtraits.hpp"

namespace xtomic
{

struct memory_model
{
    enum type
    {
        greedy, wise,
    };
};

struct default_memory_model: public integral_const<memory_model::type,
        memory_model::greedy>
{

};

template<typename HashTable, memory_model::type type>
struct hash_map_table_base_traits;

template<typename HashTable, memory_model::type type>
struct hash_set_table_base_traits;

template<typename HashTable>
struct hash_map_table_base_traits<HashTable, memory_model::greedy> :
                                                                     public get_hash_map_table_base_type<
                                                                             HashTable,
                                                                             true>
{
};

template<typename HashTable>
struct hash_map_table_base_traits<HashTable, memory_model::wise> :
                                                                   public get_hash_map_table_base_type<
                                                                           HashTable,
                                                                           false>
{
};

template<typename HashTable>
struct hash_set_table_base_traits<HashTable, memory_model::greedy> :
                                                                     public get_hash_set_table_base_type<
                                                                             HashTable,
                                                                             true>
{
};

template<typename HashTable>
struct hash_set_table_base_traits<HashTable, memory_model::wise> :
                                                                   public get_hash_set_table_base_type<
                                                                           HashTable,
                                                                           false>
{
};

}

#endif /* INCLUDE_MEMORY_MODEL_HPP_ */
