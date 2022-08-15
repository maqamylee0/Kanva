#include <iostream>
#include "functions.h"
#include <limits>
#include <cassert>
#include <fstream>
#include "thread"
#include "CIST/adapter.h"

struct alignas(CACHELINE_SIZE) ThreadParam;
typedef ThreadParam thread_param_t;
const int NUM_THREADS = Config.thread_num;
const uint64_t KEY_ANY = 0;
const uint64_t unused1 = 0;
void * unused2 = NULL;
Random64 * const unused3 = NULL;

auto tree = new ds_adapter<int64_t , void *>(NUM_THREADS, KEY_ANY, unused1, unused2, unused3);

volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) ThreadParam {
    uint64_t throughput;
    uint32_t thread_id;
};

void run_benchmark();
void *run_fg(void *param);
void prepare();

std::ofstream fout;
int main(int argc, char **argv) {
    parse_args(argc, argv);
    load_data();
    prepare();
    run_benchmark();
    delete tree;
    fout.close();
    return 0;
}

void prepare(){
    COUT_THIS("[Preparing Table]");
    double time_s = 0.0;
    TIMER_DECLARE(0);
    TIMER_BEGIN(0);
    size_t maxErr = 4;
    tree ->initThread(0);
    for(int i = 0; i < exist_keys.size(); i++)
        tree->insertIfAbsent(0, exist_keys[i], (void*)10);
    tree->deinitThread(0);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "Table Preparation");
}
void run_benchmark() {
    std::thread threads[Config.thread_num];
    thread_param_t thread_params[Config.thread_num];
    // check if parameters are cacheline aligned
    for (size_t i = 0; i < Config.thread_num; i++) {
        if ((uint64_t)(&(thread_params[i])) % CACHELINE_SIZE != 0) {
            COUT_N_EXIT("wrong parameter address: " << &(thread_params[i]));
        }
    }

    running = false;
    int64_t numa1 = 0, numa2 = 32, numa3 = 64, numa4 = 96;
    for(size_t worker_i = 0; worker_i < Config.thread_num; worker_i++){
        thread_params[worker_i].thread_id = worker_i;
        thread_params[worker_i].throughput = 0;
        threads[worker_i] = std::thread (run_fg,(void *)&thread_params[worker_i] );
        cpu_set_t cpuset;
        int64_t cpu_pin;
        if(worker_i < 80){
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

    double time_ns;
    double time_s;
    TIMER_DECLARE(1);
    TIMER_BEGIN(1);
    running = true;
    void *status;
    for (size_t i = 0; i < Config.thread_num; i++) {
        threads[i].join();
    }
    TIMER_END_NS(1,time_ns);
    TIMER_END_S(1,time_s);

    size_t throughput = 0;
    for (auto &p : thread_params) {
        throughput += p.throughput;
    }
    COUT_THIS("[micro] Throughput(op/s): " << throughput / time_s);
}

void *run_fg(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
    tree->initThread(thread_id);
    size_t key_n_per_thread = YCSBconfig.operate_num / Config.thread_num;
    size_t key_start = thread_id * key_n_per_thread;
    size_t key_end = (thread_id + 1) * key_n_per_thread;
    std::vector<std::pair<key_type, val_type>> result;
    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    ready_threads++;
    volatile result_t res = result_t::failed;
    val_type dummy_value = 1234;

    while (!running)
        ;
    for(int i=key_start; i<key_end; i++) {
        operation_item opi = YCSBconfig.operate_queue[i];
        if(opi.op == 0){     // read
            tree->find(thread_id, opi.key);
        } else if (opi.op == 1) {
            tree->insertIfAbsent(thread_id, opi.key, (void *)10);
        }
        else if (opi.op == 3) {
            tree->erase(thread_id, opi.key);
        }
        thread_param.throughput++;
    }
    pthread_exit(nullptr);
}