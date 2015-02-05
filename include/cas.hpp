/*
 * cas.hpp
 *
 *  Created on: Dec 11, 2014
 *      Author: masha
 */

#ifndef INCLUDE_CAS_HPP_
#define INCLUDE_CAS_HPP_

namespace lfds
{

namespace
{

template<class T, int = sizeof(T)>
struct CAS
{
    bool operator()(volatile T& var, const T oldVal, const T newVal) const
    {
        return __sync_bool_compare_and_swap(&var, oldVal, newVal);
    }
};

template<class T>
struct CAS<T, 16>
{
private:
    union SPLITTER
    {
        struct
        {
            unsigned long long m_lo;
            unsigned long long m_hi;
        };
        T m_data;
    };
public:
    bool operator()(volatile T& var, const T & oldVal, const T & newVal) const
    {
        bool result;

        // cmpxchg16b mem ; (RDX:RAX == mem) ? (mem = RCX:RBX, 1) : 0
        volatile SPLITTER * pVar = reinterpret_cast<volatile SPLITTER *>(&var);
        const SPLITTER* pOldVal = reinterpret_cast<const SPLITTER*>(&oldVal);
        const SPLITTER* pNewVal = reinterpret_cast<const SPLITTER*>(&newVal);

        asm volatile (
                "lock cmpxchg16b %1\n\t"
                "setz %0\n"
                // output
                : "=q" ( result )// write z flag to result
                ,"+m" ( *pVar )// pVar is bi-directional [in out]
                // input
                : "d" ( pOldVal->m_hi ), "a" ( pOldVal->m_lo )// RDX:RAX is to be compared with *pVar
                , "c" ( pNewVal->m_hi ),"b" ( pNewVal->m_lo )// load newVal to RCX:RBX
                // clobber list
                : "cc"// flags register is to be modified
        );

        return result;
    }
};
}

template<class T>
inline bool atomic_cas(volatile T & var, const T & oldVal, const T & newVal)
{
    return CAS<T>()(var, oldVal, newVal);
}

}

#endif /* INCLUDE_CAS_HPP_ */
