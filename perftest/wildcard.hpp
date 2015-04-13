/*
 * wildcard.hpp
 *
 *  Created on: Apr 13, 2015
 *      Author: masha
 */

#ifndef PERFTEST_WILDCARD_HPP_
#define PERFTEST_WILDCARD_HPP_

#include <string>

namespace lfds
{
namespace perftest
{

bool matchWildCard(const char* wild, const char* string);

struct match_wild_card
{
public:
    match_wild_card(const std::string & name) :
            m_name(name)
    {

    }

    bool operator()(const std::string & wild) const
    {
        return matchWildCard(wild.c_str(), m_name.c_str());
    }
private:
    const std::string & m_name;
};

struct match_exact
{
public:
    match_exact(const std::string & name) :
            m_name(name)
    {

    }

    bool operator()(const std::string & s) const
    {
        return s == m_name;
    }
private:
    const std::string & m_name;
};

template<bool byWildCard>
struct get_str_match;

template<>
struct get_str_match<true>
{
    typedef match_wild_card type;
};

template<>
struct get_str_match<false>
{
    typedef match_exact type;
};

}
}

#endif /* PERFTEST_WILDCARD_HPP_ */
