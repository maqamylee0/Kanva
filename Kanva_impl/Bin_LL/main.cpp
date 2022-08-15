#include <iostream>
#include "atomic"
#include "thread"
#include "vector"
#include "unistd.h"
#include "LF_LL.h"
#include "Bin.h"

std::atomic<int> count;
int64_t x = 0;
int64_t y = -1;
std::vector<int64_t> thput;
bool running = false;
std::atomic<Linked_List<int64_t, int64_t>*> list;
std::atomic<Bin<int64_t, int64_t>*> a;

//std::atomic<uintptr_t> ptr;

void LL_insert(int tid)
{
    count++;
    while (!running);
    while (running)
    {
        retry:
        Linked_List<int64_t, int64_t>* current_list = list.load();
        if(current_list -> count.load() > 100)
        {
            std::vector<int64_t> keys;
            std::vector<int64_t> vals;
            current_list -> collect(keys, vals);
            Linked_List<int64_t, int64_t>* new_list = new Linked_List<int64_t, int64_t>();
            list.compare_exchange_strong(current_list, new_list);
            goto retry;
        }
        else
        {
            if(current_list ->insert(std::rand()%1000, 10) == -1)
            {
                Linked_List<int64_t, int64_t>* new_list = new Linked_List<int64_t, int64_t>();
                list.compare_exchange_strong(current_list, new_list);
            }
        }
        thput[tid]++;
    }
}

void Bin_insert(int tid)
{
    count++;
    while (!running);
    while (running)
    {
        retry:
        Bin<int64_t, int64_t>* current_list = a.load();
        if(current_list ->insert(std::rand()%10000, 10))
        {
//
            goto retry;
        }
        thput[tid]++;
    }
}

void LL_remove(int tid)
{
    count++;
    while (!running);
    while (running)
    {
        retry:
        Linked_List<int64_t, int64_t>* current_list = list.load();
        if(current_list -> count > 100)
        {
            std::vector<int64_t> keys;
            std::vector<int64_t> vals;
            current_list -> collect(keys, vals);
            Linked_List<int64_t, int64_t>* new_list = new Linked_List<int64_t, int64_t>();
            list.compare_exchange_strong(current_list, new_list);
            goto retry;
        }
        else
        {
            if(current_list ->remove(std::rand()%1000) == -1)
            {
                Linked_List<int64_t, int64_t>* new_list = new Linked_List<int64_t, int64_t>();
                list.compare_exchange_strong(current_list, new_list);
            }
        }
        thput[tid]++;
    }
}



int main() {
//    keys = (std::atomic<int64_t>*)malloc(sizeof(std::atomic<int64_t>)*(14));
//    Bin<int64_t, int64_t> a;
    a.store(new Bin<int64_t, int64_t>());


//    for(int i = 0; i < 1000; i++)
//    {
//        if(!a.insert(std::rand()%10000, 10))
//        {
//            std::cout<<"Insertion Failed  "<<i<<"\n";
//            break;
//        }
//    }
//    list.store(new Linked_List<int64_t, int64_t>());
////    ptr = (uintptr_t)new Bin_Index<uintptr_t, uintptr_t>();
    count = 0;
    int t = 1;
    std::thread t1[t];
    for(int i = 0; i < t; i++)
    {
        thput.push_back(0);
        t1[i] = std::thread(Bin_insert, i);
    }

//    for(int i = 16; i < 32; i++)
//    {
//        thput.push_back(0);
//        t1[i] = std::thread(LL_remove, i);
//    }
    while(count < t);
    running = true;
    sleep(10);
    running = false;
    for(int i = 0; i < t; i++)
        t1[i].join();
    uint64_t total = 0;
    for(int i = 0; i < t; i++)
        total = total + thput[i];
    std::cout<<"Throughput   "<<total/10<<"\n";
    std::vector<int64_t> keys;
    std::vector<int64_t> vals;
    a.load(std::memory_order_seq_cst) ->collect(keys, vals);
    for(int i = 0; i< keys.size(); i++)
    {
        std::cout<<keys[i]<<"\n";
    }
//    dcss(0);
//    CAS(0);
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
