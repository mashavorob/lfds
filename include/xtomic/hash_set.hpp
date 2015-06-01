/*
 * hash_set.hpp
 *
 *  Created on: Feb 12, 2015
 *      Author: masha
 */

///
/// \file hash_set.hpp
///
/// @brief `hash_set<>` and related helpers.
///

#ifndef INCLUDE_HASH_SET_HPP_
#define INCLUDE_HASH_SET_HPP_

/// \cond HIDDEN_SYMBOLS

#include "hash_map.hpp"
#include "impl/memory_model.hpp"
#include "impl/hash_set_table.hpp"
#include "impl/hash_set_table_base.hpp"
#include "impl/hash_set_table_integral_key.hpp"
#include "impl/xtraits.hpp"
#include "aux/xfunctional.hpp"

/// \endcond

namespace xtomic
{

/// \cond HIDDEN_SYMBOLS

namespace
{

// hash_set is an adapter for hash_map
template<typename T, typename Hash, typename Pred, typename Allocator, bool =
        is_integral<T>::value>
struct make_hash_set_table;

template<typename T, typename Hash, typename Pred, typename Allocator>
struct make_hash_set_table<T, Hash, Pred, Allocator, false>
{
    typedef hash_set_table<T, Hash, Pred, Allocator> type;
};

template<typename T, typename Hash, typename Pred, typename Allocator>
struct make_hash_set_table<T, Hash, Pred, Allocator, true>
{
    typedef hash_set_table_integral_key<T, Hash, Pred, Allocator> type;
};
}
/// \endcond

/// \class hash_set
///
/// \brief Template class for generic thread-safe lock-free hash set. The class does not require
/// additional synchronization when used in multi-thread environment. Open-addressing technique
/// is used to resolve collisions.
///
/// @param T type of value.
/// @param Hash type of hash function, std::hash<T> is used by default.
/// @param Pred type of equal function, std::equal_to<T> is used by default.
/// @param Allocator type of allocator, by default std::allocator<T>.
/// @param MemModel memory model, by default memory_model::wise is used.
///
/// Memory model allows to find balance between performance and memory consumption:
///
/// * Wise memory model reduces memory consumption at cost of speed.
/// * Greedy memory model increases performance at cost of memory consumption.
///
/// There are useful helpers for using different memory models:
/// * [make_wise_hash_set](@ref make_wise_hash_set)
/// * [make_greedy_hash_set](@ref make_greedy_hash_set)
///
template<typename T, typename Hash = typename make_hash<T>::type,
        typename Pred = std::equal_to<T>,
        typename Allocator = std::allocator<T>, memory_model::type MemModel =
                default_memory_model::value>
class hash_set
{
public:

    /// \cond HIDDEN_SYMBOLS
    typedef hash_set<T, Hash, Pred, Allocator> this_type;
    /// \endcond

    typedef typename make_hash_set_table<T, Hash, Pred, Allocator>::type hash_table_type; ///< hash table type.
    typedef typename hash_table_type::size_type size_type;                                ///< size type.
    typedef typename hash_table_type::key_type key_type;                                  ///< key type (aka T).
    typedef typename hash_table_type::hash_func_type hash_func_type;                      ///< hash function type.
    typedef typename hash_table_type::equal_predicate_type equal_predicate_type;          ///< equal predicate type.
    typedef typename hash_table_type::allocator_type allocator_type;                      ///< allocator type.
    typedef typename hash_table_type::snapshot_type snapshot_type;                        ///< snapshot type (aka std::vector<T>).

    static constexpr bool INTEGRAL = hash_table_type::INTEGRAL;   ///< true if value is treated as integral type.
    static constexpr memory_model::type MEMORY_MODEL = MemModel;  ///< used memory model.

public:

    ///
    /// Constructor.
    ///
    /// @param initialCapacity specifies initial capacity of the container. Default value is 0.
    ///
    /// *Note:* the container starts rehashing before capacity is achieved. It is because the container
    /// reserves additional slot for each concurrent insert operation.
    ///
    hash_set(size_type initialCapacity = 0) :
            m_hash_table(),
            m_hash_table_base(m_hash_table, initialCapacity)
    {
    }

    ///
    /// The method builds snapshot of the object.
    ///
    /// @param snapshot receives content of the object.
    ///
    void getSnapshot(snapshot_type & snapshot) const
    {
        m_hash_table_base.getSnapshot(snapshot);
    }

    ///
    /// The method checks if specified key is present in the container.
    ///
    /// @param key specifies a value to find.
    /// @return true if specified value is found otherwise false.
    ///
    bool find(const key_type & key) const
    {
        return m_hash_table_base.find(key);
    }

    ///
    /// The method inserts a new value into the container.
    ///
    /// @param key specifies a value to find.
    /// @return false if the container already had the specified value otherwise true.
    ///
    bool insert(const key_type & key)
    {
        return m_hash_table_base.insert(key);
    }

    ///
    /// The method erases previously inserted value from the container.
    ///
    /// @param key specifies a value to erase.
    /// @return false if the specified value was not found otherwise true.
    ///
    bool erase(const key_type & key)
    {
        return m_hash_table_base.erase(key);
    }

    ///
    /// The method returns number of elements in the container.
    ///
    /// @return number of elements in the container.
    ///
    size_type size() const
    {
        return m_hash_table_base.size();
    }

    ///
    /// The method return capacity of the container. Capacity is a maximum number of elements that can
    /// be inserted without resizing.
    ///
    /// *Note:* the container starts rehashing before ~70% capacity is achieved. It is because the container
    /// reserves additional slot for each concurrent insert operation.
    ///
    /// @return capacity of the container.
    ///
    size_type getCapacity() const
    {
        return m_hash_table_base.getCapacity();
    }

private:
    //typedef hash_set_table_base<hash_table_type> hash_table_base_type;
    typedef hash_set_table_base_traits<hash_table_type, MemModel> hash_set_table_base_traits_type;
    typedef typename hash_set_table_base_traits_type::type hash_table_base_type;

    hash_table_type m_hash_table;
    hash_table_base_type m_hash_table_base;
};

///
/// \class make_wise_hash_set
///
/// \brief A shortcut to create a hash_set with wise memory model.
///
/// Wise memory model reduces memory consumption at cost of speed.
/// See [hash_set](@ref hash_set) for parameters description.
///
template<typename T, typename Hash = typename make_hash<T>::type,
        typename Pred = std::equal_to<T>, typename Allocator = std::allocator<T> >
struct make_wise_hash_set
{
    /// hash_set with wise memory model
    typedef hash_set<T, Hash, Pred, Allocator, memory_model::wise> type;
};

///
/// \class make_greedy_hash_set
///
/// \brief A shortcut to create a hash_set with greedy memory model.
///
/// Greedy memory model increases speed at cost of memory consumption.
/// See [hash_set](@ref hash_set) for parameters description.
///
template<typename T, typename Hash = typename make_hash<T>::type,
        typename Pred = std::equal_to<T>, typename Allocator = std::allocator<T> >
struct make_greedy_hash_set
{
    /// hash_set with greedy memory model
    typedef hash_set<T, Hash, Pred, Allocator, memory_model::greedy> type;
};

}

#endif /* INCLUDE_HASH_SET_HPP_ */
