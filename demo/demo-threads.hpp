/*
 * demo-threads.hpp
 *
 *  Created on: Mar 31, 2015
 *      Author: masha
 */

#ifndef DEMO_DEMO_THREADS_HPP_
#define DEMO_DEMO_THREADS_HPP_

#include <pthread.h>

namespace demo
{

namespace
{
class ICallable
{
public:
    virtual ~ICallable()
    {

    }
    virtual void Call() = 0;
};

template<typename Callable>
class ICallableImpl: public ICallable
{
public:
    ICallableImpl(const Callable & pred) :
            m_pred(pred)
    {

    }
    void Call()
    {
        m_pred();
    }
private:
    Callable m_pred;
};
}

class thread
{
public:
    template<typename Callable>
    thread(const Callable & pred) : m_impl(new ICallableImpl<Callable>(pred)), m_thread(0)
    {

    }
private:
    thread(const thread&);
    thread& operator=(const thread&);
private:
    ICallable* m_impl;
    pthread_t m_thread;
};

}

#endif /* DEMO_DEMO_THREADS_HPP_ */
