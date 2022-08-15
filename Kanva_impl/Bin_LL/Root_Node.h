
#ifndef BIN_LL_ROOT_NODE_H
#define BIN_LL_ROOT_NODE_H
#include "Node.h"
//#include "Leaf_Node.h"
#include "util1.h"

template<typename K, typename V>
class Internal_Node : public Node<int64_t, int64_t>
{
public:
    std::vector<K> key;
    leaf_node<K,V>* ptr[root_max + 1];
    void mark();
    Internal_Node()
    {
        is_leaf = false;
        for(int64_t i = 0; i < root_max; i++) {
            key.push_back(0);
            ptr[i] = nullptr;
        }
        ptr[root_max] = nullptr;
    }
    Node<K,V>* insert(K key, V value);
};



#endif //BIN_LL_ROOT_NODE_H
