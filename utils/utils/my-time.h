/*
 * my-time.h
 *
 *  Created on: Apr 1, 2015
 *      Author: masha
 */

#ifndef UTILS_UTILS_MY_TIME_H_
#define UTILS_UTILS_MY_TIME_H_

#include <cppbasics.hpp>

#include <time.h>

namespace lfds
{
namespace my
{

namespace timer
{
enum erealtime
{
    realtime
};

enum emonotonic
{
    monotonic
};
enum ethread
{
    thread
};
enum eprocess
{
    process
};
}

class timestamp
{
public:
    static constexpr unsigned int NANOSECONDS_PER_SECOND =
            static_cast<unsigned int>(1e9);

    timestamp()
    {

    }
    explicit timestamp(const timespec & data)
    {
        m_data = data;
    }
    explicit timestamp(const double seconds)
    {
        m_data.tv_sec = static_cast<time_t>(seconds);
        const double frac_seconds = seconds
                - static_cast<double>(m_data.tv_sec);
        const double to_ns = static_cast<double>(NANOSECONDS_PER_SECOND);
        m_data.tv_nsec = static_cast<long>(frac_seconds * to_ns);
    }
    explicit timestamp(timer::erealtime )
    {
        clock_gettime(CLOCK_REALTIME, &m_data);
    }
    explicit timestamp(timer::emonotonic )
    {
        clock_gettime(CLOCK_MONOTONIC, &m_data);
    }
    explicit timestamp(timer::ethread )
    {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &m_data);
    }
    explicit timestamp(timer::eprocess )
    {
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &m_data);
    }

    static void now(int pthread_timer, timestamp& result)
    {
        clock_gettime(pthread_timer, &result.data());
    }
    static timestamp now(int pthread_timer)
    {
        timestamp result;
        now(pthread_timer, result);
        return result;
    }
    void sub(const timestamp & other)
    {
        if (m_data.tv_nsec < other.m_data.tv_nsec)
        {
            --m_data.tv_sec;
            m_data.tv_nsec += NANOSECONDS_PER_SECOND;
        }
        m_data.tv_nsec -= other.m_data.tv_nsec;
        m_data.tv_sec -= other.m_data.tv_sec;
    }
    void add(const timestamp & other)
    {
        m_data.tv_nsec += other.m_data.tv_nsec;
        if (m_data.tv_nsec >= NANOSECONDS_PER_SECOND)
        {
            m_data.tv_nsec -= NANOSECONDS_PER_SECOND;
            ++m_data.tv_sec;
        }
        m_data.tv_sec += other.m_data.tv_sec;
    }
    bool less(const timestamp & other) const
    {
        if ( m_data.tv_sec < other.m_data.tv_sec )
        {
            return true;
        }
        else if ( m_data.tv_sec > other.m_data.tv_sec )
        {
            return false;
        }
        return m_data.tv_nsec < other.m_data.tv_nsec;
    }
    timespec& data()
    {
        return m_data;
    }
    const timespec& data() const
    {
        return m_data;
    }
    double seconds() const
    {
        const double to_ns = static_cast<double>(NANOSECONDS_PER_SECOND);
        const double integer_part = static_cast<double>(m_data.tv_sec);
        const double fractional_part = static_cast<double>(m_data.tv_nsec)/to_ns;
        return integer_part + fractional_part;
    }

    timestamp& operator-=(const timestamp & other)
    {
        sub(other);
        return *this;
    }
    timestamp& operator+=(const timestamp & other)
    {
        add(other);
        return *this;
    }
    timestamp& operator-(const timestamp & other) const
    {
        timestamp res(*this);
        res.sub(other);
        return res;
    }
    timestamp& operator+(const timestamp & other) const
    {
        timestamp res(*this);
        res.add(other);
        return res;
    }
    bool operator<(const timestamp & other) const
    {
        return less(other);
    }
    bool operator<=(const timestamp & other) const
    {
        return !other.less(*this);
    }
    bool operator==(const timestamp & other) const
    {
        return !less(other) && !other.less(*this);
    }
    bool operator>=(const timestamp & other) const
    {
        return !less(other);
    }
    bool operator>(const timestamp & other) const
    {
        return other.less(*this);
    }

private:
    timespec m_data;
};

}
}

#endif /* UTILS_UTILS_MY_TIME_H_ */
