
#ifndef BIN_LL_BIN_H
#define BIN_LL_BIN_H

#include "Leaf_Node.h"
#include "Root_Node.h"

namespace kanva_impl {
template<typename K, typename V>
class Bin {
public:
    std::atomic<Node<K,V>*> root;
    std::atomic <int64_t> size;
    std::vector <std::pair<K, V>> collect();
    bool insert(K, V);
    bool remove(K);
    void collect(std::vector<K> &keys, std::vector<V> &Vals);
    Internal_Node<K,V>* split_root(std::vector<K> &keys, std::vector<V> &Vals);
    Bin()
    {
        root = (Node<K,V>*) (new leaf_node<K,V>());
        size = 0;
    }

    Node<K, V> *insert_leaf(K key, V value, leaf_node<K, V> *current_child);

    Node<K, V> *insert_internal(K key, V value, Internal_Node<K, V> *current_root);

    Node<K, V> *remove_leaf(K key, leaf_node<K, V> *current_child);

    Node<K, V> *remove_internal(K key, Internal_Node<K, V> *current_root);

    V search(K key);
};

template<typename K, typename V>
V Bin<K,V>::search(K key) {
    Node<K,V>* curr_root = (Node<K,V>*)unset_mark((long) root.load(std::memory_order_seq_cst));
    if(curr_root == nullptr) return V{}; // Return default value if root is null
    
    if(curr_root -> is_leaf)
        return ((leaf_node<K,V>*) curr_root) -> data_array_list.search(key);
    Internal_Node<K,V>* node = (Internal_Node<K,V>*) curr_root;
    int ptr_idx = std::lower_bound(node->key.begin(), node->key.begin() + node->count, key) -
                  node->key.begin();
    return node->ptr[ptr_idx]->data_array_list.search(key);
}

template<typename K, typename V>
bool Bin<K,V>::remove(K key) {
    retry:
    Node<K,V>* curr_root = root.load(std::memory_order_seq_cst);
    if(is_marked((uintptr_t) curr_root))
        return false;
    if(curr_root -> is_leaf)
    {
        Node<K,V>* new_node = remove_leaf(key, (leaf_node<K,V>*) curr_root);
        if(new_node) {
            root.compare_exchange_strong(curr_root, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst);
            goto retry;
        }
        size++;
        return true;
    }
    else
    {
        Node<K,V>* new_node =  remove_internal(key, (Internal_Node<K,V>*)curr_root);
        if(new_node) {
            root.compare_exchange_strong(curr_root, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst);
            goto retry;
        }
        size++;
        return true;
    }
}

template<typename K, typename V>
Node<K,V>* Bin<K,V>::remove_leaf(K key, leaf_node<K,V>* current_child) {
    bool res = current_child -> data_array_list.remove(key);
    if(res == -1)
    {
        std::vector<K> keys;
        std::vector<V> vals;
        current_child -> data_array_list.collect(keys, vals);
        Node<K,V>* new_root = (Node<K,V>*)split_root(keys, vals);
        return new_root;
    }
    else {
        current_child -> count--;
        return nullptr;
    }
}

template<typename K, typename V>
void Bin<K,V>::collect(std::vector<K> &keys, std::vector<V> &Vals) {
    Node<K,V>* curr_root = (Node<K,V>*)unset_mark((long) root.load(std::memory_order_seq_cst));
    if(curr_root -> is_leaf) {
        // If root is a leaf node, collect directly from it
        ((leaf_node<K,V>*) curr_root) -> data_array_list.collect(keys, Vals);
    } else {
        // If root is an internal node, iterate through its children
        for(int i = 0; i <= curr_root -> count; i++)
        {
            leaf_node<K,V>* curr_child = (leaf_node<K,V>*) ((Internal_Node<K,V>*)curr_root) ->ptr[i];
            curr_child -> data_array_list.collect(keys, Vals);
        }
    }
}

template<typename K, typename V>
Node<K,V>* Bin<K,V>::remove_internal(K key, Internal_Node<K,V>* current_root) {
    int ptr_idx = std::lower_bound(current_root->key.begin(), current_root->key.begin() + current_root->count, key) -
                  current_root->key.begin();
    leaf_node<K, V> *current_child = (leaf_node<K, V> *) current_root->ptr[ptr_idx];
    int res = current_child->data_array_list.remove(key);
    if (res == -1) {
        if (current_root->count < root_max) {
            std::vector<K> keys;
            std::vector<V> vals;
            current_child->data_array_list.collect(keys, vals);
            Internal_Node<K, V> *new_root = split_root(keys, vals);
            Internal_Node<K, V> *new_root_node = new Internal_Node<K, V>();
            int i = 0;
            for (; i < ptr_idx; i++) {
                new_root_node->key.push_back(current_root->key[i]);
                new_root_node->ptr[i] = current_root->ptr[i];
            }
            new_root_node->key.push_back(new_root->key[0]);
            new_root_node->ptr[ptr_idx] = new_root->ptr[0];
            new_root_node->ptr[ptr_idx + 1] = new_root->ptr[1];
            for (int i = ptr_idx + 1; i <= current_root->count; i++) {
                new_root_node->key[i] = current_root->key[i - 1];
                new_root_node->ptr[i + 1] = current_root->ptr[i - 1];
            }
            new_root_node->count = current_root->count + 1;
            return (Node<K,V>*)new_root_node;
        } else
            return (Node<K, V> *) set_mark((long) current_root);
    }
}

template<typename K, typename V>
Internal_Node<K,V>* Bin<K,V>::split_root(std::vector<K> &keys, std::vector<V> &vals) {
    leaf_node<K, V> *left_child = new leaf_node<K, V>();
    leaf_node<K, V> *right_child = new leaf_node<K, V>();
    int curr_node_count = keys.size();
    left_child -> count.store(curr_node_count / 2, std::memory_order_seq_cst);
    int left_count = curr_node_count / 2;
    right_child -> count.store(curr_node_count - left_count);
    int right_count = curr_node_count - left_count;
    int64_t j = 0;
    ll_Node<K,V> *curr_list_node = left_child -> data_array_list.head;
    ll_Node<K,V> *last_list_node = left_child -> data_array_list.head -> next.load(std::memory_order_seq_cst);
    for(int i = left_count - 1; i >= 0; i--)
    {
//        left_child -> data_array_list.insert(keys[i], vals[i]);
        ll_Node<K,V> *new_node = new ll_Node<K,V>(keys[j], vals[j], last_list_node);
        curr_list_node -> next.store(new_node);
        curr_list_node = new_node;
        j++;
    }
    curr_list_node = right_child -> data_array_list.head;
    last_list_node = right_child -> data_array_list.head -> next.load(std::memory_order_seq_cst);
    for (int64_t i = right_count - 1; i >= 0; i--) {
//        right_child -> data_array_list.insert(keys[i], vals[i]);

        ll_Node<K,V> *new_node = new ll_Node<K,V>(keys[j], vals[j], last_list_node);
        curr_list_node -> next.store(new_node);
        curr_list_node = new_node;
        j++;
    }
    Internal_Node<K,V>* new_root = new Internal_Node<K,V>();
    new_root -> count = 1;
    new_root-> key[0] = (keys[left_count]);
    new_root -> ptr[0] = left_child;
    new_root ->ptr[1] = right_child;
    return new_root;
}

template<typename K, typename V>
Node<K,V>* Bin<K,V>::insert_leaf(K key, V value, leaf_node<K,V>* current_child) {
    if(current_child -> count < child_max)
    {
        bool res = current_child -> data_array_list.insert(key, value);
        if(res == -1)
        {
            std::vector<K> keys;
            std::vector<V> vals;
            current_child -> data_array_list.collect(keys, vals);
            Node<K,V>* new_root = (Node<K,V>*) split_root(keys, vals);
            return new_root;
        }
        else if(res == 1){
            current_child -> count++;
            return nullptr;
        }
        else
            return nullptr;
    }
    else
    {
        std::vector<K> keys;
        std::vector<V> vals;
        current_child -> data_array_list.collect(keys, vals);
        Node<K,V>* new_root = (Node<K,V>*)split_root(keys, vals);
        return new_root;
    }
}

template<typename K, typename V>
Node<K,V>* Bin<K,V>::insert_internal(K key, V value, Internal_Node<K,V>* current_root)
{
    int ptr_idx = std::lower_bound(current_root -> key.begin(), current_root -> key.begin() + current_root -> count, key) - current_root -> key.begin();
    leaf_node<K,V>* current_child = (leaf_node<K,V>*) current_root -> ptr[ptr_idx];
    if(current_child -> count < child_max)
    {
        int res = current_child -> data_array_list.insert(key, value);
        if(res == -1 )
        {
            if(current_root -> count < root_max) {
                std::vector<K> keys;
                std::vector<V> vals;
                current_child->data_array_list.collect(keys, vals);
                Internal_Node<K, V> *new_root = split_root(keys, vals);
                Internal_Node<K,V>* new_root_node = new Internal_Node<K,V>();
                int i = 0;
                for(; i < ptr_idx; i++)
                {
                    new_root_node -> key[i] = (current_root -> key[i]);
                    new_root_node -> ptr[i] = current_root -> ptr[i];
                }
                new_root_node -> key[ptr_idx] = (new_root -> key[0]);
                new_root_node -> ptr[ptr_idx] = new_root -> ptr[0];
                new_root_node -> ptr[ptr_idx + 1] = new_root -> ptr[1];
                for(int i = ptr_idx + 1; i <= current_root -> count; i++)
                {
                    new_root_node -> key[i] = current_root -> key[i - 1];
                    new_root_node -> ptr[i + 1] = current_root -> ptr[i];
                }
                new_root_node -> count = current_root -> count + 1;
//                int k = new_root -> key[0];
//                new_root -> key.pop_back();
//                Node<K,V>* ptr1 = new_root -> ptr[0];
//                Node<K,V>* ptr2 = new_root -> ptr[1];
//                int i = 0;
//                for(; i < ptr_idx; i++)
//                {
//                    new_root -> key.push_back(current_root -> key[i]);
//                    new_root -> ptr[i] = current_root -> ptr[i];
//                }
//                new_root -> key.push_back(k);
//                new_root -> ptr[ptr_idx] = ptr1;
//                new_root -> ptr[ptr_idx + 1] = ptr2;
//                for(int i = ptr_idx + 1; i <= current_root -> count; i++)
//                {
//                    new_root -> key.push_back(current_root -> key[i - 1]);
//                    new_root -> ptr[i + 1] = current_root -> ptr[i - 1];
//                }
//                new_root -> count = current_root -> count + 1;
                return (Node<K,V>*)new_root_node;
            }
            else
                return (Node<K,V>*) set_mark((long)current_root);
        } else if( res == 1) {
            current_child -> count++;
            return nullptr;
        }
        else
            return nullptr;
    }
    else if(current_root -> count < root_max)
    {
        std::vector<K> keys;
        std::vector<V> vals;
        current_child->data_array_list.collect(keys, vals);
        Internal_Node<K, V> *new_root = split_root(keys, vals);
        Internal_Node<K,V>* new_root_node = new Internal_Node<K,V>();
        int i = 0;
        for(; i < ptr_idx; i++)
        {
            new_root_node -> key[i] = (current_root -> key[i]);
            new_root_node -> ptr[i] = current_root -> ptr[i];
        }
        new_root_node -> key[ptr_idx] = (new_root -> key[0]);
        new_root_node -> ptr[ptr_idx] = new_root -> ptr[0];
        new_root_node -> ptr[ptr_idx + 1] = new_root -> ptr[1];
        for(int i = ptr_idx + 1; i <= current_root -> count; i++)
        {
            new_root_node -> key[i] = current_root -> key[i - 1];
            new_root_node -> ptr[i + 1] = current_root -> ptr[i];
        }
        new_root_node -> count = current_root -> count + 1;
//        Internal_Node<K,V>* new_root_node = new Internal_Node<K,V>();
//        int k = new_root -> key[0];
//        new_root -> key.pop_back();
//        Node<K,V>* ptr1 = new_root -> ptr[0];
//        Node<K,V>* ptr2 = new_root -> ptr[1];
//        int i = 0;
//        for(; i < ptr_idx; i++)
//        {
//            new_root -> key.push_back(current_root -> key[i]);
//            new_root -> ptr[i] = current_root -> ptr[i];
//        }
//        new_root -> key.push_back(k);
//        new_root -> ptr[ptr_idx] = ptr1;
//        new_root -> ptr[ptr_idx + 1] = ptr2;
//        for(int i = ptr_idx + 1; i <= current_root -> count; i++)
//        {
//            new_root -> key.push_back(current_root -> key[i - 1]);
//            new_root -> ptr[i + 1] = current_root -> ptr[i - 1];
//        }
//        new_root -> count = current_root -> count + 1;
        return (Node<K,V>*)new_root_node;
    }
    else
        return (Node<K,V>*) set_mark((long)current_root);
}


template<typename K, typename V>
bool Bin<K,V>::insert(K key, V value) {
    retry:
    Node<K,V>* curr_root = root.load(std::memory_order_seq_cst);
    if(is_marked((uintptr_t) curr_root))
        return false;
    if(curr_root -> is_leaf)
    {
        Node<K,V>* new_node = insert_leaf(key, value, (leaf_node<K,V>*) curr_root);
        if(new_node) {
            root.compare_exchange_strong(curr_root, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst);
            goto retry;
        }
        size++;
        return true;
    }
    else
    {
        Node<K,V>* new_node =  insert_internal(key, value, (Internal_Node<K,V>*)curr_root);
        if(new_node) {
            root.compare_exchange_strong(curr_root, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst);
            goto retry;
        }
        size++;
        return true;
    }
}
}
#endif //BIN_LL_BIN_H
