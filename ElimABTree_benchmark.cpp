#include <random>
#include <thread>
#include <fstream>
#include <cassert>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include "Elim-AB-Tree/adapter.h"
#include "functions.h"

const int NUM_THREADS = 160;
const int KEY_ANY = 0;
const int unused1 = 0;
void * unused2 = NULL;
RandomFNV1A * const unused3 = NULL;

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
    parse_args(argc, argv);
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
    Tree ->initThread(150);
    for(int i = 0; i < exist_keys.size(); i++)
        Tree->insertIfAbsent(150, exist_keys[i], (void*) 10);
    Tree->deinitThread(150);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "Table Preparation");
}

void run_benchmark(size_t sec) {
    std::thread threads[Config.thread_num];
    thread_param_t thread_params[Config.thread_num];
    for (size_t i = 0; i < Config.thread_num; i++) {
        if ((uint64_t)(&(thread_params[i])) % CACHELINE_SIZE != 0) {
            COUT_N_EXIT("wrong parameter address: " << &(thread_params[i]));
        }
    }
    int64_t numa1 = 0, numa2 = 32, numa3 = 64, numa4 = 96;
    running = false;
    for(size_t worker_i = 0; worker_i < Config.thread_num; worker_i++){
        thread_params[worker_i].thread_id = worker_i;
        thread_params[worker_i].throughput = 0;
        threads[worker_i] = std::thread (run_fg,(void *)&thread_params[worker_i] );
        cpu_set_t cpuset;
        int64_t cpu_pin;
        if(worker_i < 64){
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
    sleep(sec);
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
}

void *run_fg(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> ratio_dis(0, 1);
    std::uniform_real_distribution<> index_this(0, data1.size() - 1);
    std::uniform_real_distribution<> index_exist(0, exist_keys.size() - 1);
    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    size_t query_i = 0, insert_i = 0, delete_i = 0, update_i = 0;
    insert_i = index_this(gen);
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
