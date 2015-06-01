/*
 * hash_map
 *
 *  Created on: Jan 28, 2015
 *      Author: masha
 */

///
/// @file hash_map.hpp
///
/// @brief `hash_map<>` and related helpers.
///

#ifndef INCLUDE_HASH_MAP_HPP_
#define INCLUDE_HASH_MAP_HPP_

/// \cond HIDDEN_SYMBOLS

#include "impl/memory_model.hpp"
#include "impl/hash_map_table.hpp"
#include "impl/hash_map_table_integral_pair.hpp"
#include "impl/hash_map_table_integral_value.hpp"
#include "impl/hash_map_table_integral_key.hpp"
#include "impl/hash_map_table_base.hpp"
#include "impl/xtraits.hpp"
#include "aux/xfunctional.hpp"
#include "aux/cppbasics.hpp"

#include <functional>

/// \endcond

/// @brief Generic namespace for xtomic library.
namespace xtomic
{

/// \cond HIDDEN_SYMBOLS

//
// some meta classes to specify hash table implementation from Key and Value types
//
namespace
{

template<typename Key, typename Value, bool = (sizeof(Key) < sizeof(Value))>
struct dummy_hash_tuple
{
    Key m_key;
    Value m_value;
    int8_t m_dummy;
};

template<typename Key, typename Value>
struct dummy_hash_tuple<Key, Value, true>
{
    Key m_key;
    int8_t m_dummy;
    Value m_value;
};

template<typename Key, typename Value>
struct is_integral_pair
{
    enum
    {
        value = is_integral<Key>::value && is_integral<Value>::value
                && (sizeof(dummy_hash_tuple<Key, Value> ) <= 16)
    };
};

template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value>, bool IntegralKey = is_integral<Key>::value,
        bool IntegralValue = is_integral<Value>::value, bool IntegralKeyValue =
                is_integral_pair<Key, Value>::value>
struct hash_table_traits
{
    typedef xtomic::hash_map_table<Key, Value, Hash, Pred, Allocator> type;
};

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator, bool IntegralKey, bool IntegralValue>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, IntegralKey,
        IntegralValue, true>
{
    typedef xtomic::hash_map_table_integral_pair<Key, Value, Hash, Pred, Allocator> type;
};

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator, bool IntegralKey>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, IntegralKey, true,
        false>
{
    typedef xtomic::hash_map_table_integral_value<Key, Value, Hash, Pred,
            Allocator> type;
};

template<typename Key, typename Value, typename Hash, typename Pred,
        typename Allocator>
struct hash_table_traits<Key, Value, Hash, Pred, Allocator, true, false, false>
{
    typedef xtomic::hash_map_table_integral_key<Key, Value, Hash, Pred, Allocator> type;
};

}

/// \endcond

/// \class hash_map
///
/// \brief The class is associative container with constant complexity of insert, find and
/// erase operations. The container is thread-safe and it does not require additional
/// synchronization in multi-thread environment.
///
/// The class internally uses hash table with collision resolution by means of open-addressing.
/// Open-addressing provides better performance comparing with chaining. The tradeoff is memory
/// consumption due to using sparse array of bigger size.
///
/// This implementation provides optimizations when integral types are used as keys and values:
///
/// - Integral key-value pair, the best case: types of keys and values are integral, size of key-value pair is less then
///   16 bytes. The whole key-value pair is treated as integral, so all operations are atomic,
///   find operation is wait-free. Memory consumption is low.
/// - Value type is integral, improved performance: almost all operations are atomic, almost
///   there is no spinning in find operation. Memory consumption is reduced.
/// - Key is integral - improved memory consumption: performance at least not worse comparing
///   with general case.
///
/// Additional performance boost might be achieved by using greedy memory model. It allows
/// to the container do not release allocated memory until destructor is called. This approach
/// significantly relaxes synchronization that is very CPU consuming. The tradeoff is memory
/// consuming in case when it is impossible to reserve enough memory in constructor.
///
/// @param Key type of key.
/// @param Value type of mapped value.
/// @param Hash type of hash function, std::hash<Key> is used by default.
/// @param Pred type of equal function, std::equal_to<Key> is used by default.
/// @param Allocator type of allocator, by default std::allocator<Value>.
/// @param MemModel memory model, by default memory_model::wise is used.
///
/// Memory model allows to find balance between performance and memory consumption:
///
/// * Wise memory model reduces memory consumption at cost of speed.
/// * Greedy memory model increases performance at cost of memory consumption.
///
/// There are useful helpers for using different memory models:
/// * [make_wise_hash_map](@ref make_wise_hash_map)
/// * [make_greedy_hash_map](@ref make_greedy_hash_map)
template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value>, memory_model::type MemModel =
                default_memory_model::value>
class hash_map
{
public:

    /// \cond HIDDEN_SYMBOLS
    typedef hash_map<Key, Value, Hash, Pred, Allocator, MemModel> this_type;
    typedef hash_table_traits<Key, Value, Hash, Pred, Allocator> hash_table_traits_type;
    /// \endcond

    typedef typename hash_table_traits_type::type hash_table_type;               ///< hash table type.
    typedef typename hash_table_type::key_type key_type;                         ///< key type.
    typedef typename hash_table_type::mapped_type mapped_type;                   ///< mapped value type.
    typedef typename hash_table_type::hash_func_type hash_func_type;             ///< hash function type.
    typedef typename hash_table_type::equal_predicate_type equal_predicate_type; ///< equal predicate type.
    typedef typename hash_table_type::allocator_type allocator_type;             ///< allocator type.
    typedef typename hash_table_type::size_type size_type;                       ///< size type.
    typedef typename hash_table_type::value_type value_type;                     ///< key:mapped-value pair.
    typedef typename hash_table_type::snapshot_type snapshot_type;               ///< vector of key:mapped-value pairs.

    static constexpr bool INTEGRAL_KEY = hash_table_type::INTEGRAL_KEY;          ///< true if key type is treated as integral type.
    static constexpr bool INTEGRAL_VALUE = hash_table_type::INTEGRAL_VALUE;      ///< true if mapped type is treated as integral type.
    static constexpr bool INTEGRAL_KEYVALUE = hash_table_type::INTEGRAL_KEYVALUE;///< true if the whole pair key:mapped-value is treated as integral type.
    static constexpr memory_model::type MEMORY_MODEL = MemModel;                 ///< used memory model: greedy or wise.
private:
    hash_map(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;

public:

    ///
    /// \brief Constructor.
    ///
    /// @param initialCapacity initial capacity of the hash table. Default value is 0.
    ///
    /// *Note:* the container may start rehashing before capacity is achieved. It is because the container
    /// reserves additional slot for each concurrent insert operation.
    ///
    hash_map(const size_type initialCapacity = 0) :
            m_hash_table(),
            m_hash_table_base(m_hash_table, initialCapacity)
    {

    }

    ///
    /// \brief The method generates snapshot of the map as vector of pairs key_type - mapped_type.
    ///
    /// @param snapshot receives snapshot as vector of key:mapped-value pairs.
    ///
    void getSnapshot(snapshot_type & snapshot) const
    {
        m_hash_table_base.getSnapshot(snapshot);
    }

    /// @name Find
    /// @{

    ///
    /// @brief The operation finds value associated with specified key.
    ///
    /// @param key specifies key to find.
    /// @param value receives a value associated with the specified key.
    ///
    /// @return
    /// - `true` if the association was found otherwise `false`.
    ///
    bool find(const key_type & key, mapped_type & value) const
    {
        return m_hash_table_base.find(key, value);
    }

    ///
    /// @brief The operation finds value associated with specified key.
    ///
    /// @param key specifies key to find.
    ///
    /// @return
    /// - value associated with the key if found otherwise default value `mapped_type()`.
    ///
    const mapped_type find(const key_type & key) const
    {
        mapped_type value = mapped_type();
        m_hash_table_base.find(key, value);
        return value;
    }
    /// @}

    /// @name Insert
    /// @{

    ///
    /// The method inserts a new key-value pair. The method does nothing if
    /// the key already has associated value.
    ///
    /// @param key specifies a key
    /// @param val specifies a mapped-value
    /// @return
    /// - `true` if a new association was inserted.
    /// - `false` if the container already had a value associated with the specified key.
    ///    The old value remains unchanged.
    ///
#if XTOMIC_USE_CPP11
    template<typename ... Args>
    bool insert(const key_type & key, Args&&... val)
#else
    bool insert(const key_type & key, const mapped_type& val)
#endif
    {
        return m_hash_table_base.insert(key, false, std_forward(Args, val));
    }

    ///
    /// The method inserts a new key-value pair or updates existing one.
    ///
    /// @param key specifies a key.
    /// @param val specifies a mapped-value.
    ///
#if XTOMIC_USE_CPP11
    template<typename ... Args>
    void insertOrUpdate(const key_type & key, Args&&... val)
#else
    void insertOrUpdate(const key_type & key, const mapped_type& val)
#endif
    {
        m_hash_table_base.insert(key, true, std_forward(Args, val));
    }

    /// @}

    ///
    /// The method erases an existing association for specified key.
    ///
    /// @param key specifies a key.
    /// @return
    /// - `true` if an association for specified key was found. The operation always erases the association.
    /// - `false` if the container did not have value associated with specified key.
    ///
    bool erase(const key_type & key)
    {
        return m_hash_table_base.erase(key);
    }

    ///
    /// The method returns actual number of elements in the container.
    ///
    /// @return size of the container.
    ///
    size_type size() const
    {
        return m_hash_table_base.size();
    }

    ///
    /// The method return capacity of the container. Capacity is a maximum number of elements that can
    /// be inserted without rehashing.
    ///
    /// *Note:* the container may start rehashing before capacity is achieved. It is because the container
    /// reserves additional slot for each concurrent insert operation.
    ///
    /// @return capacity of the container.
    ///
    size_type getCapacity() const
    {
        return m_hash_table_base.getCapacity();
    }

private:
    //typedef hash_map_table_base<hash_table_type> hash_table_base_type;
    typedef hash_map_table_base_traits<hash_table_type, MemModel> hash_map_table_base_traits_type;
    typedef typename hash_map_table_base_traits_type::type hash_table_base_type;

    hash_table_type m_hash_table;
    hash_table_base_type m_hash_table_base;
};

///
/// \class make_wise_hash_map
///
/// \brief A shortcut to create a hash_map with wise memory model.
///
/// Wise memory model reduces memory consumption at cost of speed.
/// See [hash_map](@ref hash_map) for parameters description.
///
template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value> >
struct make_wise_hash_map
{
    /// hash_map with wise memory model
    typedef hash_map<Key, Value, Hash, Pred, Allocator, memory_model::wise> type;
};

///
/// \class make_greedy_hash_map
///
/// \brief A shortcut to create a hash_map with greedy memory model.
///
/// Greedy memory model increases speed at cost of memory consumption.
/// See [hash_map](@ref hash_map) for parameters description.
///
template<typename Key, typename Value,
        typename Hash = typename make_hash<Key>::type,
        typename Pred = std::equal_to<Key>, typename Allocator = std::allocator<
                Value> >
struct make_greedy_hash_map
{
    /// hash_map with greedy memory model
    typedef hash_map<Key, Value, Hash, Pred, Allocator, memory_model::greedy> type;
};

}

#endif /* INCLUDE_HASH_MAP_HPP_ */
