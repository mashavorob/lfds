/*
 * trie_set.cpp
 *
 *  Created on: Feb 18, 2015
 *      Author: masha
 */



#include <gtest/gtest.h>

#include <trie_set.hpp>

using lfds::trie_set;

TEST(TrieSet, empty)
{
    typedef lfds::trie_set<> trie_set;
    typedef trie_set::size_type size_type;

    trie_set ts;

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

TEST(TrieSet, insert)
{
    typedef lfds::trie_set<> trie_set;
    typedef trie_set::size_type size_type;

    trie_set ts;

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

    res = ts.find("b");
    EXPECT_FALSE(res);

    res = ts.insert("bb");
    EXPECT_TRUE(res);

    res = ts.find("b");
    EXPECT_FALSE(res);

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

TEST(TrieSet, erase)
{
    typedef lfds::trie_set<> trie_set;
    typedef trie_set::size_type size_type;

    trie_set ts;

    bool res = false;
    size_type size = 0;

    ts.insert("");
    res = ts.erase("");
    EXPECT_TRUE(res);
    res = ts.find("");
    EXPECT_FALSE(res);
    size = ts.size();
    EXPECT_EQ(size, 0);

    ts.insert("abcd");
    ts.insert("abcdef");
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

TEST(TrieSet, erase2)
{
    typedef lfds::trie_set<> trie_set;
    typedef trie_set::size_type size_type;

    trie_set ts;

    bool res = false;
    size_type size = 0;

    ts.insert("abcdef");
    ts.insert("abcdfe");

    res = ts.erase("abcdef");
    EXPECT_TRUE(res);
    size = ts.size();
    EXPECT_EQ(size, 1);
    res = ts.find("abcdef");
    EXPECT_FALSE(res);
    res = ts.find("abcdfe");
    EXPECT_TRUE(res);
    res = ts.erase("abcdfe");
    EXPECT_TRUE(res);
    res = ts.find("abcdfe");
    EXPECT_FALSE(res);
    size = ts.size();
    EXPECT_EQ(size, 0);
}
