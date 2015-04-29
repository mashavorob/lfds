/*
 * virtualbenchmark.cpp
 *
 *  Created on: Dec 30, 2014
 *      Author: masha
 */

#include <cassert>
#include <chrono>
#include <iostream>
#include <ctime>
#include <cstdlib>

enum Operation
{
    Plus, Minus,
};

class IArithmeticOperation
{
public:
    virtual ~IArithmeticOperation() = 0;
    virtual int op(int a, int b) = 0;
};
IArithmeticOperation::~IArithmeticOperation()
{
}

class COperationPlus: public IArithmeticOperation
{
private:
    friend class CArithmetricFactory;

    COperationPlus()
    {
    }
public:
    ~COperationPlus()
    {
    }
public:
    int op(int a, int b)
    {
        return a + b;
    }
};

class COperationMinus: public IArithmeticOperation
{
private:
    friend class CArithmetricFactory;

    COperationMinus()
    {
    }
public:
    ~COperationMinus()
    {
    }
public:
    int op(int a, int b)
    {
        return a - b;
    }
};

class CArithmetricFactory
{
public:

    CArithmetricFactory()
    {
    }

    IArithmeticOperation* Create(const Operation code) const
    {
        static COperationPlus plus;
        static COperationMinus minus;
        switch (code)
        {
        case Plus:
            return &plus;
        case Minus:
            return &minus;
        }
        assert(false);
        return nullptr;
    }
};

class CDynCastBaseClass
{
protected:
    CDynCastBaseClass()
    {
    }
public:
    virtual ~CDynCastBaseClass()
    {

    }

    int op(int a, int b) const;
};

class CDynCastMinus: public CDynCastBaseClass
{
    friend class CDynCastFactory;
private:
    CDynCastMinus()
    {

    }
public:
    int operationImpl(int a, int b) const
    {
        return a - b;
    }
};

class CDynCastPlus: public CDynCastBaseClass
{
    friend class CDynCastFactory;
private:
    CDynCastPlus()
    {

    }
public:
    int operationImpl(int a, int b) const
    {
        return a + b;
    }
};

int CDynCastBaseClass::op(int a, int b) const
{
    const CDynCastPlus* pPlus = dynamic_cast<const CDynCastPlus*>(this);
    if (pPlus)
    {
        return pPlus->operationImpl(a, b);
    }
    const CDynCastMinus* pMinus = dynamic_cast<const CDynCastMinus*>(this);
    if (pMinus)
    {
        return pMinus->operationImpl(a, b);
    }
    return 0;
}

class CDynCastFactory
{
public:
    const CDynCastBaseClass* Create(Operation code) const
    {
        static const CDynCastPlus plus;
        static const CDynCastMinus minus;

        switch (code)
        {
        case Plus:
            return &plus;
        case Minus:
            return &minus;
        }
        return nullptr;
    }
};

template<Operation code>
struct CArithmeticOperationImpl
{
    int operator()(int a, int b) const;
};

template<>
struct CArithmeticOperationImpl<Plus>
{
    int operator()(int a, int b) const
    {
        return a + b;
    }
};

template<>
struct CArithmeticOperationImpl<Minus>
{
    int operator()(int a, int b) const
    {
        return a - b;
    }
};

struct VirtualRunner
{
    int operator()(const Operation code, std::size_t repetitions, int a,
            int b) const
    {
        static const CArithmetricFactory factory;
        IArithmeticOperation* p = factory.Create(code);

        int summ = 0;
        do
        {
            summ += p->op(++a, b);
        } while (--repetitions);
        return summ;
    }
    static const char* name()
    {
        return "Virtual Function Runner";
    }
};

struct DynCastRunner
{
    int operator()(const Operation code, std::size_t repetitions, int a,
            int b) const
    {
        static const CDynCastFactory factory;
        const CDynCastBaseClass* p = factory.Create(code);

        int summ = 0;
        do
        {
            summ += p->op(++a, b);
        } while (--repetitions);
        return summ;
    }
    static const char* name()
    {
        return "Dynamic Cast Runner";
    }
};

template<Operation code>
struct TemplateRunnerImpl
{
    int operator()(std::size_t repetitions, int a, int b) const
    {
        CArithmeticOperationImpl<code> operation = CArithmeticOperationImpl<code>();
        int summ = 0;
        do
        {
            summ += operation(++a, b);
        } while (--repetitions);
        return summ;
    }
};

struct TemplateRunner
{
    int operator()(const Operation code, std::size_t repetitions, int a,
            int b) const
    {
        switch (code)
        {
        case Plus:
            return TemplateRunnerImpl<Plus>()(repetitions, a, b);
        case Minus:
            return TemplateRunnerImpl<Minus>()(repetitions, a, b);
        }
        assert(false);
        return 0;
    }
    static const char* name()
    {
        return "Template Function Runner";
    }
};

template<typename Runner>
struct Benchmark
{
    static const std::size_t repcount = 1000000000;

    typedef std::chrono::high_resolution_clock clock_type;
    typedef std::chrono::duration<double> seconds_type;
    typedef clock_type::duration duration_type;
    typedef Runner runner_type;

    int operator()(int a, int b)
    {
        std::cout << "Benchmark for " << runner_type::name() << std::endl
                << "---------------------------------" << std::endl;

        int r = runtest(Plus, a, b);
        r += runtest(Minus, a, b);
        std::cout << "Benchmark for " << runner_type::name() << std::endl
                << "---------------------------------" << std::endl
                << std::endl;
        return r;
    }
    int runtest(const Operation code, int a, int b)
    {
        seconds_type duration;
        runner_type runner;

        std::cout << "running " << runner_type::name()
                << (code == Plus ? " (Plus Operation)" : " (Minus Operation)")
                << std::endl;
        auto start_point = clock_type::now();
        int r = runner(code, repcount, a, b);
        auto hiresDuration = clock_type::now() - start_point;
        duration = std::chrono::duration_cast < seconds_type > (hiresDuration);
        std::cout << "duration: " << duration.count() << " secs" << std::endl;
        std::cout << duration.count() * 1.0e9 / static_cast<double>(repcount)
                << " ns per op" << std::endl;
        return r;
    }

};

void BenchmarkVirtualFunction()
{
    Benchmark<VirtualRunner> vr;
    Benchmark<DynCastRunner> dr;
    Benchmark<TemplateRunner> tr;

    std::cout << "Compare virtual function peformance vs template one"
            << std::endl;
    srand(time(nullptr));
    int a = rand();
    int b = rand();
    int r1 = vr(a, b);
    int r2 = dr(a, b);
    int r3 = tr(a, b);
    std::cout << " Result calculated by algorithm based on virtual functions: "
            << r1 << std::endl;
    std::cout << "    Result calculated by algorithm based on dynamic_cast<>: "
            << r2 << std::endl;
    std::cout << "Result calculated by algorithm based on template functions: "
            << r3 << std::endl;
}
