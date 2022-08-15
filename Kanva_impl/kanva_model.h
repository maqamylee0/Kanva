//
// Created by gaurav on 6/8/22.
//

#ifndef KANVA_KANVA_MODEL_H
#define KANVA_KANVA_MODEL_H
#include "lr_model.h"
#include "lr_model_impl.h"
#include "util.h"
#include "Bin_LL/Bin.h"

template<class key_t, class val_t>
class KanvaModel {
public:
    typedef LinearRegressionModel<key_t> lrmodel_type;
    typedef KanvaModel<key_t, val_t> kanva_model_type;

    typedef struct model_or_bin {
        typedef union pointer{
            kanva_model_type * ai;
            Bin<key_t ,val_t>* lflb;
        }pointer_t;
        pointer_t mob;
        bool volatile isbin = true;   // true = lb, false = ai
        volatile uint8_t locked = 0;
    }model_or_bin_t;

public:
    inline KanvaModel();
    ~KanvaModel();
    KanvaModel(lrmodel_type &lrmodel, const typename std::vector<key_t>::const_iterator &keys_begin,
               const typename std::vector<val_t>::const_iterator &vals_begin,
               size_t size, size_t _maxErr);
    inline void print_model();
    void print_keys();
    void print_model_retrain();
    void self_check();
    void self_check_retrain();

    val_t find_retrain(const key_t &key, val_t &val);
    bool insert_retrain(const key_t &key, const val_t &val);
    bool remove(const key_t &key);

private:
    inline size_t predict(const key_t &key);
    inline size_t locate_in_levelbin(const key_t &key, const size_t pos);
    inline size_t find_lower(const key_t &key, const size_t pos);
    inline size_t linear_search(const key_t *arr, int n, key_t key);
    inline size_t find_lower_avx(const int *arr, int n, int key);
    inline size_t find_lower_avx(const int64_t *arr, int n, int64_t key);
    bool insert_model_or_bin(const key_t &key, const val_t &val, size_t bin_pos);
    bool remove_model_or_bin(const key_t &key, const int bin_pos);


private:
    lrmodel_type* model = nullptr;
    size_t maxErr = 64;
    size_t err = 0;
    key_t* keys = nullptr;
    val_t* vals = nullptr;
    bool* valid_flag = nullptr;
    std::atomic<model_or_bin_t*>* mobs_lf = nullptr;
    const size_t capacity;
};
#endif //KANVA_KANVA_MODEL_H
