#ifndef __KANVA_RS_UTIL_H__
#define __KANVA_RS_UTIL_H__

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <climits>
#include <immintrin.h>
#include <cassert>
#include <random>
#include <memory>
#include <array>
#include <time.h>
#include <unistd.h>
#include <atomic>
#include <getopt.h>
#include <unistd.h>
#include <algorithm>
#include <mutex>
#include "Uruv/util.h"
#include "Uruv/VersionTracker/TrackerList.h"

#define NS_PER_S 1000000000.0
#define TIMER_DECLARE(n) struct timespec b##n,e##n
#define TIMER_BEGIN(n) clock_gettime(CLOCK_MONOTONIC, &b##n)
#define TIMER_END_NS(n,t) clock_gettime(CLOCK_MONOTONIC, &e##n); \
    (t)=(e##n.tv_sec-b##n.tv_sec)*NS_PER_S+(e##n.tv_nsec-b##n.tv_nsec)
#define TIMER_END_S(n,t) clock_gettime(CLOCK_MONOTONIC, &e##n); \
    (t)=(e##n.tv_sec-b##n.tv_sec)+(e##n.tv_nsec-b##n.tv_nsec)/NS_PER_S

#define COUT_THIS(this) std::cout << this << std::endl;
#define COUT_VAR(this) std::cout << #this << ": " << this << std::endl;
#define COUT_POS() COUT_THIS("at " << __FILE__ << ":" << __LINE__)
#define COUT_N_EXIT(msg) \
  COUT_THIS(msg);        \
  COUT_POS();            \
  abort();
#define INVARIANT(cond)            \
  if (!(cond)) {                   \
    COUT_THIS(#cond << " failed"); \
    COUT_POS();                    \
    abort();                       \
  }

#if defined(NDEBUGGING)
#define DEBUG_THIS(this)
#else
#define DEBUG_THIS(this) std::cerr << this << std::endl
#endif

//enum class Result { ok, failed, retry, retrain };
//typedef Result result_t;

#define CACHELINE_SIZE (1 << 6)

// ==================== memory fence =========================
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)




// ========================= seach-schemes ====================

#define SHUF(i0, i1, i2, i3) (i0 + i1*4 + i2*16 + i3*64)
#define FORCEINLINE __attribute__((always_inline)) inline

// power of 2 at most x, undefined for x == 0
//FORCEINLINE uint32_t bsr(uint32_t x) {
// return 31 - __builtin_clz(x);
//}

/*
template<typename KEY_TYPE>
static int binary_search_branchless(const KEY_TYPE *arr, int n, KEY_TYPE key) {
//static int binary_search_branchless(const int *arr, int n, int key) {
  intptr_t pos = -1;
  intptr_t logstep = bsr(n - 1);
  intptr_t step = intptr_t(1) << logstep;

  pos = (arr[pos + n - step] < key ? pos + n - step : pos);
  step >>= 1;

  while (step > 0) {
    pos = (arr[pos + step] < key ? pos + step : pos);
    step >>= 1;
  }
  pos += 1;

  return (int) (arr[pos] >= key ? pos : n);
}
*/

template<class key_t, class val_t> //CHECK:class or typename? no diff between the two (most likely)
class VersionedArray{

  public:
    key_t key;
    std::atomic<Vnode<val_t>*> vhead;

    VersionedArray(key_t key,val_t value){
        this -> key = key;
        Vnode<val_t> *temp = new Vnode(value);
        vhead.store(temp, std::memory_order_seq_cst);
        
    }

    VersionedArray(key_t key, Vnode<val_t>* vhead)
    {
        this -> key = key;
        this -> vhead.store(vhead, std::memory_order_seq_cst);
        
    }

    val_t getValue(){
      return vhead.load(std::memory_order_seq_cst)->value;
    }

    void init_ts(Vnode<val_t> *node,TrackerList *version_tracker){
        if(node ->ts.load(std::memory_order_seq_cst) == -1){
            int64_t invalid_ts = -1;
            int64_t global_ts = (*version_tracker).get_latest_timestamp();
            node -> ts.compare_exchange_strong(invalid_ts, global_ts, std::memory_order_seq_cst, std::memory_order_seq_cst );
        }
    }

    val_t read(TrackerList *version_tracker) {
        init_ts(vhead,version_tracker);
        return vhead.load(std::memory_order_seq_cst) -> value;
    }

    bool vCAS(val_t old_value,val_t new_value,TrackerList *version_tracker){
        Vnode<val_t>* head = (Vnode<val_t>*) unset_mark((uintptr_t)vhead.load(std::memory_order_seq_cst));
        init_ts(head,version_tracker);
        if(head -> value != old_value) return false;
		    if(head -> value == new_value)
			    return true;
        Vnode<val_t>* new_node = new Vnode<val_t>(new_value, head);
        
        if(vhead.compare_exchange_strong(head,new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            init_ts(new_node,version_tracker);
            return true;
        }
        else{
            //delete new_node;
            return false;
        }
    }

    bool insert(val_t value,TrackerList *version_tracker){
      
      while(true)
      {
        val_t curr_value = read(version_tracker); //read value of current node w.r.t timestamp
        if(curr_value == value)
          return false; //already present
        if(vCAS(curr_value,value,version_tracker)) //at vhead
            break;
        //else if(is_marked_ref((uintptr_t)vhead.load(std::memory_order_seq_cst)))
            //return false;
      }
      return 0;
  
    }

    val_t getVersionedValue(TrackerList *version_tracker,int64_t curr_ts){

      Vnode<val_t>* curr_vhead = (Vnode<val_t>*) get_unmarked_ref((uintptr_t) vhead.load(std::memory_order_seq_cst));
      init_ts(curr_vhead,version_tracker);
      while(curr_vhead && curr_vhead -> ts >= curr_ts) curr_vhead = curr_vhead -> nextv;
      if(curr_vhead && curr_vhead  -> value != -1){
            //std::cout<<"Range query in level bin"<<std::endl;
            return curr_vhead->value;
      }
      return -1;
    }

};



#endif
