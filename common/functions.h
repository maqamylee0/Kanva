#ifndef KANVA_FUNCTIONS_H
#define KANVA_FUNCTIONS_H

#include <cstring>
#include "util.h"
#include "random"
#include "string"

void wiki_ts_200M_uint64();
void uniform_sparse_200M_uint64();
void lognormal_200M_uint64();
void osm_cellids_200M_uint64();
void normal_200M_uint64();
inline void load_data();
inline void parse_args(int, char **);
void fb_200M_uint64();
void books_200M_uint64();
void load_ycsb(const char *path);
void run_ycsb(const char *path);
void skew_data();

typedef struct operation_item {
    key_type key;
    int32_t range;
    uint8_t op;
} operation_item;

struct {
    size_t operate_num = 0;
    std::vector<operation_item> operate_queue;
} YCSBconfig;

struct config{
    int ds = 0;
    double read_ratio = 0.5;
    double insert_ratio = 0.3;
    double delete_ratio = 0.2;
    size_t item_num  = 200000000;
    size_t exist_num = 10000000;
    size_t runtime = 10;
    size_t thread_num = 64;
    size_t benchmark = 6;
    double skewness = 1;
}Config;

inline void parse_args(int argc, char **argv) {
    struct option long_options[] = {
            {"read", required_argument, 0, 'a'},
            {"insert", required_argument, 0, 'b'},
            {"remove", required_argument, 0, 'c'},
            {"ds", required_argument, 0, 'd'},
            {"runtime", required_argument, 0, 'f'},
            {"thread_num", required_argument, 0, 'g'},
            {"benchmark", required_argument, 0, 'h'},
            {"exist_num", required_argument, 0, 'i'},
            {0, 0, 0, 0}};
    std::string ops = "a:b:c:d:e:f:g:h:i:j:k:";
    int option_index = 0;

    while (1) {
        int c = getopt_long(argc, argv, ops.c_str(), long_options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                if (long_options[option_index].flag != 0) break;
                abort();
                break;
            case 'a':
                Config.read_ratio = strtod(optarg, NULL);
                INVARIANT(Config.read_ratio >= 0 && Config.read_ratio <= 1);
                std::cout<<"read_ratio is:  "<<Config.read_ratio<<"\n";
                break;
            case 'b':
                Config.insert_ratio = strtod(optarg, NULL);
                INVARIANT(Config.insert_ratio >= 0 && Config.insert_ratio <= 1);
                break;
            case 'c':
                Config.delete_ratio = strtod(optarg, NULL);
                INVARIANT(Config.delete_ratio >= 0 && Config.delete_ratio <= 1);
                break;
            case 'd':
                Config.ds = strtod(optarg, NULL);
                INVARIANT(Config.ds >= 0 && Config.ds <= 5);
                break;
            case 'f':
                Config.runtime = strtoul(optarg, NULL, 10);
                INVARIANT(Config.runtime > 0);
                break;
            case 'g':
                Config.thread_num = strtoul(optarg, NULL, 10);
                INVARIANT(Config.thread_num > 0);
                break;
            case 'h':
                Config.benchmark = strtoul(optarg, NULL, 10);
                INVARIANT(Config.benchmark >= 0 && Config.benchmark<13);
                break;
            case 'i':
                Config.exist_num = strtoul(optarg, NULL, 10);
                INVARIANT(Config.exist_num >= 0);
                break;
            default:
                abort();
        }
    }

    COUT_THIS("[micro] Read:Insert:Update:Delete:Scan = "
                      << Config.read_ratio << ":" << Config.insert_ratio << ":"
                      << Config.delete_ratio );
    double ratio_sum =
            Config.read_ratio + Config.insert_ratio + Config.delete_ratio;
    INVARIANT(ratio_sum > 0.9999 && ratio_sum < 1.0001);  // avoid precision lost
    COUT_VAR(Config.runtime);
    COUT_VAR(Config.thread_num);
    COUT_VAR(Config.benchmark);
}

void load_data() {
    switch (Config.benchmark) {
        case 0:
            books_200M_uint64();
            break;
        case 1:
            fb_200M_uint64();
            break;
        case 2:
            lognormal_200M_uint64();
            break;
        case 3:
            normal_200M_uint64();
            break;
        case 4:
            osm_cellids_200M_uint64();
            break;
        case 5:
            uniform_sparse_200M_uint64();
            break;
        case 6:
            wiki_ts_200M_uint64();
            break;
        case 7:
            load_ycsb("workload/loada_uniform.txt");
            run_ycsb("workload/runa_uniform.txt");
            COUT_THIS("load workloada_uniform");
            break;
        case 8:
            load_ycsb("workload/loadb_uniform.txt");
            run_ycsb("workload/runb_uniform.txt");
            COUT_THIS("load workloadb_uniform");
            break;
        case 9:
            load_ycsb("workload/loadc_uniform.txt");
            run_ycsb("workload/runc_uniform.txt");
            COUT_THIS("load workloadc_uniform");
            break;
        case 10:
            load_ycsb("workload/loada_ziphian.txt");
            run_ycsb("workload/runa_ziphian.txt");
            COUT_THIS("load workloada_ziphian");
            break;
            break;
        case 11:
            load_ycsb("workload/loadb_ziphian.txt");
            run_ycsb("workload/runb_ziphian.txt");
            COUT_THIS("load workloadb_ziphian");
            break;
        case 12:
            load_ycsb("workload/loadc_ziphian.txt");
            run_ycsb("workload/runc_ziphian.txt");
            COUT_THIS("load workloadc_ziphian");
            break;
        case 13:
            COUT_THIS("Skewed Data")
            skew_data();
            break;
        default:
            abort();
    }

    COUT_THIS("[processing data]")
    std::sort(exist_keys.begin(), exist_keys.end());
    exist_keys.erase(std::unique(exist_keys.begin(), exist_keys.end()), exist_keys.end());
    std::sort(exist_keys.begin(), exist_keys.end());
    for(size_t i=1; i<exist_keys.size(); i++){
        assert(exist_keys[i]>=exist_keys[i-1]);
    }
    COUT_VAR(exist_keys.size());
}

void books_200M_uint64() {
    std::cout << "books_200M_uint64\n";
    std::string filename = "data/books_200M_uint64";
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "unable to open " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    uint64_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    data1.resize(size);
    // Read values.
    in.read(reinterpret_cast<char *>(data1.data()), size * sizeof(key_type));
    in.close();
    std::cout << "Data Size  " << data1.size() << "\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> index_dis(0, data1.size() - 1);
    exist_keys.reserve(Config.exist_num);
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}

void fb_200M_uint64() {
    std::cout << "fb_200M_uint64\n";
    std::string filename = "data/fb_200M_uint64";
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "unable to open " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    uint64_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    data1.resize(size);
    // Read values.
    in.read(reinterpret_cast<char *>(data1.data()), size * sizeof(key_type));
    in.close();
    std::cout << "Data Size  " << data1.size() << "\n";
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}


void lognormal_200M_uint64() {
    std::cout << "lognormal_200M_uint64\n";
    std::string filename = "data/lognormal_200M_uint64";
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "unable to open " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    uint64_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    data1.resize(size);
    // Read values.
    in.read(reinterpret_cast<char *>(data1.data()), size * sizeof(key_type));
    in.close();
//    data1.erase(data.begin() + 199999990);
    std::cout << "Data Size  " << data1.size() << "\n";
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}

void normal_200M_uint64() {
    std::cout << "normal_200M_uint64\n";
    std::string filename = "data/normal_200M_uint64";
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "unable to open " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    uint64_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    data1.resize(size);
    // Read values.
    in.read(reinterpret_cast<char *>(data1.data()), size * sizeof(key_type));
    in.close();
    std::cout << "Data Size  " << data1.size() << "\n";
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}


void osm_cellids_200M_uint64() {
    std::cout << "osm_cellids_200M_uint64\n";
    std::string filename = "data/osm_cellids_200M_uint64";
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "unable to open " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    uint64_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    data1.resize(size);
    // Read values.
    in.read(reinterpret_cast<char *>(data1.data()), size * sizeof(key_type));
    in.close();
    std::cout << "Data Size  " << data1.size() << "\n";
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}

void uniform_dense_200M_uint64() {
    std::cout << "uniform_dense_200M_uint64\n";
    std::string filename = "data/uniform_dense_200M_uint64";
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "unable to open " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    uint64_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    data1.resize(size);
    // Read values.
    in.read(reinterpret_cast<char *>(data1.data()), size * sizeof(key_type));
    in.close();
    std::cout << "Data Size  " << data1.size() << "\n";
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}

void uniform_sparse_200M_uint64(){
    std::cout<<"uniform_sparse_200M_uint64\n";
    std::string filename = "data/uniform_sparse_200M_uint64";
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "unable to open " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    uint64_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
    data1.resize(size);
    // Read values.
    in.read(reinterpret_cast<char*>(data1.data()), size * sizeof(key_type));
    in.close();
    std::cout<<"Data Size  "<<data1.size()<<"\n";
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}

void wiki_ts_200M_uint64() {
    std::string filename = "data/wiki_ts_200M_uint64";
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "unable to open " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    uint64_t size;
    in.read(reinterpret_cast<char *>(&size), sizeof(uint64_t));
    data1.resize(size);
    // Read values.
    in.read(reinterpret_cast<char *>(data1.data()), size * sizeof(key_type));
    in.close();
    std::sort(data1.begin(), data1.end());
    data1.erase(std::unique(data1.begin(), data1.end()), data1.end());
    std::cout<<"Data Size::  "<<data1.size();
    for (int i = 1; i < data1.size(); i++) {
        if (data1[i] <= data1[i - 1]) {
            std::cout << data1[i] << "\n";
            std::cout << data1[i - 1] << "\n";
            std::cout << i << "\n";
        }
    }
    std::cout << "Data Size  " << data1.size() << "\n";
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}

void load_ycsb(const char *path) {
    FILE *ycsb, *ycsb_read;
    char *buf = NULL;
    size_t len = 0;
    size_t item_num = 0;
    char key[16];
    key_type dummy_key = 1234;
    if ((ycsb = fopen(path, "r")) == NULL) {
        perror("fail to read");
    }
    COUT_THIS("load data");
    int n = 10000000;
    exist_keys.reserve(n);
    while (getline(&buf, &len, ycsb) != -1) {
        if (strncmp(buf, "INSERT", 6) == 0) {
            memcpy(key, buf + 21, 16);
            dummy_key = strtoul(key, NULL, 10);
            exist_keys.push_back(dummy_key);
            item_num++;
        }

    }
    fclose(ycsb);
    uint64_t size = exist_keys.size();
    assert(exist_keys.size() == item_num);
    std::cerr << "load number : " << item_num << std::endl;
}

void run_ycsb(const char *path) {
    FILE *ycsb, *ycsb_read;
    char *buf = NULL;
    size_t len = 0;
    size_t query_i = 0, insert_i = 0, delete_i = 0, update_i = 0, scan_i = 0;
    size_t item_num = 0;
    char key[16];
    key_type dummy_key = 1234;


    if ((ycsb = fopen(path, "r")) == NULL) {
        perror("fail to read");
    }

    while (getline(&buf, &len, ycsb) != -1) {
        if (strncmp(buf, "READ", 4) == 0) {
            operation_item op_item;
            memcpy(key, buf + 19, 16);
            dummy_key = strtoul(key, NULL, 10);
            op_item.key = dummy_key;
            op_item.op = 0;
            YCSBconfig.operate_queue.push_back(op_item);
            item_num++;
            YCSBconfig.operate_num++;
            query_i++;
        } else if (strncmp(buf, "UPDATE", 6) == 0) {
            operation_item op_item;
            memcpy(key, buf + 21, 16);
            dummy_key = strtoul(key, NULL, 10);
            op_item.key = dummy_key;
            op_item.op = 1;
            YCSBconfig.operate_queue.push_back(op_item);
            item_num++;
            YCSBconfig.operate_num++;
            update_i++;
        } else if (strncmp(buf, "REMOVE", 6) == 0) {
            operation_item op_item;
            memcpy(key, buf + 21, 16);
            dummy_key = strtoul(key, NULL, 10);
            op_item.key = dummy_key;
            op_item.op = 2;
            YCSBconfig.operate_queue.push_back(op_item);
            item_num++;
            YCSBconfig.operate_num++;
            delete_i++;
        }
        else if(strncmp(buf, "INSERT", 6) == 0) {
            operation_item op_item;
            memcpy(key, buf+21, 16);
            dummy_key = strtoul(key, NULL, 10);
            op_item.key = dummy_key;
            op_item.op = 1;
            YCSBconfig.operate_queue.push_back(op_item);
            item_num++;
            YCSBconfig.operate_num++;
            insert_i++;
        }
        else {
            continue;
        }
    }
    fclose(ycsb);
    std::cerr << "  read: " << query_i << std::endl;
    std::cerr << "insert: " << insert_i << std::endl;
    std::cerr << "update: " << update_i << std::endl;
    std::cerr << "remove: " << delete_i << std::endl;
    std::cout<<"Total Item::  "<<item_num<<"   "<<YCSBconfig.operate_num<<"\n";
}

void skew_data()
{
    std::string filename = "data/uniform_sparse_200M_uint64";
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "unable to open " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    uint64_t size;
    in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
    data1.resize(size);
    // Read values.
    in.read(reinterpret_cast<char*>(data1.data()), size * sizeof(key_type));
    in.close();
    std::cout<<"Data Size  "<<data1.size()<<"\n";
    std::sample(
            data1.begin(),
            data1.end() - (data1.size() - data1.size()*Config.skewness),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}



#endif //KANVA_FUNCTIONS_H
