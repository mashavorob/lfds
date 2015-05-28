/*
 * set_insert_erase.hpp
 *
 *  Created on: Apr 17, 2015
 *      Author: masha
 */

#ifndef TESTS_UNITS_SET_INSERT_ERASE_HPP_
#define TESTS_UNITS_SET_INSERT_ERASE_HPP_

#include <xtomic/queue.hpp>
#include <utils/my-rand.hpp>

#include <vector>

namespace xtomic
{
namespace testing
{

template<typename Set, int SetSize, int Repetitions>
class set_insert_erase
{
public:
    // Repetitions
    typedef set_insert_erase<Set, SetSize, Repetitions> this_type;

    typedef Set set_type;
    typedef typename set_type::key_type key_type;

    typedef std::size_t size_type;

    static const int LowerWatermark = SetSize - SetSize / 10;
    static const int UpperWatermark = SetSize + SetSize / 10;

private:
    typedef xtomic::my::random_generator<key_type> rand_type;
    typedef xtomic::queue<key_type, xtomic::Queue::FixedSize> queue_type;
    typedef std::vector<key_type> vector_type;

public:
    set_insert_erase(set_type& set) :
            m_set(set),
            m_forInsert(UpperWatermark + 1),
            m_forErase(UpperWatermark + 1),
            m_failsOnInsert(0),
            m_failsOnFind(0),
            m_failsOnMissings(0),
            m_failsOnErase(0)
    {

    }
public:
    size_type getFailsOnInsert() const
    {
        return m_failsOnInsert;
    }
    size_type getFailsOnFind() const
    {
        return m_failsOnFind;
    }
    size_type getFailsOnMissing() const
    {
        return m_failsOnMissings;
    }
    size_type getFailsOnErase() const
    {
        return m_failsOnErase;
    }

    void run()
    {
        m_failsOnInsert = 0;
        m_failsOnFind = 0;
        m_failsOnMissings = 0;
        m_failsOnErase = 0;
        m_finish = false;

        static const size_type sampleSize = SetSize * 2;
        static const size_type totalSize = sampleSize * 2;

        rand_type rand;

        vector_type allKeys;
        rand(totalSize, allKeys);

        typename vector_type::const_iterator beg = allKeys.begin();
        typename vector_type::const_iterator mid = beg + sampleSize;
        typename vector_type::const_iterator end = allKeys.end();

        for (typename vector_type::const_iterator i = beg; i != mid; ++i)
        {
            key_type key = *i;
            m_forInsert.push(key);
        }

        vector_type missings(mid, end);
        m_missings.swap(missings);

        enum
        {
            inserter_0 = 0, inserter_1, eraser_0, eraser_1, finder, max_therads
        };

        pthread_t threads[max_therads] =
        { 0 };
        void* parg = reinterpret_cast<void*>(this);
        pthread_create(&threads[inserter_0], 0, &inserter_end, parg);
        pthread_create(&threads[inserter_1], 0, &inserter_end, parg);
        pthread_create(&threads[eraser_0], 0, &erraser_end, parg);
        pthread_create(&threads[eraser_1], 0, &erraser_end, parg);
        pthread_create(&threads[finder], 0, &find_missings, parg);

        pthread_join(threads[inserter_0], 0);
        pthread_join(threads[inserter_1], 0);
        m_finish = true;

        pthread_join(threads[eraser_0], 0);
        pthread_join(threads[eraser_1], 0);
        pthread_join(threads[finder], 0);
    }

private:
    static void* inserter_end(void* parg)
    {
        this_type* pThis = reinterpret_cast<this_type*>(parg);
        set_type& set = pThis->m_set;
        queue_type& forInsert = pThis->m_forInsert;
        queue_type& forErase = pThis->m_forErase;
        for (int i = 0; i < Repetitions; ++i)
        {
            // wait for eraser
            while (set.size() > UpperWatermark)
                ;

            key_type key;
            bool res;

            // wait for eraser
            do
            {
                res = forInsert.pop(key);
            }
            while (!res);

            res = set.insert(key);
            if (!res)
            {
                ++pThis->m_failsOnInsert;
            }
            forErase.push(key);
        }
        pthread_exit(0);
    }
    static void* erraser_end(void* parg)
    {
        this_type* pThis = reinterpret_cast<this_type*>(parg);
        set_type& set = pThis->m_set;
        queue_type& forInsert = pThis->m_forInsert;
        queue_type& forErase = pThis->m_forErase;

        while (!pThis->m_finish)
        {
            // wait for inserter
            if (set.size() < LowerWatermark)
            {
                continue;
            }

            key_type key;
            bool res = forErase.pop(key);
            if (!res)
            {
                continue;
            }

            res = set.find(key);
            if (!res)
            {
                ++pThis->m_failsOnFind;
            }
            res = set.erase(key);
            if (!res)
            {
                ++pThis->m_failsOnErase;
            }
            forInsert.push(key);
        }

        pthread_exit(0);
    }
    static void* find_missings(void* parg)
    {
        this_type* pThis = reinterpret_cast<this_type*>(parg);
        set_type& map = pThis->m_set;
        const vector_type& missings = pThis->m_missings;

        typedef typename vector_type::const_iterator const_iterator;

        const_iterator beg = missings.begin();
        const_iterator end = missings.end();

        for (const_iterator i = beg; !pThis->m_finish; ++i)
        {
            if (i == end)
            {
                i = beg;
            }
            key_type key = *i;
            bool res = map.find(key);
            if (res)
            {
                ++pThis->m_failsOnMissings;
            }
        }

        pthread_exit(0);
    }
private:
    set_type & m_set;
    queue_type m_forInsert;
    queue_type m_forErase;
    vector_type m_missings;

    size_type m_failsOnInsert;
    size_type m_failsOnFind;
    size_type m_failsOnMissings;
    size_type m_failsOnErase;

    volatile bool m_finish;
};

}
}

#endif /* TESTS_UNITS_SET_INSERT_ERASE_HPP_ */
