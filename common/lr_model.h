
#ifndef KANVA_LR_MODEL_H
#define KANVA_LR_MODEL_H
#include <array>
#include <vector>
#include <stdlib.h>


template <class key_t>
class LinearRegressionModel{


public:
    inline LinearRegressionModel();
    inline LinearRegressionModel(double w, double b);
    ~LinearRegressionModel();
    void train(const typename std::vector<key_t>::const_iterator &it, size_t size);
    void train(const std::vector<key_t> &keys,
               const std::vector<size_t> &positions);
    void print_weights() const;
    size_t predict(const key_t &key) const;
    std::vector<size_t> predict(const std::vector<key_t> &keys) const;
    size_t max_error(const typename std::vector<key_t>::const_iterator &keys_begin,
                     size_t size);
    size_t max_error(const std::vector<key_t> &keys,
                     const std::vector<size_t> &positions);
    inline double get_weight0(){ return weights[0]; }
    inline double get_weight1(){ return weights[1]; }
    inline size_t get_maxErr() { return maxErr; }

private:
    size_t maxErr = 0;
    std::array<double, 2> weights;

};
#endif //KANVA_LR_MODEL_H
