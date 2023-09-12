#ifndef __KANVA_RS_IMPL_H__
#define __KANVA_RS_IMPL_H__

#include "kanva_RS.h"
#include "util.h"
#include "kanva_RS_model.h"
#include "kanva_RS_model_impl.h"
#include "piecewise_linear_model.h"



template<class key_t, class val_t>
inline Kanva_RS<key_t, val_t>::Kanva_RS(int num_threads)
    : maxErr(64), learning_step(1000), learning_rate(0.1), llRecMgr(new ll_record_manager_t(num_threads)), vnodeRecMgr(new vnode_record_manager_t(num_threads))
{
   
}
/*
template<class key_t, class val_t>
inline Kanva_RS<key_t, val_t>::Kanva_RS(int _maxErr, int _learning_step, float _learning_rate, ll_record_manager_t *__llRecMgr, vnode_record_manager_t *__vnodeRecMgr)
    : maxErr(_maxErr), learning_step(_learning_step), learning_rate(_learning_rate), llRecMgr(__llRecMgr), vnodeRecMgr(__vnodeRecMgr)
{
    
}
*/
template<class key_t, class val_t>
Kanva_RS<key_t, val_t>::~Kanva_RS(){
    
}

// ====================== train models ========================
template<class key_t, class val_t>
void Kanva_RS<key_t, val_t>::train(const std::vector<key_t> &keys, 
                                const std::vector<val_t> &vals, size_t _maxErr, thread_id_t tid)
{
    assert(keys.size() == vals.size());
    maxErr = _maxErr;
    std::cout<<"training begin, length of training_data is:" << keys.size() <<" ,maxErr: "<< maxErr << std::endl;

    size_t start = 0;
    size_t end = learning_step<keys.size()?learning_step:keys.size();
    while(start<end){
        
        lrmodel_type model;
        model.train(keys.begin()+start, end-start);
        /*
        size_t err = model.get_maxErr();
        if(err == maxErr) {
            append_model(model, keys.begin()+start, vals.begin()+start, end-start, err, tid);
        }else if(err < maxErr) {
            if(end>=keys.size()){
                append_model(model, keys.begin()+start, vals.begin()+start, end-start, err, tid);
                break;
            }
            end += learning_step;
            if(end>keys.size()){
                end = keys.size();
            }
            continue;
        } else {
            size_t offset = backward_train(keys.begin()+start, vals.begin()+start, end-start, int(learning_step*learning_rate), tid);
			end = start + offset;
        }
        */
        append_model(model, keys.begin()+start, vals.begin()+start, end-start, 0, tid);
        start = end;
        end += learning_step;
        if(end>=keys.size())
            end = keys.size();
       
    }

    
    COUT_THIS("[aidle] get models -> "<< model_keys.size());
    assert(model_keys.size()==aimodels.size());
    //std::cout<<"CHT"<<std::endl;
    /*
    key_t min1 = model_keys.front();
    key_t max1 = model_keys.back();
    const unsigned numBins = 128; // each node will have 128 separate bins
    const unsigned chtmaxError = 1; // the error of the index
    cht::Builder<key_t> chtb(min1, max1, numBins, chtmaxError, false,false);
    for (const auto& key2 : model_keys) chtb.AddKey(key2);
    cht = chtb.Finalize();
    */


    

}

template<class key_t, class val_t>
size_t Kanva_RS<key_t, val_t>::backward_train(const typename std::vector<key_t>::const_iterator &keys_begin, 
                                           const typename std::vector<val_t>::const_iterator &vals_begin,
                                           uint32_t size, int step, thread_id_t tid)
{   
    if(size<=10){
        step = 1;
    } else {
        while(size<=step){
            step = int(step*learning_rate);
        }
    }
    assert(step>0);
    size_t start = 0;
    size_t end = size-step;
    while(end>0){
        lrmodel_type model;
        model.train(keys_begin, end);
        size_t err = model.get_maxErr();
        if(err<=maxErr){
            append_model(model, keys_begin, vals_begin, end, err, tid);
            return end;
        }
        if(end>step)
        end -= step;
        else break;
    }
    end = backward_train(keys_begin, vals_begin, end, int(step*learning_rate), tid);
	return end;
}


template<class key_t, class val_t>
void Kanva_RS<key_t, val_t>::append_model(lrmodel_type &model, 
                                       const typename std::vector<key_t>::const_iterator &keys_begin, 
                                       const typename std::vector<val_t>::const_iterator &vals_begin, 
                                       size_t size, int err, thread_id_t tid)
{
    key_t key = *(keys_begin+size-1);
    
    // set learning_step
    /*
    int n = size/10;
    learning_step = 1;
    while(n!=0){
        n/=10;
        learning_step*=10;
    }
    */
    assert(err<=maxErr);
    kanva_RSmodel_type aimodel(model, keys_begin, vals_begin, size, 0, llRecMgr, vnodeRecMgr);

    model_keys.push_back(key);
    aimodels.push_back(aimodel);
}

template<class key_t, class val_t>
typename Kanva_RS<key_t, val_t>::kanva_RSmodel_type* Kanva_RS<key_t, val_t>::find_model(const key_t &key, thread_id_t tid)
{   //CHECK :add switch for biary search branchless
    /*
    size_t model_key=key;
    cht::SearchBound bound = cht.GetSearchBound(model_key);
    size_t model_pos=-1;
    
    for(int i=0;i<=2 && model_pos==-1;i++){
        if(bound.begin+i<model_keys.size()){
            if(model_keys[bound.begin+i]>=model_key) model_pos=bound.begin+i;
        }
    }
   */
 
    size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
    //std::cout<<model_pos2<<" "<<model_pos<<std::endl;

  if(model_pos >= aimodels.size())
        model_pos = aimodels.size()-1;
    return &aimodels[model_pos];
}


// ===================== print data =====================
template<class key_t, class val_t>
void Kanva_RS<key_t, val_t>::print_models(thread_id_t tid)
{
    
    for(int i=0; i<model_keys.size(); i++){
        std::cout<<"model "<<i<<" ,key:"<<model_keys[i]<<" ->";
        aimodels[i].print_model();
    }

    
    
}

template<class key_t, class val_t>
void Kanva_RS<key_t, val_t>::self_check(thread_id_t tid)
{
    for(int i=0; i<model_keys.size(); i++){
        aimodels[i].self_check(tid);
    }
    
}


// =================== search the data =======================
template<class key_t, class val_t>
inline result_t Kanva_RS<key_t, val_t>::find(const key_t &key, val_t &val, thread_id_t tid)
{   
    

    return find_model(key, tid)[0].find_retrain(key, val, tid) ? result_t::ok : result_t::failed;

}


// =================  scan ====================
template<class key_t, class val_t>
int Kanva_RS<key_t, val_t>::rangequery(const key_t &key, const size_t n, std::vector<std::pair<key_t, val_t>> &result, thread_id_t tid)
{       
    //int ts=((version_tracker.add_timestamp()))->ts;
    TrackerNode* tracker_node = version_tracker.add_timestamp();
    int ts = tracker_node->ts;

    size_t remaining = n;
    size_t model_pos = binary_search_branchless(&model_keys[0], model_keys.size(), key);
    if(model_pos >= aimodels.size())
        model_pos = aimodels.size()-1;
    while(remaining>0 && model_pos < aimodels.size()){
        remaining = aimodels[model_pos++].scan(key, remaining, result,&version_tracker,ts, tid);//CHECK:shouldn't model_pos increase by 1
    }
    tracker_node->finish = true;
    return remaining;

}



// =================== insert the data =======================
template<class key_t, class val_t>
inline result_t Kanva_RS<key_t, val_t>::insert(
        const key_t& key, const val_t& val, thread_id_t tid)
{
    return find_model(key, tid)[0].insert_retrain(key, val,&version_tracker, tid) ? result_t::ok : result_t::failed;
    //return find_model(key)[0].con_insert(key, val);
}


// ================ update =================
template<class key_t, class val_t>
inline result_t Kanva_RS<key_t, val_t>::update(
        const key_t& key, const val_t& val, thread_id_t tid)
{   //Not used anywhere
    return find_model(key, tid)[0].update(key, val, tid);
    //return find_model(key)[0].con_insert(key, val);
}


// ==================== remove =====================
template<class key_t, class val_t>
inline result_t Kanva_RS<key_t, val_t>::remove(const key_t& key, thread_id_t tid)
{
    return find_model(key, tid)[0].remove(key,&version_tracker, tid)
     ? result_t::ok : result_t::failed;
    //return find_model(key)[0].con_insert(key, val);
}

// ========================== using OptimalLPR train the model ==========================
template<class key_t, class val_t>
void Kanva_RS<key_t, val_t>::train_opt(const std::vector<key_t> &keys, 
                                    const std::vector<val_t> &vals, size_t _maxErr, thread_id_t tid)
{
    using pair_type = typename std::pair<size_t, size_t>;

    assert(keys.size() == vals.size());
    maxErr = _maxErr;
    std::cout<<"training begin, length of training_data is:" << keys.size() <<" ,maxErr: "<< maxErr << std::endl;

    segments = make_segmentation(keys.begin(), keys.end(), maxErr);
    COUT_THIS("[aidle] get models -> "<< segments.size());

   
}

template<class key_t, class val_t>
size_t Kanva_RS<key_t, val_t>::model_size(thread_id_t tid){
    return segments.size();
}





#endif
