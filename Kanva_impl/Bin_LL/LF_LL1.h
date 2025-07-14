

#ifndef UNTITLED_LF_LL_H
#define UNTITLED_LF_LL_H

#include<atomic>
#include <limits>
#include<vector>
#include <unordered_set>
#include <iostream>
#include "util1.h"


namespace kanva_impl {
template<typename K, typename V>
class Linked_List {
public:
    ll_Node<K, V> *head;
    std::atomic<int> count;
    Linked_List() {
        ll_Node<K, V> *max = new ll_Node<K, V>(std::numeric_limits<K>::max(), 0);
        ll_Node<K, V> *min = new ll_Node<K, V>(std::numeric_limits<K>::min(), 0);
        min->next.store(max);
        head = min;
    }


    void mark();

    int insert(K Key, V value);

    bool insert(K Key, V value, ll_Node<K, V> *new_node);

    int insert(K Key, V value, int tid, int phase);

    bool search(K key);

    void collect(std::vector<K> &, std::vector<V> &);

    void range_query(int64_t low, int64_t high, int64_t curr_ts, std::vector<std::pair<K, V>> &res);

    ll_Node<K, V> *find(K key);

    ll_Node<K, V> *find(K key, ll_Node<K, V> **);
};


template<typename K, typename V>
void Linked_List<K,V>::collect(std::vector<K> &keys, std::vector<V> &vals) {
    ll_Node<K,V>* left_node = head;
    int i = 0;
    while(left_node -> next.load(std::memory_order_seq_cst)){
        if(!is_marked_ref((long) left_node -> next.load(std::memory_order_seq_cst)))
        {
            ll_Node<K,V>* curr_next = left_node -> next.load(std::memory_order_seq_cst);
            left_node -> next.compare_exchange_strong(curr_next, (ll_Node<K,V>*)set_mark((long) curr_next));
        }
//
        left_node = (ll_Node<K,V>*)get_unmarked_ref((long) left_node -> next.load(std::memory_order_seq_cst));
        keys.push_back(left_node-> key);
        vals.push_back(left_node->value);
    }
}


template<typename K, typename V>
bool Linked_List<K,V>::search(K key) {
    ll_Node<K,V>* curr = head;
    while(curr -> key < key)
        curr = (ll_Node<K,V>*) get_unmarked_ref((long) curr -> next.load(std::memory_order_seq_cst));
    return (curr -> key == key && !is_marked_ref((long) curr -> next.load(std::memory_order_seq_cst)));
}



template<typename K, typename V>
ll_Node<K,V>* Linked_List<K,V>::find(K key) {
    ll_Node<K, V> *left_node = head;
    while(true) {
        ll_Node<K, V> *right_node = left_node -> next.load(std::memory_order_seq_cst);
        if (is_marked_ref((long) right_node))
            return nullptr;
        if (right_node->key >= key)
            return right_node;
        (left_node) = right_node;
    }
}

template<typename K, typename V>
ll_Node<K,V>* Linked_List<K,V>::find(K key, ll_Node<K, V> **left_node) {
    (*left_node) = head;
    while(true) {
        ll_Node<K, V> *right_node = (*left_node) -> next.load(std::memory_order_seq_cst);
        if (is_marked_ref((long) right_node)) {
            return nullptr;
        }
        if (right_node -> key >= key)
            return right_node;
        (*left_node) = right_node;
    }
}

template<typename K, typename V>
int Linked_List<K,V>::insert(K key, V value) {
//    return -1;
    while(true)
    {
        ll_Node<K,V>* prev_node = nullptr;
        ll_Node<K,V>* right_node = find(key, &prev_node);
        if(right_node == nullptr)
            return -1;
        if(right_node -> key == key && value == -1){
            right_node -> value == -1;
        }
        else
        {
            if(value == -1)
                return 0;
            ll_Node<K,V>* new_node = new ll_Node<K,V>(key,value);
            new_node -> next.store(right_node);
            if(prev_node -> next.compare_exchange_strong(right_node, new_node)) {
                count++;
                return 1;
            }
        }
    }
}

}



#endif //UNTITLED_LF_LL_H
