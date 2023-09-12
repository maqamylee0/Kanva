#ifndef __KANVA_RS_MODEL_H__
#define __KANVA_RS_MODEL_H__

/*#include "plex_model.h"
#include "plex_model_impl.h"*/
#include "lr_model.h"
#include "lr_model_impl.h"
#include "util.h"
#include "Uruv/LF_LL.h"
#include "Uruv/VersionTracker/TrackerList.h"
#include <atomic>
#include <stdlib.h>


namespace kanva_RS{



template<class key_t, class val_t>
class Kanva_RSModel {
public:
    //typedef PlexModel<key_t> plexmodel_type;
    typedef LinearRegressionModel<key_t> lrmodel_type;
    typedef kanva_RS::Kanva_RSModel<key_t, val_t> kanva_RSmodel_type;

    typedef struct model_or_bin {
        typedef union pointer{
            Linked_List<key_t,val_t> *lflb;//versioned linked list
            kanva_RSmodel_type* ai;
        }pointer_t;
        pointer_t mob;
        bool volatile isbin = true;   // true = lb, false = ai
    }model_or_bin_t;

    typedef record_manager<
        reclaimer_debra<key_t>,
        allocator_new<key_t>,
        pool_none<key_t>,
        ll_Node<key_t, val_t>>
        ll_record_manager_t;
    typedef record_manager<
        reclaimer_debra<key_t>,
        allocator_new<key_t>,
        pool_none<key_t>,
        Vnode<key_t>>
        vnode_record_manager_t; 
        
    ll_record_manager_t *llRecMgr;
    vnode_record_manager_t *vnodeRecMgr;

public:
    inline Kanva_RSModel(ll_record_manager_t *__llRecMgr, vnode_record_manager_t *__vnodeRecMgr);
    ~Kanva_RSModel();
    Kanva_RSModel(lrmodel_type &lrmodel, const typename std::vector<key_t>::const_iterator &keys_begin,
               const typename std::vector<key_t>::const_iterator &vals_begin,
               size_t size, size_t _maxErr, ll_record_manager_t *__llRecMgr, vnode_record_manager_t *__vnodeRecMgr);
    Kanva_RSModel(lrmodel_type &lrmodel, const typename std::vector<key_t>::const_iterator &keys_begin,
               
               size_t size, size_t _maxErr,std::vector<Vnode<val_t>*> version_lists, ll_record_manager_t *__llRecMgr, vnode_record_manager_t *__vnodeRecMgr);
    inline size_t get_capacity(thread_id_t tid);
    inline void print_model(thread_id_t tid);
    void print_keys(thread_id_t tid);
    void print_model_retrain(thread_id_t tid);
    void self_check(thread_id_t tid);
    //void self_check_retrain();
    
    //bool find(const key_t &key, val_t &val);
   // bool con_find(const key_t &key, val_t &val);
    //result_t con_find_retrain(const key_t &key, val_t &val);
    result_t update(const key_t &key, const val_t &val, thread_id_t tid); //NOT IMPLEMENTED
    //inline bool con_insert(const key_t &key, const val_t &val);
    //result_t con_insert_retrain(const key_t &key, const val_t &val);
    bool remove(const key_t &key,TrackerList *version_tracker, thread_id_t tid);
    bool find_retrain(const key_t &key, val_t &val, thread_id_t tid);
    int scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result,TrackerList *version_tracker,int ts, thread_id_t tid);
    bool insert_retrain(const key_t &key, const val_t &val,TrackerList *version_tracker, thread_id_t tid);//-1 for fail
    
    //void resort(std::vector<key_t> &keys, std::vector<val_t> &vals);

private:
    inline size_t predict(const key_t &key, thread_id_t tid);
    inline size_t locate_in_levelbin(const key_t &key, const size_t pos, thread_id_t tid);//binary search
    //inline size_t find_lower(const key_t &key, const size_t pos);
    //inline size_t linear_search(const key_t *arr, int n, key_t key);
    //inline size_t find_lower_avx(const int *arr, int n, int key);
    //inline size_t find_lower_avx(const int64_t *arr, int n, int64_t key);
    bool insert_model_or_bin(const key_t &key, const val_t &val, size_t bin_pos,TrackerList *version_tracker, thread_id_t tid);
    bool remove_model_or_bin(const key_t &key, const int bin_pos,TrackerList *version_tracker, thread_id_t tid);


private:
    lrmodel_type* model = nullptr;
    size_t maxErr = 64;
    size_t err = 0;
    VersionedArray<key_t,val_t> **model_array=nullptr;
    bool* valid_flag = nullptr;
    std::atomic<model_or_bin_t *> *mobs_lf = nullptr;
    const size_t capacity;

};

} //namespace kanva_RS



#endif
