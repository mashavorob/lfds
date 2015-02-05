/*
 * test_conditions.hpp
 *
 *  Created on: Dec 16, 2014
 *      Author: masha
 */

#ifndef DEMO_TEST_EVENTS_HPP_
#define DEMO_TEST_EVENTS_HPP_

#include <atomic>
#include <condition_variable>
#include <mutex>

class test_events
{
private:
    typedef std::unique_lock<std::mutex> lock_type;

    test_events(const test_events&);
    test_events& operator=(const test_events&);
public:

    class event
    {
    public:
        event(std::condition_variable & cv, bool & flag, std::mutex & mutex) :
                m_cv(cv), m_flag(flag), m_mutex(mutex)
        {
        }

        void wait()
        {
            std::unique_lock < std::mutex > lock(m_mutex);
            while (!m_flag)
                m_cv.wait(lock);
        }

    private:
        std::condition_variable& m_cv;bool& m_flag;
        std::mutex& m_mutex;
    };

    test_events() :
            m_start(false), m_stop(false)
    {
    }

    void go()
    {
        if (!m_start)
        {
            std::unique_lock < std::mutex > lock(m_mutex);
            m_start = true;
            m_cv.notify_all();
        }
    }

    void stop()
    {
        if (!m_stop.load(std::memory_order_relaxed))
        {
            go();
            m_stop.store(true, std::memory_order_release);
        }
    }

    event get_start_ev()
    {
        return event(m_cv, m_start, m_mutex);
    }
    std::atomic<bool> & get_stop_ev()
    {
        return m_stop;
    }

private:
    std::condition_variable m_cv;
    std::mutex m_mutex;bool m_start;
    std::atomic<bool> m_stop;

};

#endif /* DEMO_TEST_EVENTS_HPP_ */
