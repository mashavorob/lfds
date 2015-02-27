/*
 * random_strings.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: masha
 */

#include "random_strings.hpp"

#include <unordered_set>
#include <cstring>
#include <cctype>

namespace {
void TokenizeText(const char* text, std::unordered_set<std::string> & tokens)
{
    const char* p = text;
    while ( p && *p )
    {
        const char* n = p;
        while ( *n && isalnum(*n) ) ++n;
        tokens.insert( std::string(p, n) );
        while ( *n && !isalnum(*n) ) ++n;
        p = n;
    }
}

void MultiplicateTokens(std::size_t num, std::unordered_set<std::string> & tokens)
{
    typedef std::unordered_set<std::string>::const_iterator iterator;

    while ( tokens.size() < num )
    {
        std::unordered_set<std::string> dummy = tokens;

        iterator end = tokens.end();

        for ( iterator i = tokens.begin(); dummy.size() < num && i != end; ++i)
        {
            iterator j = i;
            const std::string& w1 = *i;
            for ( ++j; j != end && dummy.size() < num; ++j)
            {
                const std::string& w2 = *j;
                dummy.insert(w1 + " " + w2);
            }
        }
        tokens.swap(dummy);
    }
}

}

void MakeUniqueStrings(const char* text, std::size_t numUniqueStrings,
        std::vector<std::string> & res)
{
    std::unordered_set<std::string> tokens;

    TokenizeText(text, tokens);
    MultiplicateTokens(numUniqueStrings, tokens);

    res.assign(tokens.begin(), tokens.end());
}



