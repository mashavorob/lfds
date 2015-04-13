/*
 * wildcard.cpp
 *
 *  Created on: Apr 13, 2015
 *      Author: masha
 */

namespace lfds
{
namespace perftest
{
bool matchWildCard(const char* wild, const char* string)
{
    while (*wild && *string)
    {
        if (*wild == '?')
        {
            ++wild;
            ++string;
        }
        else if (*wild == '*')
        {
            while (*wild && *wild == '*')
            {
                ++wild;
            }
            if (*wild == 0)
            {
                return true;
            }
            while (*string)
            {
                if (matchWildCard(wild, string))
                {
                    return true;
                }
                ++string;
            }
            return false;
        }
        else if (*wild == *string)
        {
            ++wild;
            ++string;
        }
        else
        {
            return false;
        }
    }
    return *wild == 0 && *string == 0;
}

}
}
