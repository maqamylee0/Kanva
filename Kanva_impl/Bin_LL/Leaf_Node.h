
#ifndef BIN_LL_LEAF_NODE_H
#define BIN_LL_LEAF_NODE_H
#include "Node.h"
//#include "Root_Node.h"

template<typename K, typename V>
class leaf_node: public Node<int64_t, int64_t>{
public:
    Linked_List<K,V> data_array_list;
    int64_t insert_leaf(K,V);
    void split_leaf();
    int64_t delete_leaf(K);
    leaf_node<K,V> ()
    {
        is_leaf = true;
    }
};

//template<typename K, typename V>
//Node<K,V>* leaf_node<K,V>::insert(K key, V value) {
//    if(count < child_max)
//    {
//        bool res = data_array_list -> insert(key, value);
//        if(res == -1)
//        {
//            std::vector<K> keys;
//            std::vector<V> vals;
//            data_array_list.collect(keys, vals);
//            Node<K,V>* new_root = split_root(keys, vals);
//            return new_root;
//        }
//        else
//            return nullptr;
//    }
//    else
//    {
//        std::vector<K> keys;
//        std::vector<V> vals;
//        data_array_list.collect(keys, vals);
//        Node<K,V>* new_root = split_root(keys, vals);
//        return new_root;
//    }
//}

//template<typename K, typename V>
//Node<K,V>* leaf_node<K,V>::split_root(std::vector<K> &keys, std::vector<V> &vals) {
//    leaf_node<K, V> *left_child = new leaf_node<K, V>();
//    leaf_node<K, V> *right_child = new leaf_node<K, V>();
//    int curr_node_count = keys.size();
//    left_child -> count.store(curr_node_count / 2, std::memory_order_seq_cst);
//    int left_count = curr_node_count / 2;
//    right_child -> count.store(curr_node_count - left_count);
//    int right_count = curr_node_count - left_count;
//    int64_t j = 0;
//    ll_Node<K,V> *curr_list_node = left_child -> data_array_list.head;
//    ll_Node<K,V> *last_list_node = left_child -> data_array_list.head -> next.load(std::memory_order_seq_cst);
//    for(int i = 0; i < left_count; i++)
//    {
//        ll_Node<K,V> *new_node = new ll_Node<K,V>(keys[j], vals[j], last_list_node);
//        curr_list_node -> next.store(new_node);
//        curr_list_node = new_node;
//        j++;
//    }
//    curr_list_node = right_child -> data_array_list.head;
//    last_list_node = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst);
//    for (int64_t i = 0; i < right_count; i++) {
//        ll_Node<K,V> *new_node = new ll_Node<K,V>(keys[j], vals[j], last_list_node);
//        curr_list_node -> next.store(new_node);
//        curr_list_node = new_node;
//        j++;
//    }
//    Internal_Node<K,V>* new_root = new Node<K,V>();
//    new_root -> count = 1;
//    new_root-> key.push_back(keys[left_count]);
//    new_root -> ptr[0] = left_child;
//    new_root -> ptr[1] = right_child;
//    return new_root;
//}

#endif //BIN_LL_LEAF_NODE_H
