
#ifndef UNTITLED_UTIL_H
#define UNTITLED_UTIL_H

#include<atomic>
#include <random>
#include<vector>
#include<stdint.h>
//#include "VersionTracker/TrackerList.h"

// namespace kanva_impl {
const int64_t child_max = 32;
const int64_t root_max = 32;


const int64_t MIN = 8;

std::atomic<int> curr_ts;

int NUM_THREADS;
int HELPING_DELAY = 1;
int MAX_FAILURE = 1;
std::vector<int64_t> worst_case_resp_time;
thread_local int FAILURE = 0;
thread_local int cnt = 0;
thread_local int ttid;
bool expt_sleep = false;
thread_local std::uniform_int_distribution<int> dice(1, 10);
thread_local std::random_device rd;
thread_local std::mt19937 gen(rd());

template<typename V>
class Vnode{
public:
    V value;
    std::atomic<int64_t> ts;
    std::atomic<Vnode*> nextv;
    Vnode(){
        this -> value = std::rand();
        nextv.store(nullptr);
        ts = -1;
    }
    Vnode(V val){
        this -> value = val;
        nextv.store(nullptr);
        ts = -1;
    }
    Vnode(V val, Vnode<V>* n){
        this -> value = val;
        nextv.store(n);
        ts = -1;
    }
};

template<typename K, typename V>
class ll_Node{
public:
    K key;
    V value;
    std::atomic<ll_Node<K,V>*> next;
    ll_Node(K __key, V __value)
    {
        key = __key;
        value = __value;
        next.store(nullptr, std::memory_order_seq_cst);
    }

    ll_Node(K __key, V __value, ll_Node<K,V>* next)
    {
        key = __key;
        value = __value;
        this -> next.store(next, std::memory_order_seq_cst);
    }
};

ll_Node<int64_t, int64_t>* dummy_node = new ll_Node<int64_t, int64_t>(-1, -1);



int64_t is_freeze(uintptr_t i)
{
    return (int64_t)(i & (uintptr_t)0x02);
}


int64_t is_marked(uintptr_t i)
{
    return (int64_t)(i & (uintptr_t)0x01);
}

uintptr_t set_freeze(uintptr_t i)
{
    return (i | (uintptr_t)0x02);
}

uintptr_t set_mark(uintptr_t i)
{
    return (i | (uintptr_t)0x01);
}

uintptr_t unset_freeze(uintptr_t i)
{
    return (i & ~(uintptr_t)0x02);
}

uintptr_t unset_freeze_mark(uintptr_t i)
{
    return (i & ~(uintptr_t)0x03);
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
//
//TrackerList version_tracker;
//}
#endif //UNTITLED_UTIL_H
