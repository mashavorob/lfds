/*
 * ctrie.h
 *
 *  Created on: Mar 12, 2015
 *      Author: masha
 */

#ifndef INCLUDE_HASH_TRIE_HPP_
#define INCLUDE_HASH_TRIE_HPP_

#include "cas.hpp"
#include "pool_buffer.hpp"
#include "meta_utils.hpp"
#include "ref_lock.hpp"
#include "inttypes.hpp"
#include "xtomic.hpp"
#include "xfunctional.hpp"
#include "cppbasics.hpp"

#include <cassert>

namespace lfds
{

namespace htrie
{

template<int BFactor>
struct get_shift_size;

template<int BFactor>
struct get_mask;

template<class HashType, int BFactor>
struct get_max_level;

template<>
struct get_shift_size<16>
{
    static constexpr int value = 4;
};

template<>
struct get_shift_size<32>
{
    static constexpr int value = 5;
};

template<>
struct get_shift_size<64>
{
    static constexpr int value = 6;
};

template<>
struct get_shift_size<128>
{
    static constexpr int value = 7;
};

template<>
struct get_shift_size<256>
{
    static constexpr int value = 8;
};

template<>
struct get_mask<16>
{
    static constexpr int value = 0xf;
};

template<>
struct get_mask<32>
{
    static constexpr int value = 0x1f;
};

template<>
struct get_mask<64>
{
    static constexpr int value = 0x3f;
};

template<>
struct get_mask<128>
{
    static constexpr int value = 0x7f;
};

template<>
struct get_mask<256>
{
    static constexpr int value = 0xff;
};

namespace
{
template<std::size_t val, std::size_t shift>
struct get_nested_level
{
    static constexpr int value = get_nested_level<(val >> shift), shift>::value
            + 1;
};

template<std::size_t shift>
struct get_nested_level<0, shift>
{
    static constexpr int value = 0;
};

template<class T>
struct get_mask_by
{
    typedef typename get_uint_by_size<sizeof(T)>::type u_type;

    static constexpr std::size_t get()
    {
        return static_cast<std::size_t>(static_cast<u_type>(-1));
    }
};
}

template<class HashType, int BFactor>
struct get_max_level;

template<class HashType>
struct get_max_level<HashType, 16>
{
    enum
    {
        value = sizeof(HashType)*2
    };
};

template<class HashType>
struct get_max_level<HashType, 256>
{
    enum
    {
        value = sizeof(HashType)
    };
};


typedef std::size_t counter_type;
typedef xtomic<counter_type> atomic_counter_type;

enum NodeType
{
    branch, chain,
};

struct Node
{

    Node(NodeType type) :
            m_type(type)
    {

    }

    const NodeType m_type;
};

class __attribute__((aligned(sizeof(void*)*2))) NodePtr
{
public:
    typedef get_uint_by_size<sizeof(Node*) / 2>::type counter_type;

    NodePtr() :
            m_node(nullptr),
            m_abaCount(0),
            m_refCount(0)
    {
    }

    NodePtr(Node* n, const NodePtr & other) :
            m_node(n),
            m_abaCount(other.m_abaCount.load(barriers::relaxed) + 1),
            m_refCount(0)
    {
    }
    NodePtr(const NodePtr & other) :
            m_node(other.m_node.load(barriers::relaxed)),
            m_abaCount(other.m_abaCount.load(barriers::relaxed)),
            m_refCount(0)
    {
    }
    NodePtr(Node* n, const counter_type abaCount) :
            m_node(n),
            m_abaCount(abaCount),
            m_refCount(0)
    {

    }
    Node* node()
    {
        return m_node.load(barriers::relaxed);
    }
    const Node* node() const
    {
        return m_node.load(barriers::relaxed);
    }
    void node(Node* p)
    {
        m_node.store(p, barriers::relaxed);
    }
    counter_type add_ref() const
    {
        return ++m_refCount;
    }
    counter_type release() const
    {
        return --m_refCount;
    }
    bool isReferenced() const
    {
        return m_refCount.load(barriers::relaxed) != 0;
    }
    void wait() const
    {
        while (m_refCount.load(barriers::relaxed) > 0)
            ;
    }
    bool atomic_cas(const NodePtr & expected, const NodePtr & replacement)
    {
        return lfds::atomic_cas(*this, expected, replacement);
    }
    bool atomic_cas_strong(const NodePtr & expected,
                           const NodePtr & replacement)
    {
        while (expected == *this)
        {
            wait();
            if (atomic_cas(expected, replacement))
            {
                return true;
            }
        }
        return false;
    }
    counter_type getAbaCount() const
    {
        return m_abaCount.load(barriers::relaxed);
    }
    bool isEqual(const NodePtr & other) const
    {
        return m_node.load(barriers::relaxed)
                == other.m_node.load(barriers::relaxed)
                && m_abaCount.load(barriers::relaxed)
                        == other.m_abaCount.load(barriers::relaxed);
    }
    bool operator==(const NodePtr & other) const
    {
        return isEqual(other);
    }
    bool operator!=(const NodePtr & other) const
    {
        return !isEqual(other);
    }
private:
    xtomic<Node*> m_node;                      // generic node
    xtomic<counter_type> m_abaCount;           // aba protection
    mutable xtomic<counter_type> m_refCount;   // reference count
};

// chain node
template<class Key, class Value, class HashType>
struct CNode: public Node
{
    typedef Key key_type;
    typedef Value value_type;
    typedef HashType hash_type;
    typedef CNode<key_type, value_type, hash_type> l_type;

    CNode() :
            Node(chain),
            m_next(nullptr),
            m_hash(),
            m_allocated(true)
    {

    }

    l_type* m_next;
    hash_type m_hash;

    xtomic<bool> m_allocated;

    key_type* key()
    {
        return reinterpret_cast<key_type*>(m_keyBuff);
    }
    const key_type* key() const
    {
        return reinterpret_cast<const key_type*>(m_keyBuff);
    }
    value_type* value()
    {
        return reinterpret_cast<value_type*>(m_valueBuff);
    }
    const value_type* value() const
    {
        return reinterpret_cast<const value_type*>(m_valueBuff);
    }
private:
    char m_keyBuff[sizeof(key_type)] __attribute__((aligned(__alignof(key_type))));
    char m_valueBuff[sizeof(value_type)] __attribute__((aligned(__alignof(value_type))));
private:
    CNode(const l_type & other); // = delete;
    l_type & operator=(const l_type & other); // = delete;
};

// branch node
template<class Key, class Value, class HashType, int BFactor>
struct BNode: public Node
{
    typedef Key key_type;
    typedef Value value_type;
    typedef HashType hash_type;
    typedef xtomic<hash_type> atomic_hash_type;
    typedef NodePtr ptr_type;
    typedef BNode<key_type, value_type, hash_type, BFactor> b_type;
    typedef CNode<key_type, value_type, hash_type> c_type;
    typedef std::size_t counter_type;

    static constexpr int SIZE = BFactor;

    b_type* m_parent;
    mutable xtomic<counter_type> m_refCount;
    ptr_type m_array[SIZE];

private:
    BNode(const b_type & other); // = delete;
    b_type & operator=(const b_type & other); // = delete;
public:
    BNode() :
            Node(branch),
            m_parent(nullptr),
            m_refCount(0)
    {

    }

    counter_type add_ref() const
    {
        return ++m_refCount;
    }
    counter_type release() const
    {
        return --m_refCount;
    }
    void wait()
    {
        while (m_refCount.load(barriers::relaxed))
            ;
    }
};

template<class T, class Hash, int = sizeof(std::size_t)>
struct hash_adapter;

template<class T, class Hash>
struct hash_adapter<T, Hash, 4>
{
public:
    typedef uint32_t hash_type;

    hash_type operator()(const T& val) const
    {
        return m_hashFunc(val);
    }
private:
    Hash m_hashFunc;
};

template<class T, class Hash>
struct hash_adapter<T, Hash, 8>
{
private:
    union Dummy
    {
        struct
        {
            uint32_t h;
            uint32_t l;
        } s;
        std::size_t v;
    };
public:
    typedef uint32_t hash_type;

    hash_type operator()(const T& val) const
    {
        Dummy dummy;
        dummy.v = m_hashFunc(val);
        return dummy.s.h + dummy.s.l;
    }
private:
    Hash m_hashFunc;
};

}

template<class Key, class Value, int BFactor = 16, class Hash = typename get_hash<Key>::type,
        class Pred = std::equal_to<Key>,
        class Allocator = std::allocator<Value> >
class hash_trie
{
public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef std::pair<key_type, mapped_type> value_type;
    typedef Hash hash_base_type;
    typedef Pred predicate_type;
    typedef Allocator allocator_type;

    static constexpr int BFACTOR = BFactor; // branching factor

    typedef hash_trie<key_type, mapped_type, BFACTOR, hash_base_type, predicate_type,
            allocator_type> this_type;

    typedef htrie::hash_adapter<key_type, hash_base_type> hash_func_type;
    typedef typename hash_func_type::hash_type hash_type;

    static constexpr int NFACTOR =
            htrie::get_max_level<hash_type, BFACTOR>::value;

    typedef htrie::Node n_type;
    typedef htrie::NodePtr ptr_type;
    typedef htrie::BNode<key_type, mapped_type, hash_type, BFACTOR> b_type;
    typedef htrie::CNode<key_type, mapped_type, hash_type> c_type;

    typedef pool_buffer<b_type, allocator_type> b_buffer_type;
    typedef pool_buffer<c_type, allocator_type> c_buffer_type;

    typedef typename Allocator::template rebind<key_type>::other key_allocator_type;
    typedef typename Allocator::template rebind<mapped_type>::other mapped_allocator_type;

    typedef std::size_t size_type;

private:
    hash_trie(const this_type &); // = delete;
    this_type& operator=(const this_type &); // = delete;

    enum Return
    {
        failed, succeeded, proceed, retry,
    };

    static constexpr int POOL_SIZE = 16;

    typedef ref_lock<b_type> b_lock;
    typedef ref_lock<ptr_type> ptr_lock;

    typedef ref_lock<const b_type> cb_lock;
    typedef ref_lock<const ptr_type> cptr_lock;

public:
    hash_trie() :
            m_size(0),
            m_bsize(0),
            m_b_buffer(POOL_SIZE),
            m_c_buffer(POOL_SIZE),
            m_eqFunc(),
            m_hashFunc()
    {

    }
    ~hash_trie()
    {
        assert(!dbgCheckRefrences());
    }
    bool find(const key_type & key, mapped_type & val) const
    {
        const hash_type hash = m_hashFunc(key);
        hash_type shash = hash; // shifted hash

        constexpr hash_type mask = htrie::get_mask<BFACTOR>::value;
        cb_lock bn(m_root);

        const int index = hash & mask;
        cptr_lock ptr(bn->m_array[index]);

        for (;;)
        {
            const n_type* p = ptr->node();
            if (!p)
            {
                return false;
            }
            else if (p->m_type == htrie::branch)
            {
                traverse(bn, ptr, p, shash);
            }
            else if (p->m_type == htrie::chain)
            {
                return findInChain(bn, ptr, p, hash, key, val);
            }
        }
        assert(false); // shit happens
        return false;
    }
#if LFDS_USE_CPP11
    template<class ... Args>
    bool insert(const key_type & key, Args&&... val)
#else
    bool insert(const key_type & key, const mapped_type& val)
#endif
    {
        constexpr hash_type mask = htrie::get_mask<BFACTOR>::value;
        constexpr int shift = htrie::get_shift_size<BFACTOR>::value;
        const hash_type hash = m_hashFunc(key);

        Return ret = proceed;
        hash_type shash = hash; // shifted hash
        int level = 0;
        const int index = shash & mask;
        b_lock bn(m_root);
        ptr_lock ptr(bn->m_array[index]);

        for (;;)
        {
            n_type* p = ptr->node();
            if (!p)
            {
                ret = insertChain(bn, ptr, p, hash, key, std_forward(Args, val));
            }
            else if (p->m_type == htrie::branch)
            {
                traverse(bn, ptr, p, shash);
                ++level;
            }
            else if (p->m_type == htrie::chain)
            {
                ret = insertBranch(bn, ptr, p, level, shash, hash, key, std_forward(Args, val));
            }
            switch (ret)
            {
            case failed:
                return false;
            case succeeded:
                ++m_size;
                return true;
            case proceed:
                break;
            case retry:
                ret = proceed;
                shash = hash;
                level = 0;
                bn.swap(m_root);
                ptr.swap(bn->m_array[index]);
                break;
            default:
                assert(false); // shit happens
            }
        }
        assert(false); // shit happens
        return false;
    }
    bool erase(const key_type & key)
    {
        constexpr hash_type mask = htrie::get_mask<BFACTOR>::value;
        constexpr int shift = htrie::get_shift_size<BFACTOR>::value;
        const hash_type hash = m_hashFunc(key);

        Return ret = proceed;
        int level = 0;
        hash_type shash = hash; // shifted hash
        const int index = shash & mask;
        b_lock bn(m_root);
        ptr_lock ptr(bn->m_array[index]);

        for (;;)
        {
            n_type* p = ptr->node();
            if (!p)
            {
                return false;
            }
            else if (p->m_type == htrie::branch)
            {
                traverse(bn, ptr, p, shash);
                ++level;
            }
            else if (p->m_type == htrie::chain)
            {
                ret = eraseFromChain(bn, ptr, p, level, hash, key);
            }

            switch (ret)
            {
            case failed:
                return false;
            case succeeded:
                --m_size;
                return true;
            case proceed:
                break;
            case retry:
                ret = proceed;
                shash = hash;
                level = 0;
                bn.swap(m_root);
                ptr.swap(bn->m_array[index]);
                break;
            default:
                assert(false); // shit happens
            }
        }
        assert(false); // shit happens
        return false;
    }
    size_type size() const
    {
        return m_size.load(barriers::relaxed);
    }
    // diagnosis
public:
    bool dbgCheckRefrences() const
    {
        return dbgCheckRefrencesImpl(&m_root);
    }
    size_type dbgCountBranches() const
    {
        return dbgCountBranchesImpl(&m_root);
    }

private:
    bool findInChain(cb_lock& bn,
                     cptr_lock& ptr,
                     const n_type* p,
                     const hash_type hash,
                     const key_type & key,
                     mapped_type & val) const
    {
        const c_type* cn = reinterpret_cast<const c_type*>(p);

        while (cn)
        {
            if (cn->m_hash == hash && m_eqFunc(key, *cn->key()))
            {
                if (!cn->m_allocated.load(barriers::relaxed))
                {
                    break;
                }
                val = *cn->value();
                return true;
            }
            cn = cn->m_next;
        }
        return false;
    }
#if LFDS_USE_CPP11
    template<class ... Args>
    Return insertChain(b_lock& bn,
                       ptr_lock& ptr,
                       n_type* p,
                       const hash_type hash,
                       const key_type & key,
                       Args&&... val)
#else
    Return insertChain(b_lock& bn,
                       ptr_lock& ptr,
                       n_type* p,
                       const hash_type hash,
                       const key_type & key,
                       const mapped_type& val)
#endif
    {
        // check if the value already exists
        const c_type* cn = reinterpret_cast<c_type*>(p);

        while (cn)
        {
            if (cn->m_hash == hash && m_eqFunc(key, *cn->key()))
            {
                bool result = cn->m_allocated.load(barriers::relaxed);
                if (!result)
                {
                    break;
                }
                return failed; // false if an concurent insert has finished it before us
            }
            cn = cn->m_next;
        }

        c_type* cnn = m_c_buffer.allocate();

        cnn->m_hash = hash;
        m_keyAllocator.construct(cnn->key(), key);
        m_mappedAllocator.construct(cnn->value(), std_forward(Args, val));
        cnn->m_next = reinterpret_cast<c_type*>(p);

        ptr_type ptre = *ptr;
        ptr_type ptrn(cnn, ptre);

        Return ret;
        ptr_type& raw_ptr = *ptr;
        ptr.swap();
        if (raw_ptr.atomic_cas(ptre, ptrn))
        {
            ret = succeeded;
        }
        else
        {
            ret = retry;
            // clear the mess up
            m_keyAllocator.destroy(cnn->key());
            m_mappedAllocator.destroy(cnn->value());
            m_c_buffer.deallocate(cnn);
        }
        return ret;
    }
#if LFDS_USE_CPP11
    template<class ... Args>
    Return insertBranch(b_lock& bn,
                        ptr_lock& ptr,
                        n_type* p,
                        int& level,
                        hash_type& shash,
                        const hash_type hash,
                        const key_type & key,
                        Args&&... val)
#else
    Return insertBranch(b_lock& bn,
                        ptr_lock& ptr,
                        n_type* p,
                        int& level,
                        hash_type& shash,
                        const hash_type hash,
                        const key_type & key,
                        const mapped_type& val)
#endif
    {
        // not empty chain is expected
        c_type* cn = reinterpret_cast<c_type*>(p);

        // check if we can use it rather then transform it to a new branch
        if (!cn->m_hash == hash || level == NFACTOR)
        {
            Return ret = insertChain(bn, ptr, p, hash, key, std_forward(Args, val));
            return ret;
        }

        // chain needs to be transformed into a new branch
        ++level;

        constexpr hash_type mask = htrie::get_mask<BFACTOR>::value;
        constexpr int shift = htrie::get_shift_size<BFACTOR>::value;

        hash_type chainedHash = cn->m_hash;
        const int chainedIndex = (chainedHash >> (shift * level)) & mask;

        b_type* bnn = m_b_buffer.allocate();

        bnn->m_parent = bn.get();
        bnn->m_array[chainedIndex].node(cn);

        ptr_type ptre = *ptr;
        ptr_type ptrn(bnn, ptre);

        ptr_type & raw_ptr = *ptr;
        ptr.swap();
        if (raw_ptr.atomic_cas(ptre, ptrn))
        {
            shash >>= shift;
            const int index = shash & mask;

            bn.swap(*bnn);
            ptr.swap(bnn->m_array[index]);
            return proceed;
        }

        m_b_buffer.deallocate(bnn);
        return retry;
    }
    Return eraseFromChain(b_lock& bn,
                          ptr_lock& ptr,
                          n_type* p,
                          int level,
                          const hash_type hash,
                          const key_type & key)
    {
        c_type* head = reinterpret_cast<c_type*>(p);
        c_type* cn = head;

        while (cn)
        {
            if (cn->m_hash == hash && m_eqFunc(key, *cn->key())
                    && cn->m_allocated.load(barriers::relaxed))
            {
                cn->m_allocated.store(false, barriers::relaxed);
                break;
            }
            cn = cn->m_next;
        }
        Return ret = cn ? succeeded : failed;

        // try to clear from the head
        cn = head;

        while (cn && !cn->m_allocated.load(barriers::relaxed))
        {
            cn = cn->m_next;
        }

        // cn is the first allocated node in the chain

        if (cn != head)
        {
            ptr_type ptre = *ptr;
            ptr_type ptrn(cn, ptre);

            ptr_type & raw_ptr = *ptr;
            ptr.swap();
            raw_ptr.atomic_cas_strong(ptre, ptrn);
        }

        // try to tear down the whole branch
        while (bn.get() != &m_root)
        {
            // check if branch is empty
            for (int i = 0; i < BFACTOR; ++i)
            {
                ptr_type & item = bn->m_array[i];
                item.add_ref();
                if (item.node() != nullptr)
                {
                    do
                    {
                        bn->m_array[i].release();
                        --i;
                    }
                    while (i >= 0);
                    return ret;
                }
            }

            b_lock parent(*bn->m_parent);

            --level;

            constexpr hash_type mask = htrie::get_mask<BFACTOR>::value;
            const int shift = htrie::get_shift_size<BFACTOR>::value * level;
            const int index = (hash >> shift) & mask;

            ptr_type* p_ptr = &parent->m_array[index];

            b_type* raw_b = bn.get();

            ptr_type p_ptre(raw_b, p_ptr->getAbaCount());
            ptr_type p_ptrn(nullptr, p_ptre);
            if (!p_ptr->atomic_cas_strong(p_ptre, p_ptrn))
            {
                return ret;
            }

            bn.swap(parent);
            parent.swap();
            raw_b->wait();
            m_b_buffer.deallocate(raw_b);
        }
        return ret;
    }

    void traverse(cb_lock& bn,
                  cptr_lock& ptr,
                  const n_type* p,
                  hash_type & hash) const
    {
        constexpr hash_type mask = htrie::get_mask<BFACTOR>::value;
        constexpr int shift = htrie::get_shift_size<BFACTOR>::value;

        hash >>= shift;
        const hash_type index = hash & mask;

        cb_lock bnn(*reinterpret_cast<const b_type*>(p));
        cptr_lock ptrn(bnn->m_array[index]);

        bn.swap(bnn);
        ptr.swap(ptrn);
    }
    void traverse(b_lock& bn, ptr_lock& ptr, n_type* p, hash_type & hash)
    {
        constexpr hash_type mask = htrie::get_mask<BFACTOR>::value;
        constexpr int shift = htrie::get_shift_size<BFACTOR>::value;

        hash >>= shift;
        const hash_type index = hash & mask;

        b_lock bnn(*reinterpret_cast<b_type*>(p));
        ptr_lock ptrn(bnn->m_array[index]);
        bn.swap(bnn);
        ptr.swap(ptrn);
    }
    bool dbgCheckRefrencesImpl(const b_type* bn) const
    {
        if (bn->m_refCount.load(barriers::relaxed))
        {
            return true;
        }
        for (int i = 0; i < BFACTOR; ++i)
        {
            const ptr_type *ptr = &bn->m_array[i];
            if (ptr->isReferenced())
            {
                return true;
            }
            const n_type* node = bn->m_array[i].node();
            if (!node || node->m_type != htrie::branch)
            {
                continue;
            }
            const b_type* nested = reinterpret_cast<const b_type*>(node);
            if (dbgCheckRefrencesImpl(nested))
            {
                return true;
            }
        }
        return false;
    }
    size_type dbgCountBranchesImpl(const b_type* bn) const
    {
        size_type count = 1;
        for (int i = 0; i < BFACTOR; ++i)
        {
            const n_type* node = bn->m_array[i].node();
            if (!node || node->m_type != htrie::branch)
            {
                continue;
            }
            const b_type* nested = reinterpret_cast<const b_type*>(node);
            count += dbgCountBranchesImpl(nested);
        }
        return count;
    }

private:
    b_type m_root;
    xtomic<size_type> m_size;
    xtomic<size_type> m_bsize;

    key_allocator_type m_keyAllocator;
    mapped_allocator_type m_mappedAllocator;
    b_buffer_type m_b_buffer;
    c_buffer_type m_c_buffer;
    const hash_func_type m_hashFunc;
    const predicate_type m_eqFunc;
}
;
}

#endif /* INCLUDE_HASH_TRIE_HPP_ */
