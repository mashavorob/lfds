/*
 * async.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

/// \file quantum.hpp
///
/// @brief Memory barriers and atomics.
///

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

    /// @brief Default constructor.
    ///
    /// Constructs an object initialized by zero.
    ///
    quantum() :
            m_val()
    {

    }

    /// @brief Constructor with initial value.
    ///
    /// @param val specifies initial value.
    ///
    quantum(const T val) :
            m_val(val)
    {

    }

    /// @name Atomic store.
    /// @{

    ///
    /// The method atomically stores specified value and implements requested barrier.
    ///
    /// @param val specifies a value to store.
    ///
    void store(const T val, barriers::erelaxed);

    void store(const T val, barriers::erelaxed) volatile;

    void store(const T val, barriers::erelease);

    void store(const T val, barriers::erelease) volatile;
    /// @}


    /// @name Atomic load.
    /// @{

    ///
    /// The operation implements requested barrier and atomically loads a value from the object.
    ///
    /// @return value that the object holds.
    ///
    T load(const barriers::erelaxed) const;

    T load(const barriers::erelaxed) const volatile;

    T load(const barriers::eacquire) const;

    T load(const barriers::eacquire) const volatile;
    /// @}

    /// @name Atomic fetch and add operation.
    /// @{

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
    ///     thread_fence(barrier);
    ///     return oldVal;
    /// }
    /// \endcode
    ///
    /// @param val specifies a value to add.
    /// @return value that was stored in the object before the operation.
    ///
    T fetch_add(const T val, barriers::erelaxed);

    T fetch_add(const T val, barriers::erelaxed) volatile;

    T fetch_add(const T val, barriers::erelease);

    T fetch_add(const T val, barriers::erelease) volatile;
    /// @}

    /// @name Atomic fetch and sub operation.
    /// @{

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
    ///     thread_fence(barrier);
    ///     return oldVal;
    /// }
    /// @endcode
    ///
    /// @param val specifies a value to add.
    /// @return value that was in the object before the operation.
    ///
    T fetch_sub(const T val, barriers::erelaxed);

    T fetch_sub(const T val, barriers::erelaxed) volatile;

    T fetch_sub(const T val, barriers::erelease);

    T fetch_sub(const T val, barriers::erelease) volatile;
    /// @}


    /// @name Atomic compare and swap operation.
    /// @{

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

    bool atomic_cas(const T e, const T n);

    bool atomic_cas(const this_type & e, const this_type & n);
    /// @}

    ///
    /// @name Increments and decrements.
    ///
    /// The operators implement full memory barrier.
    ///
    /// @{

    T operator++();

    T operator++() volatile;

    T operator++(int);

    T operator++(int) volatile;

    T operator--();

    T operator--() volatile;


    T operator--(int);

    T operator--(int) volatile;
    /// @}

private:
    volatile T m_val;
};


}

#if XTOMIC_USE_CPP11

#include "impl/xtomic-modern.hpp"

#else

#include "impl/xtomic-obsolete.hpp"

#endif

#endif /* INCLUDE_XTOMIC_HPP_ */
