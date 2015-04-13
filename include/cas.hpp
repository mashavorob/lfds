/*
 * cas.hpp
 *
 *  Created on: Dec 11, 2014
 *      Author: masha
 */

#ifndef INCLUDE_CAS_HPP_
#define INCLUDE_CAS_HPP_

#include "inttypes.hpp"

namespace lfds
{

namespace
{

template<class T, int = sizeof(T)>
struct CAS
{
    bool operator()(volatile T* var, const T* oldVal, const T* newVal) const
    {
        typedef typename get_type_by_size<sizeof(T)>::type data_type;

        volatile data_type* pVar = reinterpret_cast<volatile data_type*>(var);
        const data_type * pOldVal = reinterpret_cast<const data_type*>(oldVal);
        const data_type * pNewVal = reinterpret_cast<const data_type*>(newVal);

        // just magic
        asm volatile("" : : : "memory");

        return __sync_bool_compare_and_swap(pVar, *pOldVal, *pNewVal);
    }
};


template<class T>
struct CAS<T, 16>
{
    bool operator()(volatile T* var, const T* oldVal, const T* newVal) const
    {
        bool result;

        //
        // Both asm blocks here have "memory" among clobbers
        // but just the first one makes the magic. Just remove it
        // and you will see extremely strange effects.
        //
        asm volatile("" : : : "memory");

        asm volatile (
                "lock cmpxchg16b %[dst]\n\t"
                "setz %[res]\n"
                // output
                : [res] "=q" ( result )// write z flag to result
                ,[dst] "+m" ( *var )// pVar is bi-directional [in out]
                // input
                // RDX:RAX is to be compared with *var
                : "d" ( reinterpret_cast<const long long*>(oldVal)[1] ), "a" ( reinterpret_cast<const long long*>(oldVal)[0] )
                  // load *newVal to RCX:RBX
                , "c" ( reinterpret_cast<const long long*>(newVal)[1] ),"b" ( reinterpret_cast<const long long*>(newVal)[0] )
                // clobber list
                : "cc", "memory" // flags register is to be modified
        );

        return result;
    }
};

template<class T>
struct CAS<T, 8>
{
    bool operator()(volatile T* var, const T* oldVal, const T* newVal) const
    {
        bool result;

        //
        // Both asm blocks here have "memory" among clobbers
        // but just the first one makes the magic. Just remove it
        // and you will see extremely strange effects.
        //
        asm volatile("" : : : "memory");

        asm volatile (
                "lock cmpxchg %[new_val], %[dest]\n\t"
                "setz %[res]\n\t"
                // output
                : [res] "=q" ( result )// write z flag to result
                , [dest] "+m" ( *reinterpret_cast<volatile long long*>(var) )// pVar is bi-directional [in out]
                // input
                // RAX is to be compared with *var
                : [expected_val] "a" ( *reinterpret_cast<const long long*>(oldVal) )
                // load newVal to 32-bit register or use immediate value
                , [new_val] "r" ( *reinterpret_cast<const long long*>(newVal) )
                // clobber list
                : "cc", "memory"// flags register is to be modified
        );
        return result;
    }
};

template<class T>
struct CAS<T, 4>
{
    bool operator()(volatile T* var, const T* oldVal, const T* newVal) const
    {
        bool result;

        //
        // Both asm blocks here have "memory" among clobbers
        // but just the first one makes the magic. Just remove it
        // and you will see extremely strange effects.
        //
        asm volatile("" : : : "memory");

        asm volatile (
                "lock cmpxchg %[new_val], %[dest]\n\t"
                "setz %[res]\n\t"
                // output
                : [res] "=q" ( result )// write z flag to result
                , [dest] "+m" ( *var )// pVar is bi-directional [in out]
                // input
                // EAX is to be compared with *var
                : [expected_val] "a" ( *reinterpret_cast<const int*>(oldVal) )
                // load newVal to 32-bit register or use immediate value
                , [new_val] "r" ( *reinterpret_cast<const int*>(newVal) )
                // clobber list
                : "cc", "memory"// flags register is to be modified
        );
        return result;
    }
};

template<class T>
struct CAS<T, 2>
{
    bool operator()(volatile T* var, const T* oldVal, const T* newVal) const
    {
        bool result;

        //
        // Both asm blocks here have "memory" among clobbers
        // but just the first one makes the magic. Just remove it
        // and you will see extremely strange effects.
        //
        asm volatile("" : : : "memory");

        asm volatile (
                "lock; cmpxchgw %[new_val], %[dest]\n\t"
                "setz %[res]\n\t"
                // output
                : [res] "=q" ( result )// write z flag to result
                , [dest] "+m" ( *reinterpret_cast<volatile short*>(var) ) // var is bi-directional [in out]
                // input
                // ax is to be compared with *var
                : [expected_val] "wa" ( *reinterpret_cast<const short*>(oldVal) )
                // load newVal to a word register or use immediate value
                , [new_val] "r" ( *reinterpret_cast<const short*>(newVal) )
                // clobber list
                : "cc", "memory"// flags register is to be modified
        );
        return result;
    }
};

template<class T>
struct CAS<T, 1>
{
    bool operator()(volatile T* var, const T* oldVal, const T* newVal) const
    {
        bool result;

        //
        // Both asm blocks here have "memory" among clobbers
        // but just the first one makes the magic. Just remove it
        // and you will see extremely strange effects.
        //
        asm volatile("" : : : "memory");

        asm volatile (
                "lock; cmpxchgb %[new_val], %[dest]\n\t"
                "setz %[res]\n\t"
                // output
                : [res] "=q" ( result )// write z flag to result
                , [dest] "+m" ( *reinterpret_cast<volatile char*>(var) )// pVar is bi-directional [in out]
                // input
                // al is to be compared with *var
                : [expected_val] "a" ( *reinterpret_cast<const char*>(oldVal) )
                // load newVal to a byte register or use immediate value
                , [new_val] "r" ( *reinterpret_cast<const char*>(newVal) )
                // clobber list
                : "cc", "memory"// flags register is to be modified
        );
        return result;
    }
};
}

template<class T>
inline bool atomic_cas(volatile T & var, const T & oldVal, const T & newVal)
{
    return CAS<T>()(&var, &oldVal, &newVal);
}

}

#endif /* INCLUDE_CAS_HPP_ */
