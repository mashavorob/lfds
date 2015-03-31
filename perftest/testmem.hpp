/*
 * testmem.hpp
 *
 *  Created on: Mar 30, 2015
 *      Author: masha
 */

#ifndef PERFTEST_TESTMEM_HPP_
#define PERFTEST_TESTMEM_HPP_

#include "performancetest.hpp"

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
