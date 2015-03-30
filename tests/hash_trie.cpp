/*
 * hash_trie.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: masha
 */

#include <gtest/gtest.h>

#include <hash_trie.hpp>

struct BadHashFunc
{
    std::size_t operator()(int v) const
    {
        return v << 16;
    }
};

struct TheWorstHashFunc
{
    std::size_t operator()(int) const
    {
        return 0;
    }
};

TEST(HashTrie, empty)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_trie<key_type, value_type> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    size = ht.size();
    EXPECT_EQ(size, 0);

    EXPECT_EQ(ht.dbgCountBranches(), 1);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    value_type val;
    res = ht.find(0, val);
    EXPECT_FALSE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.erase(0);
    EXPECT_FALSE(res);
}

TEST(HashTrie, insert)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_trie<key_type, value_type> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    res = ht.insert(0, -1);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.insert(0, -2);
    EXPECT_FALSE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    size = ht.size();
    EXPECT_EQ(size, 1);
    EXPECT_FALSE(ht.dbgCheckRefrences());
}

TEST(HashTrie, insertBranch)
{
    typedef int key_type;
    typedef int value_type;

    typedef lfds::hash_trie<key_type, value_type, 16, BadHashFunc> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    ht.insert(0, 0);
    res = ht.insert(1, 1);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.insert(1, 1);
    EXPECT_FALSE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    size = ht.size();
    EXPECT_EQ(size, 2);
    EXPECT_FALSE(ht.dbgCheckRefrences());
}

TEST(HashTrie, insertChain)
{
    typedef int key_type;
    typedef int value_type;

    typedef lfds::hash_trie<key_type, value_type, 16, TheWorstHashFunc> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    ht.insert(0, 0);
    res = ht.insert(1, 1);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.insert(1, 1);
    EXPECT_FALSE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    size = ht.size();
    EXPECT_EQ(size, 2);
}

TEST(HashTrie, find)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_trie<key_type, value_type> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    ht.insert(0, 0);
    ht.insert(1, 1);
    ht.insert(-2, -2);

    value_type v = -1;
    res = ht.find(0, v);
    EXPECT_TRUE(res);
    EXPECT_EQ(v, 0);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.find(1, v);
    EXPECT_TRUE(res);
    EXPECT_EQ(v, 1);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.find(-2, v);
    EXPECT_TRUE(res);
    EXPECT_EQ(v, -2);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.find(-1, v);
    EXPECT_FALSE(res);
    EXPECT_EQ(v, -2);
    EXPECT_FALSE(ht.dbgCheckRefrences());
}

TEST(HashTrie, findBranch)
{
    typedef int key_type;
    typedef int value_type;

    typedef lfds::hash_trie<key_type, value_type, 16, BadHashFunc> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    ht.insert(0, 0);
    ht.insert(1, 1);
    ht.insert(-2, -2);

    value_type v = -1;
    res = ht.find(0, v);
    EXPECT_TRUE(res);
    EXPECT_EQ(v, 0);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.find(1, v);
    EXPECT_TRUE(res);
    EXPECT_EQ(v, 1);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.find(-2, v);
    EXPECT_TRUE(res);
    EXPECT_EQ(v, -2);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.find(-1, v);
    EXPECT_FALSE(res);
    EXPECT_EQ(v, -2);
    EXPECT_FALSE(ht.dbgCheckRefrences());
}

TEST(HashTrie, findChain)
{
    typedef int key_type;
    typedef int value_type;

    typedef lfds::hash_trie<key_type, value_type, 16, TheWorstHashFunc> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    ht.insert(0, 0);
    ht.insert(1, 1);
    ht.insert(-2, -2);

    value_type v = -1;
    res = ht.find(0, v);
    EXPECT_TRUE(res);
    EXPECT_EQ(v, 0);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.find(1, v);
    EXPECT_TRUE(res);
    EXPECT_EQ(v, 1);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.find(-2, v);
    EXPECT_TRUE(res);
    EXPECT_EQ(v, -2);
    EXPECT_FALSE(ht.dbgCheckRefrences());

    res = ht.find(-1, v);
    EXPECT_FALSE(res);
    EXPECT_EQ(v, -2);
    EXPECT_FALSE(ht.dbgCheckRefrences());
}

TEST(HashTrie, erase)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_trie<key_type, value_type> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    res = ht.erase(0);
    EXPECT_FALSE(res);

    ht.insert(0, 0);
    ht.insert(1, 1);
    ht.insert(-2, -2);

    res = ht.erase(0);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 2);

    res = ht.erase(0);
    EXPECT_FALSE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 2);

    res = ht.erase(1);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 1);

    res = ht.erase(-2);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 0);
}

TEST(HashTrie, eraseBranch)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_trie<key_type, value_type, 16, BadHashFunc> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    res = ht.erase(0);
    EXPECT_FALSE(res);

    ht.insert(0, 0);
    ht.insert(1, 1);
    ht.insert(-2, -2);

    res = ht.erase(0);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 2);

    res = ht.erase(0);
    EXPECT_FALSE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 2);

    res = ht.erase(1);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 1);

    res = ht.erase(-2);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 0);
    EXPECT_EQ(ht.dbgCountBranches(), 1);
}

TEST(HashTrie, eraseChain)
{
    typedef int key_type;
    typedef int value_type;
    typedef lfds::hash_trie<key_type, value_type, 16, TheWorstHashFunc> hash_trie;
    typedef hash_trie::size_type size_type;

    hash_trie ht;

    bool res = false;
    size_type size = 0;

    res = ht.erase(0);
    EXPECT_FALSE(res);

    ht.insert(0, 0);
    ht.insert(1, 1);
    ht.insert(-2, -2);

    res = ht.erase(0);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 2);

    res = ht.erase(0);
    EXPECT_FALSE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 2);

    res = ht.erase(1);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 1);

    res = ht.erase(-2);
    EXPECT_TRUE(res);
    EXPECT_FALSE(ht.dbgCheckRefrences());
    size = ht.size();
    EXPECT_EQ(size, 0);
    EXPECT_EQ(ht.dbgCountBranches(), 1);
}
