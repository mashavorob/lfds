/*
 * bit_string_table.hpp
 *
 *  Created on: Feb 19, 2015
 *      Author: masha
 */

#ifndef INCLUDE_BIT_STRING_TABLE_HPP_
#define INCLUDE_BIT_STRING_TABLE_HPP_

#include <algorithm>
#include <climits>
#include <cstdint>

namespace lfds
{

class bit_string_table
{
public:
    static constexpr int TABLE_SIZE = UCHAR_MAX + 1;
    static constexpr int ROW_SIZE = 8;

    typedef bit_string_table this_type;
    typedef uint8_t index_type;
    typedef index_type row_type[ROW_SIZE];
    typedef row_type table_type[TABLE_SIZE];

private:
    bit_string_table(const this_type&) = delete;
    this_type& operator=(const this_type&) = delete;
    bit_string_table()
    {
        for (int row = 0; row < TABLE_SIZE; ++row)
        {
            for (int col = 0; col < ROW_SIZE; ++col)
            {
                int mask = 1 << col;
                m_table[row][col] = ((mask & row) != 0);
            }
        }
    }

public:

    static const bit_string_table& instance()
    {
        static const bit_string_table singletone;
        return singletone;
    }

    const index_type* getBitString(const int c) const
    {
        int index = static_cast<int>(static_cast<unsigned char>(static_cast<char>(c)));
        return m_table[index];
    }

    const index_type* operator[](const int c) const
    {
        return getBitString(c);
    }

private:
    table_type m_table;
};

}

#endif /* INCLUDE_BIT_STRING_TABLE_HPP_ */
