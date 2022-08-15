#include <random>
#include <thread>
#include <fstream>
#include <cassert>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include "AB-Tree/adapter.h"
#include "functions.h"

const int NUM_THREADS = std::thread::hardware_concurrency();;
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

void run_benchmark();
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
    run_benchmark();
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
    Tree ->initThread(NUM_THREADS - 1);
    for(int i = 0; i < exist_keys.size(); i++)
        Tree->insert(NUM_THREADS - 1, exist_keys[i], (void*) 10);
    Tree->deinitThread(NUM_THREADS - 1);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "Table Preparation");
}

void run_benchmark() {
    std::thread threads[Config.thread_num];
    thread_param_t thread_params[Config.thread_num];
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
    COUT_THIS("[micro] Throughput(op/s): " << (uint64_t)(throughput / time_s));
}

void *run_fg(void *param) {
    thread_param_t &thread_param = *(thread_param_t *)param;
    uint32_t thread_id = thread_param.thread_id;
//    aidel_type *ai = thread_param.ai;
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
            Tree->find(thread_id, opi.key);
        } else if (opi.op == 1) {
            Tree->insertIfAbsent(thread_id, opi.key, (void *)10);
        }
        else if (opi.op == 3) {
            Tree->erase(thread_id, opi.key);
        }
        thread_param.throughput++;
    }
    pthread_exit(nullptr);
}