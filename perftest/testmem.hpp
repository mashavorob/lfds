/*
 * testmem.hpp
 *
 *  Created on: Mar 30, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTMEM_HPP_
#define PERFTEST_TESTMEM_HPP_

#include "performancetest.hpp"
#include <map>

namespace lfds
{
namespace perftest
{

class IMemTester
{
public:
    virtual ~IMemTester()
    {

    }

    virtual void doTest() = 0;
};

class MemConsumptionTest: public IPerformanceTest
{
public:
    MemConsumptionTest() :
            m_tester(nullptr), m_mult(1)
    {

    }

    void setTester(IMemTester* impl)
    {
        m_tester = impl;
    }
    void setMult(double mult)
    {
        m_mult = mult;
    }

    double doTest();

private:
    static void freeHook(void *ptr, const void *caller);
    static void *mallocHook(size_t size, const void *caller);
    static void *reallocHook(void *ptr, size_t size, const void *caller);
    static void *memalignHook(size_t alignment,
                              size_t size,
                              const void *caller);

    static void onAlloc(void* ptr, size_t cb);
    static void onFree(void* ptr);

    static void saveHooks();
    static void setHooks();
    static void restoreHooks();

private:
    typedef std::size_t size_type;
    typedef std::map<void*, size_type> memmap_type;

    typedef void (*free_hook_type)(void *, const void *);
    typedef void *(*malloc_hook_type)(size_t __size, const void *);
    typedef void *(*realloc_hook_type)(void *, size_t, const void *);
    typedef void *(*memalign_hook_type)(size_t, size_t, const void *);

    static free_hook_type m_oldFreeHook;
    static malloc_hook_type m_oldMallocHook;
    static realloc_hook_type m_oldReallocHook;
    static memalign_hook_type m_oldMemalignHook;

    static memmap_type m_mmap;
    static size_type m_memSize;

    IMemTester* m_tester;
    double m_mult;
};

template<class Tester>
class IMemTesterImpl : public IMemTester
{
public:
    // override
    void doTest()
    {
        m_test();
    }
private:
    Tester m_test;
};

template<class Tester, int Multiplier>
class MemConsumptionTestImpl : public MemConsumptionTest
{
public:
    MemConsumptionTestImpl()
    {
        setTester(&m_impl);
        setMult(static_cast<double>(Multiplier));
    }
private:
    IMemTesterImpl<Tester> m_impl;
};

}
}

#endif /* PERFTEST_TESTMEM_HPP_ */
