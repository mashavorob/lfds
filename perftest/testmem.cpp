/*
 * testmem.cpp
 *
 *  Created on: Mar 30, 2015
 *      Author: masha
 */

#include "testmem.hpp"
#include <malloc.h>
#include <cassert>

namespace lfds
{
namespace perftest
{

MemConsumptionTest::free_hook_type MemConsumptionTest::m_oldFreeHook = nullptr;
MemConsumptionTest::malloc_hook_type MemConsumptionTest::m_oldMallocHook = nullptr;
MemConsumptionTest::realloc_hook_type MemConsumptionTest::m_oldReallocHook = nullptr;
MemConsumptionTest::memalign_hook_type MemConsumptionTest::m_oldMemalignHook = nullptr;

MemConsumptionTest::memmap_type MemConsumptionTest::m_mmap;
MemConsumptionTest::size_type MemConsumptionTest::m_memSize = 0;

double MemConsumptionTest::doTest()
{
    m_memSize = 0;
    saveHooks();
    setHooks();
    m_tester->doTest();
    restoreHooks();

    const double MbSize = 1024.*1024.;
    const double memSize = static_cast<double>(m_memSize)/MbSize;
    return memSize*m_mult;
}


void MemConsumptionTest::freeHook(void *ptr, const void *caller)
{
    restoreHooks();
    free(ptr);
    saveHooks();
    onFree(ptr);
    restoreHooks();
}
void *MemConsumptionTest::mallocHook(size_t size, const void *caller)
{
    restoreHooks();
    void* ptr = malloc(size);
    saveHooks();
    onAlloc(ptr, size);
    restoreHooks();
    return ptr;
}
void *MemConsumptionTest::reallocHook (void *ptr, size_t size, const void *caller)
{
    restoreHooks();
    void* ptrNew = realloc(ptr, size);
    saveHooks();
    onFree(ptr);
    onAlloc(ptrNew, size);
    restoreHooks();
    return ptrNew;
}
void *MemConsumptionTest::memalignHook(size_t alignment, size_t size, const void *caller)
{
    restoreHooks();
    void* ptr = memalign(alignment, size);
    saveHooks();
    onAlloc(ptr, size);
    restoreHooks();
    return ptr;
}
void MemConsumptionTest::onAlloc(void* ptr, size_t cb)
{
    m_memSize += cb;
    bool res = m_mmap.insert(std::make_pair(ptr, cb)).second;
    assert(res);
}
void MemConsumptionTest::onFree(void* ptr)
{
    memmap_type::iterator iter = m_mmap.find(ptr);
    if ( iter != m_mmap.end() )
    {
        size_type cb = iter->second;
        m_memSize -= cb;
        m_mmap.erase(iter);
    }

}
void MemConsumptionTest::saveHooks()
{
    m_oldFreeHook = __free_hook;
    m_oldMallocHook = __malloc_hook;
    m_oldReallocHook = __realloc_hook;
    m_oldMemalignHook = __memalign_hook;
}
void MemConsumptionTest::setHooks()
{
    __free_hook = MemConsumptionTest::freeHook;
    __malloc_hook = MemConsumptionTest::mallocHook;
    __realloc_hook = MemConsumptionTest::reallocHook;
    __memalign_hook = MemConsumptionTest::memalignHook;
}

void MemConsumptionTest::restoreHooks()
{
    __free_hook = m_oldFreeHook;
    __malloc_hook = m_oldMallocHook;
    __realloc_hook= m_oldReallocHook;
    __memalign_hook = m_oldMemalignHook;
}

}
}
