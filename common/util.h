#ifndef KANVA_UTIL_H
#define KANVA_UTIL_H
#include <getopt.h>
#include "iostream"
#include "stdlib.h"
#include "vector"
#include "assert.h"
#include "algorithm"
#include "fstream"
#include "atomic"
#include "thread"
#include "unistd.h"

typedef uint64_t key_type;
typedef uint64_t val_type;
#define COUT_THIS(this) std::cout << this << std::endl;
#define COUT_POS() COUT_THIS("at " << __FILE__ << ":" << __LINE__)
#define COUT_VAR(this) std::cout << #this << ": " << this << std::endl;

#define INVARIANT(cond)            \
  if (!(cond)) {                   \
    COUT_THIS(#cond << " failed"); \
    COUT_POS();                    \
    abort();                       \
  }

inline void memory_fence() { asm volatile("mfence" : : : "memory"); }

/** @brief Compiler fence.
 * Prevents reordering of loads and stores by the compiler. Not intended to
 * synchronize the processor's caches. */
inline void fence() { asm volatile("" : : : "memory"); }
inline uint64_t cmpxchg(uint64_t *object, uint64_t expected,
                        uint64_t desired) {
    asm volatile("lock; cmpxchgq %2,%1"
            : "+a"(expected), "+m"(*object)
            : "r"(desired)
            : "cc");
    fence();
    return expected;
}

inline uint8_t cmpxchgb(uint8_t *object, uint8_t expected,
                        uint8_t desired) {
    asm volatile("lock; cmpxchgb %2,%1"
            : "+a"(expected), "+m"(*object)
            : "r"(desired)
            : "cc");
    fence();
    return expected;
}
enum class Result { ok, failed, retry, retrain };
typedef Result result_t;
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define NS_PER_S 1000000000.0
#define TIMER_DECLARE(n) struct timespec b##n,e##n
#define TIMER_BEGIN(n) clock_gettime(CLOCK_MONOTONIC, &b##n)
#define TIMER_END_NS(n,t) clock_gettime(CLOCK_MONOTONIC, &e##n); \
    (t)=(e##n.tv_sec-b##n.tv_sec)*NS_PER_S+(e##n.tv_nsec-b##n.tv_nsec)
#define TIMER_END_S(n,t) clock_gettime(CLOCK_MONOTONIC, &e##n); \
    (t)=(e##n.tv_sec-b##n.tv_sec)+(e##n.tv_nsec-b##n.tv_nsec)/NS_PER_S
#define COUT_N_EXIT(msg) \
  COUT_THIS(msg);        \
  COUT_POS();            \
  abort();

#define CACHELINE_SIZE (1 << 6)
std::vector<key_type> exist_keys;
std::vector<val_type> data1;
#define FORCEINLINE __attribute__((always_inline)) inline
FORCEINLINE uint32_t bsr(uint32_t x) {
return 31 - __builtin_clz(x);
}
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
#endif //KANVA_UTIL_H
