/**
 * @TODO: ADD COMMENT HEADER
 */

#ifndef FPTREE_ADAPTER_H
#define FPTREE_ADAPTER_H

#include <iostream>
#include <csignal>
#include <bits/stdc++.h>
using namespace std;

#include "errors.h"
#include "record_manager.h"
#include "random_fnv1a.h"
#ifdef USE_TREE_STATS
#   include "tree_stats.h"
#endif
#include "fptree.h"
#include "threadinfo.h"

#define SIZE 64
#define DATA_STRUCTURE_T nvindex::FP_tree::Btree<K, V, SIZE>

template <typename K, typename V, class Reclaim = reclaimer_debra<K>, class Alloc = allocator_new<K>, class Pool = pool_none<K>>
class ds_adapter {
private:
    const V NO_VALUE;
    DATA_STRUCTURE_T * const ds;

public:
    ds_adapter(const int NUM_THREADS,
               const K& KEY_MIN,
               const K& KEY_MAX,
               const V& VALUE_RESERVED,
               RandomFNV1A * const unused2)
            : NO_VALUE(VALUE_RESERVED)
            , ds(new DATA_STRUCTURE_T())
    { }

    ~ds_adapter() {
        delete ds;
    }

    V getNoValue() {
        return 0;
    }

    void initThread(const int tid) {
        nvindex::register_threadinfo();
    }
    void deinitThread(const int tid) {
        nvindex::unregister_threadinfo();
    }

    void warmupEnd() {
    }

    V insert(const int tid, const K& key, const V& val) {
        setbench_error("insert-replace functionality not implemented for this data structure");
    }

    V insertIfAbsent(const int tid, const K& key, const V& val) {
        if (ds->insert(key, val)) {
            // cout << "Inserted " << key << endl;
            return (V)0;
        }
        return (V)1;
    }

    V erase(const int tid, const K& key) {
        if (ds->remove(key)) {
            // cout << "Deleted " << key << endl;
            return (V)1;
        }
        return (V)0;
    }

    V find(const int tid, const K& key) {
        double latency_breaks[3];
        return ds->get(key, latency_breaks);
    }

    bool contains(const int tid, const K& key) {
        return find(tid, key) != (V) 0;
    }
    int rangeQuery(const int tid, const K& lo, const K& hi, K * const resultKeys, V * const resultValues) {
        setbench_error("not implemented");
    }
    void printSummary() {
        // cout << "Contains" << endl;
        // for (int i = 1; i <= 5; ++i) {
        //     cout << i << " " << boolalpha << contains(0, i) << endl;
        // }
//        ds->printDebuggingDetails();
    }
    bool validateStructure() {
        return true;
    }

    void printObjectSizes() {
    }

// #ifdef USE_TREE_STATS
    class NodeHandler {
    public:
        typedef nvindex::FP_tree::Node<K, V, SIZE>* NodePtrType;
        typedef nvindex::FP_tree::LN<K, V, SIZE>* LeafNodePtrType;
        typedef nvindex::FP_tree::IN<K, V, SIZE>* InnerNodePtrType;
        K minKey;
        K maxKey;

        NodeHandler(const K& _minKey, const K& _maxKey) {
            minKey = _minKey;
            maxKey = _maxKey;
        }

        class ChildIterator {
        private:
            InnerNodePtrType node; // node being iterated over
            int ix = 0;
        public:
            ChildIterator(NodePtrType _node): node{static_cast<InnerNodePtrType>(_node)} {}

            bool hasNext() {
                return ix < node->children_number;
            }

            NodePtrType next() {
                return node->children_ptrs[ix++];
            }
        };

        bool isLeaf(NodePtrType node) {
            return node->isLeaf();
        }

        size_t getNumKeys(NodePtrType node) {
            return node->isLeaf() ? static_cast<LeafNodePtrType>(node)->number : 0;
        }

        size_t getSumOfKeys(NodePtrType node) {
            if (!node->isLeaf()) return 0;
            auto leaf = static_cast<LeafNodePtrType>(node);
            long long sum = 0;
            for (int i = 0; i < SIZE; ++i) {
                if (leaf->bitmap.test(i)) {
                    sum += leaf->kv[i].key;
                }
            }
            return sum;
        }

        ChildIterator getChildIterator(NodePtrType node) {
            return ChildIterator(node);
        }
    };
    TreeStats<NodeHandler> * createTreeStats(const K& _minKey, const K& _maxKey) {
        return new TreeStats<NodeHandler>(new NodeHandler(_minKey, _maxKey), ds->getRoot(), false);
    }
// #endif
};

#endif
