/*
 * bit_trie_set.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: masha
 */


#include <gtest/gtest.h>

#include <bit_trie_set.hpp>

TEST(BitTrieSet, empty)
{
    typedef lfds::bit_trie_set<> bit_trie_set;
    typedef bit_trie_set::size_type size_type;

    bit_trie_set ts;

    bool res = false;
    size_type size = 0;

    size = ts.size();
    EXPECT_EQ(size, 0);

    res = ts.find("");
    EXPECT_FALSE(res);

    res = ts.erase("");
    EXPECT_FALSE(res);

    res = ts.find("a");
    EXPECT_FALSE(res);

    res = ts.erase("a");
    EXPECT_FALSE(res);
}

TEST(BitTrieSet, insert)
{
    typedef lfds::bit_trie_set<> bit_trie_set;
    typedef bit_trie_set::size_type size_type;

    bit_trie_set ts;

    bool res = false;
    size_type size = 0;

    size = ts.size();
    EXPECT_EQ(size, 0);

    res = ts.insert("");
    EXPECT_TRUE(res);

    size = ts.size();
    EXPECT_EQ(size, 1);

    res = ts.find("");
    EXPECT_TRUE(res);

    res = ts.find("a");
    EXPECT_FALSE(res);

    res = ts.insert("");
    EXPECT_FALSE(res);

    res = ts.insert("a");
    EXPECT_TRUE(res);

    size = ts.size();
    EXPECT_EQ(size, 2);

    res = ts.find("");
    EXPECT_TRUE(res);

    res = ts.find("a");
    EXPECT_TRUE(res);

    res = ts.find("aa");
    EXPECT_FALSE(res);

    res = ts.find("bb");
    EXPECT_FALSE(res);

    res = ts.insert("aa");
    EXPECT_TRUE(res);

    size = ts.size();
    EXPECT_EQ(size, 3);

    res = ts.insert("bb");
    EXPECT_TRUE(res);

    size = ts.size();
    EXPECT_EQ(size, 4);

    res = ts.find("");
    EXPECT_TRUE(res);

    res = ts.find("a");
    EXPECT_TRUE(res);

    res = ts.find("aa");
    EXPECT_TRUE(res);

    res = ts.find("bb");
    EXPECT_TRUE(res);

    res = ts.find("b");
    EXPECT_FALSE(res);

    res = ts.find("abcdef");
    EXPECT_FALSE(res);

    res = ts.insert("abcdef");
    EXPECT_TRUE(res);

    size = ts.size();
    EXPECT_EQ(size, 5);

    res = ts.find("abcdef");
    EXPECT_TRUE(res);

    res = ts.find("abcd");
    EXPECT_FALSE(res);
}

TEST(BitTrieSet, erase)
{
    typedef lfds::bit_trie_set<> bit_trie_set;
    typedef bit_trie_set::size_type size_type;

    bit_trie_set ts;

    bool res = false;
    size_type size = 0;

    ts.insert("");
    res = ts.erase("");
    EXPECT_TRUE(res);
    res = ts.find("");
    EXPECT_FALSE(res);
    size = ts.size();
    EXPECT_EQ(size, 0);

    ts.insert("abcdef");
    ts.insert("abcd");
    res = ts.erase("ab");
    EXPECT_FALSE(res);
    res = ts.erase("");
    EXPECT_FALSE(res);
    size = ts.size();
    EXPECT_EQ(size, 2);
    res = ts.erase("abcd");
    EXPECT_TRUE(res);
    size = ts.size();
    EXPECT_EQ(size, 1);
    res = ts.find("abcd");
    EXPECT_FALSE(res);
    res = ts.find("abcdef");
    EXPECT_TRUE(res);
    res = ts.erase("abcdef");
    EXPECT_TRUE(res);
    res = ts.find("abcdef");
    EXPECT_FALSE(res);
    size = ts.size();
    EXPECT_EQ(size, 0);
}


