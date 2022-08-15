#include <iostream>
#include "functions.h"
#include "Finedex/aidel.h"
#include "Finedex/aidel_impl.h"

typedef aidel::AIDEL<key_type, val_type> DS;
struct alignas(CACHELINE_SIZE) ThreadParam;
typedef ThreadParam thread_param_t;
volatile bool running = false;
std::atomic<size_t> ready_threads(0);

struct alignas(CACHELINE_SIZE) ThreadParam {
    DS *ds;
    uint64_t throughput;
    uint32_t thread_id;
};

void run_benchmark(DS *ai, size_t sec);
void *run_fg(void *param);
void prepare(DS *&ai);

int main(int argc, char **argv) {
    parse_args(argc, argv);
    load_data();
    DS* ds;
    prepare(ds);
    run_benchmark(ds, Config.runtime);
    return 0;
}

void prepare(DS *&ds){
    COUT_THIS("[Training Kanva]");
    double time_s = 0.0;
    TIMER_DECLARE(0);
    TIMER_BEGIN(0);
    size_t maxErr = 4;
    ds = new DS();
    ds->train(exist_keys, exist_keys, 32);
    TIMER_END_S(0,time_s);
    printf("%8.1lf s : %.40s\n", time_s, "training");
}

void run_benchmark(DS *ds, size_t sec) {
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
        thread_params[worker_i].ds = ds;
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
    DS *ds = thread_param.ds;

    size_t key_n_per_thread = YCSBconfig.operate_num / Config.thread_num;
    size_t key_start = thread_id * key_n_per_thread;
    size_t key_end = (thread_id + 1) * key_n_per_thread;
    std::vector<std::pair<key_type, val_type>> result;


    COUT_THIS("[micro] Worker" << thread_id << " Ready.");
    ready_threads++;
    val_type dummy_value = 1234;

    while (!running)
        ;
    for(int i=key_start; i<key_end; i++) {
        operation_item opi = YCSBconfig.operate_queue[i];
        if(opi.op == 0){     // read
            ds->find(opi.key, dummy_value);
        } else if (opi.op == 1) {
            ds->insert(opi.key, opi.key);
        }
        else if (opi.op == 3) {
            ds->remove(opi.key);
        }
        thread_param.throughput++;
    }
    pthread_exit(nullptr);
}