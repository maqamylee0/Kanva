
#include <random>
#include <thread>
#include <fstream>
#include <cassert>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include "adapter.h"
#include "functions.h"

const int NUM_THREADS = 160;
const int KEY_ANY = 0;
const int unused1 = 0;
void * unused2 = NULL;
Random64 * const unused3 = NULL;

auto Tree = new ds_adapter<int, void *>(NUM_THREADS, KEY_ANY, unused1, unused2, unused3);

struct alignas(CACHELINE_SIZE) ThreadParam;
typedef ThreadParam thread_param_t;

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) ThreadParam {
    uint64_t throughput;
    uint32_t thread_id;
};

void run_benchmark(size_t sec);
void *run_fg(void *param);
void prepare();

std::ofstream fout;
int main(int argc, char **argv) {
    fout.open("AB_Tree_Result.txt", std::ios::app);
    Config.thread_num = atoi(argv[1]);
    Config.read_ratio= ((double)atoi(argv[2]))/100;
    Config.insert_ratio = ((double)atoi(argv[3]))/100;
    Config.delete_ratio = ((double)atoi(argv[4]))/100;
    Config.benchmark = ((double)atoi(argv[5]));
//    Config.skewness= ((double)atoi(argv[6]))/100;
    std::cout<<"AB-Tree\n";
    load_data();
    for(int i = 0; i < NUM_THREADS; i++){
        Tree -> initThread(i);
    }
    prepare();
    run_benchmark(Config.runtime);
    fout.close();
    for(int i = 0; i < NUM_THREADS; i++){
        Tree -> deinitThread(i);
    }
    return 0;
}

void prepare(){
    COUT_THIS("[Preparing Table]");
    double time_s = 0.0;
    TIMER_DECLARE(0);
    TIMER_BEGIN(0);
    size_t maxErr = 4;
    Tree ->initThread(150);
    for(int i = 0; i < exist_keys.size(); i++)
        Tree->insert(150, exist_keys[i], (void*) 10);
    Tree->deinitThread(150);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "Table Preparation");
}

void run_benchmark(size_t sec) {
    std::thread threads[Config.thread_num];
    thread_param_t thread_params[Config.thread_num];
    // check if parameters are cacheline aligned
    for (size_t i = 0; i < Config.thread_num; i++) {
        if ((uint64_t)(&(thread_params[i])) % CACHELINE_SIZE != 0) {
            COUT_N_EXIT("wrong parameter address: " << &(thread_params[i]));
        }
    }

    running = false;
    int64_t numa1 = 0, numa2 = 26, numa3 = 52, numa4 = 78;
    for(size_t worker_i = 0; worker_i < Config.thread_num; worker_i++){
        thread_params[worker_i].thread_id = worker_i;
        thread_params[worker_i].throughput = 0;
        threads[worker_i] = std::thread (run_fg,(void *)&thread_params[worker_i] );
//        int ret = pthread_create(&threads[worker_i], nullptr, run_fg,
//                                 (void *)&thread_params[worker_i]);
//        if (ret) {
//            COUT_N_EXIT("Error:" << ret);
//        }
        cpu_set_t cpuset;
        int64_t cpu_pin;
        if(worker_i < 52){
            if(worker_i % 2 == 0){
                cpu_pin = numa1;
                numa1++;
            }
            else{
                cpu_pin = numa2;
                numa2++;
            }
        }
        else{
            if(worker_i % 2 == 0){
                cpu_pin = numa3;
                numa3++;
            }
            else{
                cpu_pin = numa4;
                numa4++;
            }
        }
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_pin, &cpuset);
        pthread_setaffinity_np (threads[worker_i].native_handle (),
                                sizeof (cpu_set_t), &cpuset);
    }

    COUT_THIS("[micro] prepare data ...");
    while (ready_threads < Config.thread_num) sleep(0.5);

    running = true;
    std::vector<size_t> tput_history(Config.thread_num, 0);
    size_t current_sec = 0;
    while (current_sec < sec) {
        sleep(1);
        uint64_t tput = 0;
        for (size_t i = 0; i < Config.thread_num; i++) {
            tput += thread_params[i].throughput - tput_history[i];
            tput_history[i] = thread_params[i].throughput;
        }
        COUT_THIS("[micro] >>> sec " << current_sec << " throughput: " << tput);
        ++current_sec;
    }

    running = false;
    void *status;
    for (size_t i = 0; i < Config.thread_num; i++) {
        threads[i].join();
    }

    size_t throughput = 0;
    for (auto &p : thread_params) {
        throughput += p.throughput;
    }
    COUT_THIS("[micro] Throughput(op/s): " << throughput / sec);
    COUT_THIS("[micro] Total Throughput(op/s): " << throughput );
    std::cout<<Config.thread_num<<"   "<<Config.read_ratio<<"    "<<Config.insert_ratio<<"    "<<Config.delete_ratio<<"    "<<Config.benchmark<<"    "<<Config.skewness<<"    "<<throughput/sec<<"\n";
    fout<<Config.thread_num<<"   "<<Config.read_ratio<<"    "<<Config.insert_ratio<<"    "<<Config.delete_ratio<<"    "<<Config.benchmark<<"    "<<Config.skewness<<"    "<<throughput/sec<<"\n";
//    int64_t model_size = 0;
//    {
//        for(int i = 0; i < ai->model_size(); i++)
//        {
//            model_size += ai->aimodels[i].
//        }
//    }
}

void *run_fg(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
//    aidel_type *ai = thread_param.ai;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> ratio_dis(0, 1);
    std::uniform_real_distribution<> index_this(0, data1.size() - 1);
    std::uniform_real_distribution<> index_exist(0, exist_keys.size() - 1);

    size_t non_exist_key_n_per_thread = non_exist_keys.size() / Config.thread_num;
    size_t non_exist_key_start = thread_id * non_exist_key_n_per_thread;
    size_t non_exist_key_end = (thread_id + 1) * non_exist_key_n_per_thread;
    std::vector<key_type> op_keys(non_exist_keys.begin() + non_exist_key_start,
                                  non_exist_keys.begin() + non_exist_key_end);

    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    size_t query_i = 0, insert_i = 0, delete_i = 0, update_i = 0;
    insert_i = index_this(gen);
    query_i = index_exist(gen);
    delete_i = index_exist(gen);
    // exsiting keys fall within range [delete_i, insert_i)
    ready_threads++;
    volatile result_t res = result_t::failed;
    val_type dummy_value = 1234;

    while (!running)
        ;
    while (running) {
        double d = ratio_dis(gen);
        if (d <= Config.read_ratio) {                   // search
            key_type dummy_key = exist_keys[query_i % exist_keys.size()];

            Tree->find(thread_id, dummy_key);
            query_i++;
            if (unlikely(query_i == exist_keys.size())) {
                query_i = 0;
            }
        } else if (d <= Config.read_ratio+Config.insert_ratio) {  // insert
            key_type dummy_key = data1[insert_i % data1.size()];
            Tree->insertIfAbsent(thread_id, dummy_key, (void *)10);
            insert_i++;
            if (unlikely(insert_i == data1.size())) {
                insert_i = 0;
            }
//        }
//        } else if (d <= Config.read_ratio+Config.insert_ratio+Config.update_ratio) {    // update
//            key_type dummy_key = non_exist_keys[update_i % non_exist_keys.size()];
//            res = ai->update(dummy_key, dummy_key);
//            update_i++;
//            if (unlikely(update_i == non_exist_keys.size())) {
//                update_i = 0;
//            }
        }  else {                // remove
            key_type dummy_key = exist_keys[delete_i % exist_keys.size()];
            Tree->erase(thread_id, dummy_key);
            delete_i++;
            if (unlikely(delete_i == exist_keys.size())) {
                delete_i = 0;
            }
        }
        thread_param.throughput++;
    }
    pthread_exit(nullptr);
}
