//
// Created by gaurav on 15/7/22.
//

#ifndef BIN_LEVEL_INDEX_H
#define BIN_LEVEL_INDEX_H
#include "BIn.h"

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

template <typename key_type, typename val_type>
class Level_Index
{
public:
    std::atomic<RootBin<key_type, val_type>*> bin;
    Level_Index(){
        bin.store(new RootBin<key_type, val_type>());
    }
    bool insert(key_type key, val_type val);
    val_type search(key_type key)
    {
        RootBin<key_type, val_type> *current_bin = bin.load(std::memory_order_seq_cst);
        return unset_mark((uintptr_t)current_bin->search(key));
    }
    std::vector<std::pair<key_type, val_type>>* collect()
    {
        RootBin<key_type, val_type> *current_bin = bin.load(std::memory_order_seq_cst);
        return ((RootBin<key_type, val_type>*) unset_mark((uintptr_t)current_bin))->collect();
    }
};

template <typename key_type, typename val_type>
bool Level_Index<key_type, val_type>::insert(key_type key, val_type val) {
    while(true) {
        RootBin<key_type, val_type> *current_bin = bin.load(std::memory_order_seq_cst);
        if(is_marked((uintptr_t)current_bin))
            return false;
        RootBin<key_type, val_type> *new_bin = (RootBin<key_type, val_type>*) unset_mark((uintptr_t)current_bin->insert(key, val));
        if(new_bin == nullptr){
            current_bin = bin.load(std::memory_order_seq_cst);
            if(current_bin -> keycount >= Degree) {
                bin.compare_exchange_strong(current_bin,
                                            (RootBin<key_type, val_type> *) set_mark((uintptr_t) current_bin));
                return false;
            }
            else return true;
        }
        if(bin.compare_exchange_strong(current_bin, new_bin))
            return true;
    }
}
#endif //BIN_LEVEL_INDEX_H
