//
// Created by gaurav on 23/7/22.
//

#ifndef BIN_LL_NODE_H
#define BIN_LL_NODE_H
#include <mutex>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include<thread>
#include <iostream>
#include "util1.h"
#include "LF_LL.h"



template<typename K, typename V>
class Node{
public:
    bool is_leaf;
    std::atomic<int64_t> count;
//    virtual void mark() = 0;
};
#endif //BIN_LL_NODE_H
