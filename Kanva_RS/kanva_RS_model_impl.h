#ifndef __KANVA_RS_MODEL_IMPL_H__
#define __KANVA_RS_MODEL_IMPL_H__

#include "kanva_RS_model.h"
#include "../common/util.h"


namespace kanva_RS{



template<class key_t, class val_t>
Kanva_RSModel<key_t, val_t>::Kanva_RSModel(ll_record_manager_t *__llRecMgr, vnode_record_manager_t *__vnodeRecMgr): llRecMgr(__llRecMgr), vnodeRecMgr(__vnodeRecMgr) {
    model = nullptr;
    maxErr = 64;
    err = 0;
    
    //levelbins = nullptr;
    capacity = 0;
}

template<class key_t, class val_t>
Kanva_RSModel<key_t, val_t>::~Kanva_RSModel()
{
    model = nullptr;
        for (int i = 0; i < capacity; i++)
        {
            mobs_lf[i] = nullptr;
        }
        mobs_lf = nullptr;
}

template<class key_t, class val_t>
Kanva_RSModel<key_t, val_t>::Kanva_RSModel(lrmodel_type &lrmodel, 
                                     const typename std::vector<key_t>::const_iterator &keys_begin, 
                                     
                                     size_t size, size_t _maxErr,std::vector<Vnode<val_t>*> version_lists,
                                     ll_record_manager_t *__llRecMgr, vnode_record_manager_t *__vnodeRecMgr) : maxErr(_maxErr), capacity(size), llRecMgr(__llRecMgr), vnodeRecMgr(__vnodeRecMgr)
{
    model=new lrmodel_type(lrmodel.get_weight0(), lrmodel.get_weight1());
    /*keys = (key_t *)malloc(sizeof(key_t)*size);
    vals = (val_t *)malloc(sizeof(val_t)*size);
    */
    
    model_array=(VersionedArray<key_t,val_t> **)malloc(sizeof(VersionedArray<key_t,val_t> *)*size);
    valid_flag = (bool*)malloc(sizeof(bool)*size);
    for(int i=0; i<size; i++){
        /*keys[i] = *(keys_begin+i);
        vals[i] = *(vals_begin+i);
        */
        //VersionedArray tempObject();//do memory reclamation here
        model_array[i]=new VersionedArray<key_t,val_t>(*(keys_begin+i),version_lists[i]);//CHECK:should use new?
        valid_flag[i] = true;
    }
    mobs_lf = (std::atomic<model_or_bin_t *> *)malloc(sizeof(std::atomic<model_or_bin_t *>) * (size + 1));
    for (int i = 0; i < size + 1; i++)
            mobs_lf[i] = nullptr;
    
}



template<class key_t, class val_t>
Kanva_RSModel<key_t, val_t>::Kanva_RSModel(lrmodel_type &lrmodel, 
                                     const typename std::vector<key_t>::const_iterator &keys_begin, 
                                     const typename std::vector<key_t>::const_iterator &vals_begin,
                                     size_t size, size_t _maxErr,
                                     ll_record_manager_t *__llRecMgr, vnode_record_manager_t *__vnodeRecMgr) : maxErr(_maxErr), capacity(size), llRecMgr(__llRecMgr), vnodeRecMgr(__vnodeRecMgr)
{
    model=new lrmodel_type(lrmodel.get_weight0(), lrmodel.get_weight1());
    /*keys = (key_t *)malloc(sizeof(key_t)*size);
    vals = (val_t *)malloc(sizeof(val_t)*size);
    */
    model_array=(VersionedArray<key_t,val_t> **)malloc(sizeof(VersionedArray<key_t,val_t> *)*size);
    valid_flag = (bool*)malloc(sizeof(bool)*size);
    for(int i=0; i<size; i++){
        /*keys[i] = *(keys_begin+i);
        vals[i] = *(vals_begin+i);
        */
        //VersionedArray tempObject();//CHECK:do memory reclamaiion 
        model_array[i]=new VersionedArray<key_t,val_t>(*(keys_begin+i),*(vals_begin+i));//CHECK:should use new?
        valid_flag[i] = true;
    }
    mobs_lf = (std::atomic<model_or_bin_t *> *)malloc(sizeof(std::atomic<model_or_bin_t *>) * (size + 1));
    for (int i = 0; i < size + 1; i++)
            mobs_lf[i] = nullptr;
    
}

template<class key_t, class val_t>
inline size_t Kanva_RSModel<key_t, val_t>::get_capacity(thread_id_t tid)
{
    return capacity;
}

// =====================  print =====================
template<class key_t, class val_t>
inline void Kanva_RSModel<key_t, val_t>::print_model(thread_id_t tid)
{
    std::cout<<" capacity:"<<capacity<<" -->";
    model->print_weights();
    print_keys();
}

template<class key_t, class val_t>
void Kanva_RSModel<key_t, val_t>::print_keys(thread_id_t tid)
{
    if (mobs_lf[0])
        {
            if (mobs_lf[0]->isbin)
                mobs_lf[0]->mob.lb->print(std::cout);
            else
                mobs_lf[0]->mob.ai->print_keys(tid);
        }

        for (size_t i = 0; i < capacity; i++)
        {
            std::cout << "keys[" << i << "]: " << model_array[i].key<< std::endl;
            if (mobs_lf[i + 1])
            {
                if (mobs_lf[i + 1]->isbin)
                    mobs_lf[i + 1]->mob.lb->print(std::cout);
                else
                    mobs_lf[i + 1]->mob.ai->print_keys(tid);
            }
        }
}

template<class key_t, class val_t>
void Kanva_RSModel<key_t, val_t>::print_model_retrain(thread_id_t tid)
{
    std::cout<<"[print aimodel] capacity:"<<capacity<<" -->";
    model->print_weights();
    if (mobs_lf[0])
        {
            if (mobs_lf[0]->isbin)
            {
                mobs_lf[0]->mob.lb->print(std::cout);
            }
            else
            {
                mobs_lf[0]->mob.ai->print_model_retrain(tid);
            }
        }
    for(size_t i=0; i<capacity; i++){
        std::cout << "keys[" << i << "]: " <<model_array[i].key<< std::endl;
        if (mobs_lf[i + 1])
        {
            if (mobs_lf[i + 1]->isbin)
            {
                mobs_lf[i + 1]->mob.lb->print(std::cout);
            }
            else
            {
                mobs_lf[i + 1]->mob.ai->print_model_retrain(tid);
            }
        }
    }
}



template<class key_t, class val_t>
void Kanva_RSModel<key_t, val_t>::self_check(thread_id_t tid)
{
    for(size_t i=1; i<capacity; i++){
        //assert(keys[i]>keys[i-1]);
        assert(model_array[i]->key>model_array[i-1]->key);
    }
    for(size_t i=0; i<=capacity; i++){
        model_or_bin_t *mob = mobs_lf[i];//CHECK
        if(mob){
            if(mob->isbin){
                //mob->mob.lb->self_check();CHECK
            } else {
                mob->mob.ai->self_check(tid);
            }
        }
    }
}





// ============================ search =====================
template<class key_t, class val_t>
bool Kanva_RSModel<key_t, val_t>::find_retrain(const key_t &key, val_t &val, thread_id_t tid)
{
    size_t pos = predict(key, tid);

    pos = locate_in_levelbin(key, pos, tid);
    //if(key == keys[pos]){
    if(key==model_array[pos]->key){
        val_t retrieved_val = model_array[pos]->getValue();
        // Check if the value is valid
        if (retrieved_val != (val_t)-1) {
            val = retrieved_val;  // Set the output value
            return true;          // Found the key
        }
        return false; // Key exists but value is invalid/deleted
    }
    int bin_pos = key<model_array[pos]->key?pos:(pos+1);
    model_or_bin_t *mob;
    mob = mobs_lf[bin_pos].load(std::memory_order_seq_cst);
   
    
        if (mob == nullptr)
            return false;
        if (mob->isbin){
            
            return mob->mob.lflb->search(key,val, tid);//CHECK
        }
        else
            return mob->mob.ai->find_retrain(key, val, tid);
}

/*template<class key_t, class val_t>
bool Kanva_RSModel<key_t, val_t>::con_find(const key_t &key, val_t &val)
{
    size_t pos = predict(key);
    //pos = find_lower(key, pos);
   // pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            val = vals[pos];
            return true;
        }
        return false;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    if(levelbins[bin_pos]==nullptr) return false;
    return levelbins[bin_pos]->con_find(key, val);
}*/

/*template<class key_t, class val_t>
result_t Kanva_RSModel<key_t, val_t>::con_find_retrain(const key_t &key, val_t &val)
{
    size_t pos = predict(key);
   // pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            val = vals[pos];
            return result_t::ok;
        }
        return result_t::failed;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    memory_fence();
    model_or_bin_t* mob = mobs[bin_pos];
    if(mob==nullptr) return result_t::failed;
    
    result_t res = result_t::failed;
    mob->lock();
    if(mob->isbin){
        res = mob->mob.lb->con_find_retrain(key, val);
        // while(res == result_t::retrain) {
        //     while(mob->locked == 1)
        //         ;
        //     res = mob->mob.lb->con_find_retrain(key, val);
        // }
        // return res;
    } else{
        res = mob->mob.ai->con_find_retrain(key, val);
    }
    assert(res!=result_t::retrain);
    mob->unlock();
    return res;
}*/


template<class key_t, class val_t>
inline size_t Kanva_RSModel<key_t, val_t>::predict(const key_t &key, thread_id_t tid) {
    size_t index_pos = model->predict(key);
    return index_pos < capacity? index_pos:capacity-1;
}



template<class key_t, class val_t>
inline size_t Kanva_RSModel<key_t, val_t>::locate_in_levelbin(const key_t &key, const size_t pos, thread_id_t tid)
{
    // predict
    //size_t index_pos = model->predict(key);
    int index_pos = pos;
    int upbound = capacity-1;
    //index_pos = index_pos <= upbound? index_pos:upbound;

    /*
    // search
    size_t begin, end, mid;
    if(key > model_array[index_pos]->key){
        begin = index_pos+1 < upbound? (index_pos+1):upbound;
        end = begin+maxErr < upbound? (begin+maxErr):upbound;
    } else {
        end = index_pos;
        begin = end>maxErr? (end-maxErr):0;
    }
    
    assert(begin<=end);
    while(begin != end){
        mid = (end + begin+2) / 2;
        if(model_array[mid]->key<=key) {
            begin = mid;
        } else
            end = mid-1;
    }
    return begin;
    */

    int i = index_pos;int e=0;
    if(i<0 || i>upbound) return -1;
    while(i<=upbound && model_array[i]->key < key){
        i+=(pow(2,e));
        e++;
    }
    
    if(i>upbound) i=upbound;
    int right=i;
    e=1;
    if(i<0 || i>upbound) return -1;
    while(i>0 && model_array[i]->key > key){
        i-=(pow(2,e));
        e++;
    }

    if(i<0) i=0;

    int left=i;
    //std::cout<<left<<" "<<right<<std::endl;
    /*
    for(int j=left;j<=right;j++){
        if(model_array[j]->key == key) return j;
    }
    */
    while (left <= right) {
        int m = left + (right - left) / 2;
 
        // Check if x is present at mid
        if (model_array[m]->key == key)
            return m;
 
        // If x greater, ignore left half
        if (model_array[m]->key < key)
            left = m + 1;
 
        // If x is smaller, ignore right half
        else
            right = m - 1;
    }
    return 0;
}




// ======================= update =========================
template<class key_t, class val_t>
result_t Kanva_RSModel<key_t, val_t>::update(const key_t &key, const val_t &val, thread_id_t tid)
{
    /*size_t pos = predict(key);
   // pos = locate_in_levelbin(key, pos);
    if(key == keys[pos]){
        if(valid_flag[pos]){
            vals[pos] = val;
            return result_t::ok;
        }
        return result_t::failed;
    }
    int bin_pos = key<keys[pos]?pos:(pos+1);
    memory_fence();
    model_or_bin_t* mob = mobs[bin_pos];
    if(mob==nullptr) return result_t::failed;
    
    result_t res = result_t::failed;
    mob->lock();
    if(mob->isbin){
        res = mob->mob.lb->update(key, val);
        // while(res == result_t::retrain) {
        //     while(mob->locked == 1)
        //         ;
        //     res = mob->mob.lb->update(key, val);
        // }
        // return res;
    } else{
        res = mob->mob.ai->update(key, val);
    }
    assert(res!=result_t::retrain);
    mob->unlock();
    return res;*/
    return result_t::ok;//CHECK
}




// =============================== insert =======================
template<class key_t, class val_t>
inline bool Kanva_RSModel<key_t, val_t>::insert_retrain(const key_t &key, const val_t &val,TrackerList *version_tracker, thread_id_t tid)
{
    size_t pos = predict(key, tid);
    pos = locate_in_levelbin(key, pos, tid);
    //std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
    if(key == model_array[pos]->key){
        if(model_array[pos]->getValue()!=-1){ //already inserted
            return false;
        } else{
            return model_array[pos]->insert(val,version_tracker);
            
        }
    }
    //std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
    int bin_pos = pos;
    bin_pos = key<model_array[bin_pos]->key?bin_pos:(bin_pos+1);
    //std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
    return insert_model_or_bin(key, val, bin_pos,version_tracker, tid);
}

template<class key_t, class val_t>
bool Kanva_RSModel<key_t, val_t>::insert_model_or_bin(const key_t &key, const val_t &val, size_t bin_pos,TrackerList *version_tracker, thread_id_t tid)
{
    // insert bin or model
    retry:
        model_or_bin_t *mob = mobs_lf[bin_pos];
    
   
        if (mob == nullptr)
        {
            model_or_bin_t *new_mob = new model_or_bin_t();
            new_mob->mob.lflb = new Linked_List<key_t, val_t>(llRecMgr, vnodeRecMgr, tid);
            new_mob->isbin = true;
            if (!mobs_lf[bin_pos].compare_exchange_strong(mob, new_mob, std::memory_order_seq_cst, std::memory_order_seq_cst))
                goto retry;
            mob = new_mob;
        }
        assert(mob != nullptr);
        if (mob->isbin)
        { // insert into bin
            int res = mob->mob.lflb->insert(key, val,version_tracker, tid);//CHECK return type
            if (res==-2)
            {
                std::vector<key_t> retrain_keys;
                std::vector<Vnode<val_t> *> version_lists;
                // TODO: Provide min threashold
               
                bool collectResult=mob->mob.lflb->collect(&retrain_keys,&version_lists, tid);
                if(collectResult==false) goto retry;
                lrmodel_type model;
                
                model.train(retrain_keys.begin(), retrain_keys.size());
                //std::cout << __FUNCTION__ << ":" << __LINE__ << std::endl;
                size_t err = model.get_maxErr();
                //std::cout<<"Error is "<<retrain_keys.size()<<" "<<retrain_vals.size()<<std::endl;
                
                kanva_RSmodel_type *ai = new kanva_RSmodel_type(model, retrain_keys.begin(),  retrain_keys.size(), err,version_lists, llRecMgr, vnodeRecMgr);

                if(mob->mob.lflb->isReclaimed) goto retry; //to prevent model overwrite
                model_or_bin_t *new_mob = new model_or_bin_t();
                //if(!mob->isbin) goto retry;
                new_mob->mob.ai = ai;
                new_mob->isbin = false;
                if (!mobs_lf[bin_pos].compare_exchange_strong(mob, new_mob))
                {
                    goto retry;
                }
                mob->mob.lflb->isReclaimed=true;
                mob->mob.lflb->reclaimMem(version_tracker->get_oldest_timestamp(), tid);
                return ai->insert_retrain(key, val,version_tracker, tid);
                
                //return (res==1 || res==0);
            }

            else return (res>=0);//res==0 || res==1
        }
        else 
        { // insert into model
            return mob->mob.ai->insert_retrain(key, val,version_tracker, tid);
        }
        return false;
}





// ========================== remove =====================
template<class key_t, class val_t>
bool Kanva_RSModel<key_t, val_t>::remove(const key_t &key,TrackerList *version_tracker, thread_id_t tid)
{
    size_t pos = predict(key, tid);
    pos = locate_in_levelbin(key, pos, tid);
    if(key ==model_array[pos]->key){
       
        if(model_array[pos]->getValue()!=-1){
            return model_array[pos]->insert(-1,version_tracker);
            
        }
        return false;
    }
    int bin_pos = key<model_array[pos]->key?pos:(pos+1);
    return remove_model_or_bin(key, bin_pos,version_tracker, tid);
}

template<class key_t, class val_t>
bool Kanva_RSModel<key_t, val_t>::remove_model_or_bin(const key_t &key, const int bin_pos,TrackerList *version_tracker, thread_id_t tid)
{   
   retry:
        model_or_bin_t *mob = mobs_lf[bin_pos];
        if (mob == nullptr)
            return false;
        if (mob->isbin)
        {
            int res;
            res = mob->mob.lflb->remove(key,version_tracker, tid);//CHECK remove
            /*if (res == -1)
                return false;*/
            if (res == -2)
            {
                /*std::vector<key_t> retrain_keys;
                std::vector<val_t> retrain_vals;
                mob->mob.lflb->collect(&retrain_keys,&retrain_vals,(version_tracker)->get_latest_timestamp());
                lrmodel_type model;
                model.train(retrain_keys.begin(), retrain_keys.size());
                size_t err = model.get_maxErr();
                kanva_RSmodel_type *ai = new kanva_RSmodel_type(model, retrain_keys.begin(), retrain_vals.begin(), retrain_keys.size(), err);
                model_or_bin_t *new_mob = new model_or_bin_t();
                new_mob->mob.ai = ai;
                new_mob->isbin = false;
                if (!mobs_lf[bin_pos].compare_exchange_strong(mob, new_mob))
                {
                    goto retry;
                }
                return ai->remove(key,version_tracker);
                */
            }
            else
                return  (res>=0);//res==0 || res==1
        }
        else
        {
            return mob->mob.ai->remove(key,version_tracker, tid);
        }
        return false;
    
    
}


// ========================== scan ===================
template<class key_t, class val_t>
int Kanva_RSModel<key_t, val_t>::scan(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result,TrackerList *version_tracker,int ts, thread_id_t tid)
{   
    
    size_t remaining = n;
    size_t pos = predict(key, tid);
    pos = locate_in_levelbin(key, pos, tid);
    while(remaining>0 && pos<=capacity) {
        if(pos<capacity && model_array[pos]->key>=key){
            val_t value = model_array[pos]->getVersionedValue(version_tracker,ts);
            if(value!=-1){
                result.push_back(std::pair<key_t, val_t>(model_array[pos]->key, value));
                remaining--;
                if(remaining<=0) break;
            }
        }
        
        pos++;
        if(pos>capacity) break;
        //std::cout<<(mobs_lf[pos+1].load(std::memory_order_seq_cst)==nullptr)<<std::endl;
        if(mobs_lf[pos].load(std::memory_order_seq_cst)){
            //std::cout<<"Bin"<<std::endl;
            model_or_bin_t *mob;
            mob = mobs_lf[pos];
            if(mob->isbin){
                remaining = mob->mob.lflb->range_query(key, remaining,ts,result,version_tracker, tid);//change for start key,n
                
            } else {
                //std::cout<<"Bin Model"<<std::endl;
                remaining = mob->mob.ai->scan(key, remaining, result,version_tracker,ts, tid);
            }
        }
        
        //pos++;
    }
    return remaining;
    
    return 0;
    
    //TODO:Range Query

}




} //namespace kanva_RS



#endif
