

#ifndef UNTITLED_LF_LL_H
#define UNTITLED_LF_LL_H

#include <atomic>
#include <limits>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <thread>
#include "util.h"
#include "common/recordmgr/record_manager.h"

// without sentinel
#define threshhold 10

template <typename KeyT, typename ValT>
class Linked_List
{
public:
    typedef record_manager<
        reclaimer_debra<KeyT>,
        allocator_new<KeyT>,
        pool_none<KeyT>,
        ll_Node<KeyT, ValT>>
        ll_record_manager_t;
    typedef record_manager<
        reclaimer_debra<KeyT>,
        allocator_new<KeyT>,
        pool_none<KeyT>,
        Vnode<ValT>>
        vnode_record_manager_t;

    ll_record_manager_t *llRecMgr = nullptr;
    vnode_record_manager_t *vnodeRecMgr = nullptr;

    bool isReclaimed=false;

    ll_Node<KeyT, ValT> *head;
    std::atomic<size_t> size = 0;
    /*Linked_List()
    {
        ll_Node<KeyT, ValT> *max = new ll_Node<KeyT, ValT>(std::numeric_limits<KeyT>::max(), 0);
        ll_Node<KeyT, ValT> *min = new ll_Node<KeyT, ValT>(std::numeric_limits<KeyT>::min(), 0);
        min->next.store(max);
        head = min;
    }*/
    Linked_List(ll_record_manager_t *__llRecMgr, vnode_record_manager_t *__vnodeRecMgr, thread_id_t tid)
    {
        llRecMgr = __llRecMgr;
        vnodeRecMgr = __vnodeRecMgr;
        ll_Node<KeyT, ValT> *max = llRecMgr->template allocate<ll_Node<KeyT, ValT>>(tid);
        ll_Node<KeyT, ValT> *min = llRecMgr->template allocate<ll_Node<KeyT, ValT>>(tid);
        max->init(std::numeric_limits<KeyT>::max(), 0);
        min->init(std::numeric_limits<KeyT>::min(), 0);
        min->next.store(max);
        head = min;
    }
    int64_t count();

    int insert(KeyT Key, ValT value, TrackerList *version_tracker, thread_id_t tid);
    int remove(KeyT key, TrackerList *version_tracker, thread_id_t tid);

    // bool search(KeyT key, ValT value);
    bool search(KeyT key, ValT value, thread_id_t tid);

    bool collect(std::vector<KeyT> *,std::vector<Vnode<ValT> *>*,  thread_id_t tid);

    int range_query(int64_t low, int64_t remaining, int64_t curr_ts, std::vector<std::pair<KeyT, ValT>> &res, TrackerList *version_tracker, thread_id_t tid);

    // ll_Node<KeyT, ValT> *find(KeyT key, ll_Node<KeyT, ValT> **);
    ll_Node<KeyT, ValT> *find(KeyT key, ll_Node<KeyT, ValT> **, thread_id_t tid);

    void reclaimMem(int64_t tstamp_threshold, thread_id_t tid);

    void init_ts(Vnode<ValT> *node, TrackerList *version_tracker, thread_id_t tid)
    {
        auto guard = llRecMgr->getGuard(tid);
        if (node->ts.load(std::memory_order_seq_cst) == -1)
        {
            int64_t invalid_ts = -1;
            int64_t global_ts = (*version_tracker).get_latest_timestamp();
            node->ts.compare_exchange_strong(invalid_ts, global_ts, std::memory_order_seq_cst, std::memory_order_seq_cst);
        }
    }
    ValT read(ll_Node<KeyT, ValT> *node, TrackerList *version_tracker, thread_id_t tid)
    {
        auto guard = llRecMgr->getGuard(tid);
        init_ts(node->vhead, version_tracker, tid);
        return node->vhead.load(std::memory_order_seq_cst)->value;
    }

    bool vCAS(ll_Node<KeyT, ValT> *node, ValT old_value, ValT new_value, TrackerList *version_tracker, thread_id_t tid)
    {
        auto guard = llRecMgr->getGuard(tid);
        Vnode<ValT> *head = (Vnode<ValT> *)unset_mark((uintptr_t)node->vhead.load(std::memory_order_seq_cst));
        init_ts(head, version_tracker, tid);
        if (head->value != old_value)
            return false;
        if (head->value == new_value)
            return true;
        // Vnode<ValT> *new_node = new Vnode<ValT>(new_value, head);
        Vnode<ValT> *new_node = vnodeRecMgr->template allocate<Vnode<ValT>>(tid);
        new_node->init(new_value, head);
        if (expt_sleep)
        {
            int dice_roll = dice(gen);
            if (dice_roll < 5)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(5));
            }
        }
        if (node->vhead.compare_exchange_strong(head, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            init_ts(new_node, version_tracker, tid);
            return true;
        }
        else
        {
            //            delete new_node;
            return false;
        }
        return false;
    }

    bool vCAS(ll_Node<KeyT, ValT> *node, ValT old_value, ValT new_value, Vnode<ValT> *new_node, Vnode<ValT> *head, Vnode<ValT> *vnext, TrackerList *version_tracker, thread_id_t tid)
    {
        auto guard = llRecMgr->getGuard(tid);
        if (head->value != old_value)
            return false;
        if (head->value == new_value)
            return true;
        if (!new_node->nextv.compare_exchange_strong(vnext, head, std::memory_order_seq_cst, std::memory_order_seq_cst))
            return false;
        if (expt_sleep)
        {
            int dice_roll = dice(gen);
            if (dice_roll < 5)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(5));
            }
        }
        if (node->vhead.compare_exchange_strong(head, new_node, std::memory_order_seq_cst, std::memory_order_seq_cst))
        {
            init_ts(new_node, version_tracker);
            return true;
        }
        else
        {
            //            delete new_node;
            return false;
        }
    }
};

template <typename KeyT, typename ValT>
bool Linked_List<KeyT, ValT>::collect(std::vector<KeyT> *keys,std::vector<Vnode<ValT> *> *version_lists, thread_id_t tid)
{   
    if(isReclaimed) return false;
    auto guard = llRecMgr->getGuard(tid);
    
    ll_Node<KeyT, ValT> *left_node = head;
    if (!is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)))
    {
        while (true)
        {
            ll_Node<KeyT, ValT> *curr_next = left_node->next.load(std::memory_order_seq_cst);
            if (!left_node->next.compare_exchange_strong(curr_next, (ll_Node<KeyT, ValT> *)set_freeze((long)curr_next)))
                continue;
            break;
        }
    }

    if (!is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)))
    {
        while (true)
        {
            Vnode<ValT> *curr_vhead = left_node->vhead.load(std::memory_order_seq_cst);
            if (!left_node->vhead.compare_exchange_strong(curr_vhead, (Vnode<ValT> *)set_freeze((long)curr_vhead)))
                continue;
            break;
        }
    }
    left_node = (ll_Node<KeyT, ValT> *)unset_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst));
    while (unset_mark((long)left_node->next.load(std::memory_order_seq_cst)))
    {
        if (!is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)))
        {
            while (true)
            {
                ll_Node<KeyT, ValT> *curr_next = left_node->next.load(std::memory_order_seq_cst);
                if (!left_node->next.compare_exchange_strong(curr_next, (ll_Node<KeyT, ValT> *)set_freeze((long)curr_next)))
                    continue;
                break;
            }
        }

        if (!is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)))
        {
            while (true)
            {
                Vnode<ValT> *curr_vhead = left_node->vhead.load(std::memory_order_seq_cst);
                if (!left_node->vhead.compare_exchange_strong(curr_vhead, (Vnode<ValT> *)set_freeze((long)curr_vhead)))
                    continue;
                break;
            }
        }
        Vnode<ValT> *left_node_vhead = (Vnode<ValT> *)get_unmarked_ref((uintptr_t)left_node->vhead.load(std::memory_order_seq_cst));

        if(!isReclaimed){
            (*keys).push_back(left_node->key);
            //(*values).push_back(left_node_vhead->value);
            (*version_lists).push_back(left_node_vhead);
        }
        else return false;

        left_node = (ll_Node<KeyT, ValT> *)unset_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst));
        //        Vnode<ValT> *left_node_vhead = (Vnode<ValT>*) get_unmarked_ref((uintptr_t)left_node -> vhead.load(std::memory_order_seq_cst));
    }
    return true;
}

template <typename KeyT, typename ValT>
int Linked_List<KeyT, ValT>::range_query(int64_t low, int64_t remaining, int64_t curr_ts, std::vector<std::pair<KeyT, ValT>> &res, TrackerList *version_tracker, thread_id_t tid)
{

    ll_Node<KeyT, ValT> *left_node = (ll_Node<KeyT, ValT> *)get_unfreezed_unmarked_ref((long)head->next.load(std::memory_order_seq_cst));
    ll_Node<KeyT, ValT> *left_next = (ll_Node<KeyT, ValT> *)get_unfreezed_unmarked_ref((long)left_node->next.load(std::memory_order_seq_cst));
    while (left_next && left_node->key < low)
    {
        // std::cout<<"Range query in level bin"<<std::endl;
        left_node = left_next;
        left_next = (ll_Node<KeyT, ValT> *)get_unfreezed_unmarked_ref((long)left_next->next.load(std::memory_order_seq_cst));
    }
    while (left_next && remaining > 0)
    {
        Vnode<ValT> *curr_vhead = (Vnode<ValT> *)get_unfreezed_unmarked_ref((uintptr_t)left_node->vhead.load(std::memory_order_seq_cst));
        init_ts(curr_vhead, version_tracker, tid);
        while (curr_vhead && curr_vhead->ts >= curr_ts)
            curr_vhead = curr_vhead->nextv;
        if (curr_vhead && curr_vhead->value != -1)
        {
            // std::cout<<"Range query in level bin"<<std::endl;
            res.push_back(std::make_pair(left_node->key, curr_vhead->value));
            remaining -= 1;
            if (remaining <= 0)
                break;
        }
        left_node = left_next;
        left_next = (ll_Node<KeyT, ValT> *)get_unfreezed_unmarked_ref((long)left_next->next.load(std::memory_order_seq_cst));
    }
    return remaining;
}

template <typename KeyT, typename ValT>
bool Linked_List<KeyT, ValT>::search(KeyT key, ValT value, thread_id_t tid)
{ // CHANGED CHECK:inserted value fieled
    auto guard = llRecMgr->getGuard(tid);
    ll_Node<KeyT, ValT> *curr = head;
    // std::cout<<"LL search"<<std::endl;
    while (curr->key < key)
        curr = (ll_Node<KeyT, ValT> *)unset_freeze_mark((long)curr->next.load(std::memory_order_seq_cst));

    return (curr->key == key && curr->vhead.load(std::memory_order_seq_cst)->value == value && !is_marked_ref((long)curr->next.load(std::memory_order_seq_cst)));
}

template <typename KeyT, typename ValT>
ll_Node<KeyT, ValT> *Linked_List<KeyT, ValT>::find(KeyT key, ll_Node<KeyT, ValT> **left_node, thread_id_t tid)
{
    auto guard = llRecMgr->getGuard(tid);

    while (true)
    {
        (*left_node) = head;
        ll_Node<KeyT, ValT> *right_node;
        if (is_freeze((uintptr_t)(*left_node)->next.load(std::memory_order_seq_cst)))
            return nullptr;
        right_node = (ll_Node<KeyT, ValT> *)unset_freeze_mark((long)(*left_node)->next.load(std::memory_order_seq_cst));
        while (true)
        {
            ll_Node<KeyT, ValT> *right_next = right_node->next.load(std::memory_order_seq_cst);
            if (is_freeze((uintptr_t)right_next))
            {
                return nullptr;
            }
            if (right_node->key >= key)
                return right_node;
            (*left_node) = right_node;
            right_node = (ll_Node<KeyT, ValT> *)get_unmarked_ref((long)right_next);
        }
    }
}

template <typename KeyT, typename ValT>
int Linked_List<KeyT, ValT>::insert(KeyT key, ValT value, TrackerList *version_tracker, thread_id_t tid)
{
    auto guard = llRecMgr->getGuard(tid);
    //    return -1 for failed operation,0 for no incremenet,1 for sucess,-2 for retraining,

    if (size.load(std::memory_order_seq_cst) >= threshhold)
        return -2; // retraining

    while (true)
    { // std::cout<<"Insert at Linked List"<<std::endl;
        ll_Node<KeyT, ValT> *prev_node = nullptr;
        ll_Node<KeyT, ValT> *right_node = find(key, &prev_node, tid);
        if (right_node == nullptr)
            return -1;
        if (right_node->key == key)
        {
            while (true)
            {
                ValT curr_value = read(right_node, version_tracker, tid);
                if (curr_value == value)
                    return 0;
                if (vCAS(right_node, curr_value, value, version_tracker, tid))
                    break;
                else if (is_marked_ref((uintptr_t)right_node->vhead.load(std::memory_order_seq_cst)))
                    return -1; // INCORRECT
            }
            return 0; // version list has changed
        }

        if (value == -1)
            return 0;

        // ll_Node<KeyT, ValT> *new_node = new ll_Node<KeyT, ValT>(key, value);
        ll_Node<KeyT, ValT> *new_node = llRecMgr->template allocate<ll_Node<KeyT, ValT>>(tid);
        new_node->init(key, value);
        new_node->next.store(right_node);

        if (prev_node->next.compare_exchange_strong(right_node, new_node))
        {
            init_ts(new_node->vhead, version_tracker, tid);
            size += 1; // increment size
            // std::cout<<"Insert in LF_LL.h"<<size.load(std::memory_order_seq_cst)<<std::endl;
            return 1;
        }
    }
}

template <typename KeyT, typename ValT>
int Linked_List<KeyT, ValT>::remove(KeyT key, TrackerList *version_tracker, thread_id_t tid)
{
    auto guard = llRecMgr->getGuard(tid);
    return insert(key, -1, version_tracker, tid); // CHANGE
}

template <typename KeyT, typename ValT>
void Linked_List<KeyT, ValT>::reclaimMem(int64_t tstamp_threshold, thread_id_t tid)
{
    auto guard = llRecMgr->getGuard(tid);

    ll_Node<KeyT, ValT> *left_node = head, *prev_node = nullptr;
    assert (is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)));
   
    prev_node = left_node;
    left_node = (ll_Node<KeyT, ValT> *)unset_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst));
    while (unset_mark((long)left_node->next.load(std::memory_order_seq_cst)))
    {
        assert (is_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst)));
        
        Vnode<ValT> *left_node_vhead = (Vnode<ValT> *)get_unmarked_ref((uintptr_t)left_node->vhead.load(std::memory_order_seq_cst));

        Vnode<ValT> *left_vnode = left_node_vhead, *curr_vnode = nullptr;
        while (unset_freeze((long)left_vnode->nextv.load(std::memory_order_seq_cst)))
        {
            curr_vnode = (Vnode<ValT> *)unset_freeze((long)left_vnode->nextv.load(std::memory_order_seq_cst));
            // std::cout << curr_vnode->ts << " " << tstamp_threshold << std::endl;
            if (curr_vnode->ts <= tstamp_threshold)
            {
                left_vnode->nextv = nullptr;
                vnodeRecMgr->template retire(tid, curr_vnode);
                // std::cout << "Retiring\n";
            }
            left_vnode = curr_vnode;
        }
        left_node->vhead = nullptr;
        if(prev_node)
            prev_node->next = nullptr;
        llRecMgr->template retire(tid, left_node);
        prev_node = left_node;
        left_node = (ll_Node<KeyT, ValT> *)unset_freeze((uintptr_t)left_node->next.load(std::memory_order_seq_cst));
        //        Vnode<ValT> *left_node_vhead = (Vnode<ValT>*) get_unmarked_ref((uintptr_t)left_node -> vhead.load(std::memory_order_seq_cst));
    }
    llRecMgr->template retire(tid, head);
    head = nullptr;
}

#endif // UNTITLED_LF_LL_H
