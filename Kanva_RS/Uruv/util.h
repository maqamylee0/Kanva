
#ifndef UNTITLED_UTIL_H
#define UNTITLED_UTIL_H

#include<atomic>
#include <random>
#include<vector>
#include<stdint.h>
#include "VersionTracker/TrackerList.h"

const int64_t MAX = 32;
const int64_t MIN = 8;

typedef int thread_id_t;

int NUM_THREADS;



thread_local int cnt = 0;
thread_local int ttid;
bool expt_sleep = false;
thread_local std::uniform_int_distribution<int> dice(1, 10);
thread_local std::random_device rd;
thread_local std::mt19937 gen(rd());

template<typename ValT>
class Vnode{
public:
    ValT value;
    std::atomic<int64_t> ts;
    std::atomic<Vnode*> nextv;
    Vnode(){
        this -> value = std::rand();
        nextv.store(nullptr);
        ts = -1;
    }
    Vnode(ValT val){
        this -> value = val;
        nextv.store(nullptr);
        ts = -1;
    }
    Vnode(ValT val, Vnode<ValT>* n){
        this -> value = val;
        nextv.store(n);
        ts = -1;
    }
    void init(ValT val, Vnode<ValT>* n = nullptr) {
        this -> value = val;
        nextv.store(n);
        ts = -1;
    }
};

template<typename KeyT, typename ValT>
class ll_Node{
public:
    KeyT key;
    std::atomic<Vnode<ValT>*> vhead;
    std::atomic<ll_Node<KeyT,ValT>*> next;
    std::atomic<int64_t> del;
    // TODO: Delete this
    ll_Node(KeyT key, ValT value)
    {
        this -> key = key;
        Vnode<ValT> *temp = new Vnode(value);
        vhead.store(temp, std::memory_order_seq_cst);
        next.store(nullptr, std::memory_order_seq_cst);
        del = 0;
    }

    /*
    ll_Node(KeyT key, ValT value, ll_Node<KeyT,ValT>* next)
    {
        this -> key = key;
        Vnode<ValT> *temp = new Vnode<ValT>(value);
        vhead.store(temp, std::memory_order_seq_cst);
        this -> next.store(next, std::memory_order_seq_cst);
        del = 0;
    }
    ll_Node(KeyT key, Vnode<ValT>* vhead, ll_Node<KeyT,ValT>* next)
    {
        this -> key = key;
        this -> vhead.store(vhead, std::memory_order_seq_cst);
        this -> next.store(next, std::memory_order_seq_cst);
        del = 0;
    }
    */
    ll_Node() {}
    void init(KeyT key, ValT value) {
        this -> key = key;
        Vnode<ValT> *temp = new Vnode(value);
        vhead.store(temp, std::memory_order_seq_cst);
        next.store(nullptr, std::memory_order_seq_cst);
        del = 0;
    }
};

ll_Node<int64_t, int64_t>* dummy_node = new ll_Node<int64_t, int64_t>(-1, -1);

inline int64_t is_freeze(uintptr_t i)
{
    return (int64_t)(i & (uintptr_t)0x02);
}
inline uintptr_t set_freeze(uintptr_t i)
{
    return (i | (uintptr_t)0x02);
}

inline uintptr_t unset_freeze(uintptr_t i)
{
    return (i & ~(uintptr_t)0x02);
}

inline uintptr_t unset_freeze_mark(uintptr_t i)
{
    return (i & ~(uintptr_t)0x03);
}

int64_t is_marked(uintptr_t i)
{
    return (int64_t)(i & (uintptr_t)0x01);
}

uintptr_t set_mark(uintptr_t i)
{
    return (i | (uintptr_t)0x01);
}


uintptr_t unset_mark(uintptr_t i)
{
    return (i & ~(uintptr_t)0x01);
}

int64_t is_marked_ref(long i)
{
    /* return (int64_t) (i & (LONG_MIN+1)); */
    return (int64_t) (i & 0x1L);
}

long unset_mark(long i)
{
    /* i &= LONG_MAX-1; */
    i &= ~0x1L;
    return i;
}

long set_mark(long i)
{
    /* i = unset_mark(i); */
    /* i += 1; */
    i |= 0x1L;
    return i;
}

long get_unmarked_ref(long w)
{
    /* return unset_mark(w); */
    return w & ~0x1L;
}

long get_unfreezed_unmarked_ref(long w){
    return w & ~0x3L;
}

//TrackerList version_tracker;

#endif //UNTITLED_UTIL_H
