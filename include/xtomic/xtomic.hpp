/*
 * async.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

/// \file xtomic.hpp

#ifndef INCLUDE_XTOMIC_HPP_
#define INCLUDE_XTOMIC_HPP_

#include <xtomic/config.hpp>

namespace xtomic
{

///
/// \brief The namespace contains collection of barriers types.
///
/// Unlike C++11 each barrier type is implemented as stand-alone type. It allows to chose
/// appropriate implementation at compile-time. C++11 atomics provide ability to select the
/// barriers types at run time. This difference was implemented deliberately just for simplicity.
///
namespace barriers
{

///
/// \brief The barrier type to mark relaxed operations that do not imply any new restrictions.
///
enum erelaxed
{
    relaxed ///< relaxed operation, do not imply any new restrictions.
};

///
/// \brief The barrier type marks acquire operations: subsequent read operations
///        may not be moved before the barrier.
///
enum eacquire
{
    acquire ///< acquire operation: subsequent read operations may not be moved before the barrier
};

///
/// \brief The barrier marks release operations: preceding write operations may not be moved
///        after the barrier.
enum erelease
{
    release ///< release operation: preceding write operations may not be moved after the barrier.
};

///
/// \brief The barrier is a `full barrier`. It is equal to the combination of release and acquire
///        barriers.
enum efull
{
    full ///< full barrier, no optimization is is possible.
};
}

///
/// \brief The function implements release barrier.
///
inline void thread_fence(const barriers::erelease);

///
/// \brief The function implements acquire barrier.
///
inline void thread_fence(const barriers::eacquire);

///
/// \brief The class implements atomic variables of specified type. The class mimics std::atomic<>
/// from C++11 but it works even with compilers that do not support C++11 features, e.g. g++ v4.3.
/// The unconventional class name is chosen just to avoid confusing with std::atomic<>.
///
template<typename T>
class quantum
{
private:
    typedef quantum<T> this_type;

    quantum(const this_type&); // = delete;
    this_type& operator=(const this_type&); // = delete;

public:

    ///
    /// @defgroup quantum_quantum Constructors
    ///
    /// @param val specifies initial variable value.
    ///
    /// @{
    ///

    /// @brief Default constructor.
    quantum() :
            m_val()
    {

    }

    /// @brief Constructor with initial value.
    quantum(const T val) :
            m_val(val)
    {

    }
    /// @}

    ///
    /// @defgroup quantum_store Atomic store operation.
    ///
    /// The method atomically stores specified value and implements requested barrier.
    ///
    /// @param val specifies a value to store.
    ///
    /// @{

    /// @brief Relaxed store.
    void store(const T val, barriers::erelaxed);

    /// @brief Volatile version of relaxed store.
    void store(const T val, barriers::erelaxed) volatile;

    /// @brief Store with `release` barrier.
    void store(const T val, barriers::erelease);

    /// @brief Volatile version of store with `release` barrier.
    void store(const T val, barriers::erelease) volatile;
    /// @}

    ///
    /// @defgroup quantum_load Atomic load operation.
    ///
    /// The operation implements requested barrier and atomically loads a value from the object.
    ///
    /// @return value that the object holds.
    ///
    /// @{

    /// @brief Relaxed load.
    T load(const barriers::erelaxed) const;

    /// @brief Volatale version of relaxed load.
    T load(const barriers::erelaxed) const volatile;

    /// @brief Load with `acquire` barrier.
    T load(const barriers::eacquire) const;

    /// @brief Volatile version of load with `acquire` barrier.
    T load(const barriers::eacquire) const volatile;
    /// @}

    ///
    /// @defgroup quantum_fetch_add Atomic fetch and add operation.
    ///
    /// The operation atomically adds specified value to current object's value and implements requested
    /// memory barrier. At exit the methods return value, that was in object before the operation.
    ///
    /// The operation might be illustrated by this pseudo-code:
    ///
    /// \code
    /// template<typename T>
    /// T quantum<T>::fetch_add(const T val, barrier)
    /// {
    ///     T oldVal = m_val;
    ///     m_val += val;
    ///     return oldVal;
    /// }
    /// \endcode
    ///
    /// @param val specifies a value to add.
    /// @return value that was stored in the object before the operation.
    ///
    /// @{
    ///

    /// @brief Relaxed operation.
    T fetch_add(const T val, barriers::erelaxed);

    /// @brief Volatile version of relaxed operation.
    T fetch_add(const T val, barriers::erelaxed) volatile;

    /// @brief Operation with release barrier.
    T fetch_add(const T val, barriers::erelease);

    /// @brief Volatile version of the operation with release barrier.
    T fetch_add(const T val, barriers::erelease) volatile;
    /// @}

    ///
    /// @defgroup quantum_fetch_sub Atomic fetch and add operation.
    ///
    /// The operation atomically subtracts specified value from current object's value and returns
    /// value that was in the object before the operation. The operation implements requested barrier.
    ///
    /// It might be illustrated by this pseudo-code:
    ///
    /// @code
    /// template<typename T>
    /// T quantum<T>::fetch_sub(const T val, barrier)
    /// {
    ///     T oldVal = m_val;
    ///     m_val -= val;
    ///     return oldVal;
    /// }
    /// @endcode
    ///
    /// @param val specifies a value to add.
    /// @return value that was in the object before the operation.
    ///
    /// @{
    ///

    /// @brief Relaxed operation.
    T fetch_sub(const T val, barriers::erelaxed);

    /// @brief Volatile version of the relaxed operation.
    T fetch_sub(const T val, barriers::erelaxed) volatile;

    /// @brief Operation with release barrier.
    T fetch_sub(const T val, barriers::erelease);

    /// @brief Volatile version of the operation with release barrier.
    T fetch_sub(const T val, barriers::erelease) volatile;
    /// @}

    ///
    /// @defgroup quantum_atomic_cas Atomic Compare And Swap (CAS) operation.
    ///
    /// The method can be illustrated by pseudo-code:
    ///
    /// @code
    /// template<typename T>
    /// bool quantum<T>::atomic_cas(const T e, const T n)
    /// {
    ///     if ( m_val != e )
    ///         return false;
    ///     m_val = n;
    ///     return true;
    /// }
    /// @endcode
    ///
    /// @param e specifies expected value in the object.
    /// @param n specifies new value that should replace expected one.
    /// @return
    /// - `true` if expected value matched with actual value stored in the object. At exit new value
    ///   replaced old one in the object.
    /// - `false` if expected value did not match with actual value stored in the object. At exit
    ///   the object remains aunchanged.
    ///
    /// @{

    /// @brief atomic compare and swap.
    bool atomic_cas(const T e, const T n);

    /// @brief atomic compare and swap.
    bool atomic_cas(const this_type & e, const this_type & n);
    /// @}

    ///
    /// @defgroup quantum_increment Atomic increments
    ///
    /// The operators implement pre and post increments with full memory barriers.
    ///
    /// @{
    ///

    /// @brief conventional pre-increment.
    T operator++();

    /// @brief volatile pre-increment.
    T operator++() volatile;

    /// @brief conventional post-increment.
    T operator++(int);

    /// @brief volatile post-increment.
    T operator++(int) volatile;
    /// @}

    ///
    /// @defgroup quantum_decrement Atomic increments
    ///
    /// The operators implement pre and post decrements with full memory barriers.
    ///
    /// @{
    ///

    /// @brief conventional pre-decrement.
    T operator--();

    /// @brief volatile pre-decrement.
    T operator--() volatile;


    /// @brief conventional post-decrement.
    T operator--(int);

    /// @brief volatile post-decrement.
    T operator--(int) volatile;
    /// \}

private:
    volatile T m_val;
};


}

#if LFDS_USE_CPP11

#include "impl/xtomic-modern.hpp"

#else

#include "impl/xtomic-obsolete.hpp"

#endif

#endif /* INCLUDE_XTOMIC_HPP_ */
