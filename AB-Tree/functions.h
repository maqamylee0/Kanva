#include "util.h"

typedef uint64_t key_type;
typedef uint64_t val_type;

std::vector<key_type> exist_keys;
std::vector<key_type> non_exist_keys;

void wiki_ts_200M_uint64();
void uniform_dense_200M_uint64();
void uniform_sparse_200M_uint64();
void lognormal_200M_uint64();
void osm_cellids_200M_uint64();
void normal_200M_uint64();
inline void load_data();
inline void parse_args(int, char **);
void fb_200M_uint64();
void books_200M_uint64();
void uniform_data();
void normal_data();
void lognormal_data();
void timestamp_data();
void documentid_data();
void lognormal_data_with_insert_factor();
void skew_data();
void load_ycsb(const char *path);
void run_ycsb(const char *path);
std::vector<int> read_data(const char *path);
template<typename key_type>
std::vector<key_type> read_timestamp(const char *path);
template<typename key_type>
std::vector<key_type> read_document(const char *path);

typedef struct operation_item {
    key_type key;
    int32_t range;
    uint8_t op;
} operation_item;

// parameters
struct config{
    double read_ratio = 0.5;
    double insert_ratio = 0.5;
    double update_ratio = 0;
    double delete_ratio = 0;
    size_t item_num  = 20000000;
    size_t exist_num = 10000000;
    size_t runtime = 10;
    size_t thread_num = 128;
    size_t benchmark = 0;
    size_t insert_factor = 1;
    double skewness = 0.05;
}Config;

struct {
    size_t operate_num = 0;
    std::vector<operation_item> operate_queue;
} YCSBconfig;

inline void parse_args(int argc, char **argv) {
    struct option long_options[] = {
            {"read", required_argument, 0, 'a'},
            {"insert", required_argument, 0, 'b'},
            {"remove", required_argument, 0, 'c'},
            {"update", required_argument, 0, 'd'},
            {"item_num", required_argument, 0, 'e'},
            {"runtime", required_argument, 0, 'f'},
            {"thread_num", required_argument, 0, 'g'},
            {"benchmark", required_argument, 0, 'h'},
            {"insert_factor", required_argument, 0, 'i'},
            {"skewness", required_argument, 0, 'j'},
            {"exist_num", required_argument, 0, 'k'},
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
                Config.update_ratio = strtod(optarg, NULL);
                INVARIANT(Config.update_ratio >= 0 && Config.update_ratio <= 1);
                break;
            case 'e':
                Config.item_num = strtoul(optarg, NULL, 10);
                INVARIANT(Config.item_num > 0);
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
                Config.insert_factor = strtoul(optarg, NULL, 10);
                INVARIANT(Config.insert_factor >= 0);
                break;
            case 'j':
                Config.skewness = strtod(optarg, NULL);
                INVARIANT(Config.skewness > 0 && Config.skewness <= 1);
                break;
            case 'k':
                Config.exist_num = strtoul(optarg, NULL, 10);
                INVARIANT(Config.exist_num > 0);
                break;
            default:
                abort();
        }
    }

    COUT_THIS("[micro] Read:Insert:Update:Delete:Scan = "
                      << Config.read_ratio << ":" << Config.insert_ratio << ":" << Config.update_ratio << ":"
                      << Config.delete_ratio );
    double ratio_sum =
            Config.read_ratio + Config.insert_ratio + Config.delete_ratio + Config.update_ratio;
    INVARIANT(ratio_sum > 0.9999 && ratio_sum < 1.0001);  // avoid precision lost
    COUT_VAR(Config.runtime);
    COUT_VAR(Config.thread_num);
    COUT_VAR(Config.benchmark);
}

void ZIPH0();
void ZIPH1();
void ZIPH0_200();
void ZIPH1_200();

void load_data(){
    switch (Config.benchmark) {
        case 0:
            books_200M_uint64();
//            ZIPH0();
            break;
        case 1:
//            ZIPH1();
            fb_200M_uint64();
            break;
        case 2:
//            ZIPH0_200();
            lognormal_200M_uint64();
            break;
        case 3:
//            ZIPH1_200();
            normal_200M_uint64();
            break;
            // === used for insert_factor and skew_data
        case 4:
            osm_cellids_200M_uint64();
            break;
        case 5:
            uniform_dense_200M_uint64();
            break;
            // ===== case 6--11 YCSB ===================
        case 6:
            uniform_sparse_200M_uint64();
            //load_ycsb("../../ycsb/workloads/run.100K");
//            load_ycsb("../FINEdex2/workloads/ycsb_data/workloada/load.10M");
//            run_ycsb("../FINEdex2/workloads/ycsb_data/workloada/run.10M");
//            COUT_THIS("load workloada");
            break;
        case 7:
            wiki_ts_200M_uint64();
//            load_ycsb("../FINEdex2/workloads/ycsb_data/workloadb/load.10M");
//            run_ycsb("../FINEdex2/workloads/ycsb_data/workloadb/run.10M");
//            COUT_THIS("load workloadb");
            break;
        case 8:
            load_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/loada_uniform.txt");
            run_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/runa_uniform.txt");
            COUT_THIS("load workloadc");
            break;
        case 9:
            load_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/loadb_uniform.txt");
            run_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/runb_uniform.txt");
            COUT_THIS("load workloadd");
            break;
        case 10:
            load_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/loadc_uniform.txt");
            run_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/runc_uniform.txt");
            COUT_THIS("load workloade");
            break;
        case 11:
            load_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/loada_ziphian.txt");
            run_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/runa_ziphian.txt");
            COUT_THIS("load workloade");
            break;
            break;
        case 12:
            load_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/loadb_ziphian.txt");
            run_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/runb_ziphian.txt");
            COUT_THIS("load workloadf");
            break;
        case 13:
            load_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/loadc_ziphian.txt");
            run_ycsb("/home/student/2019/cs19resch11003/YCSB/ycsb-0.17.0/workload/runc_ziphian.txt");
            COUT_THIS("load workloadf");
            break;
        case 14:
            COUT_THIS("Skewed Data");
            skew_data();
            break;
        default:
            abort();
    }

    // initilize XIndex (sort keys first)
//    COUT_THIS("[processing data]")
//    for(size_t i=1; i<exist_keys.size(); i++){
//        assert(exist_keys[i]>=exist_keys[i-1]);
//    }
//    std::sort(exist_keys.begin(), exist_keys.end());
//    exist_keys.erase(std::unique(exist_keys.begin(), exist_keys.end()), exist_keys.end());
//    std::sort(exist_keys.begin(), exist_keys.end());
//    for(size_t i=1; i<exist_keys.size(); i++){
//        assert(exist_keys[i]>=exist_keys[i-1]);
//    }
    //std::vector<val_type> vals(exist_keys.size(), 1);

    COUT_VAR(exist_keys.size());
    COUT_VAR(non_exist_keys.size());
}


void uniform_data(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> rand_uniform(0, Config.exist_num*100);

    exist_keys.reserve(Config.exist_num);
    for (size_t i = 0; i < Config.exist_num; ++i) {
        exist_keys.push_back(i*100);
    }
    if (Config.insert_ratio > 0) {
        non_exist_keys.reserve(Config.item_num);
        for (size_t i = 0; i < Config.item_num; ++i) {
            key_type a = rand_uniform(gen);
            non_exist_keys.push_back(a);
        }
    }
}

void ZIPH0()
{
    std::ifstream reader;
    reader.open("/home/student/2019/cs19resch11003/ZIPH0.txt", std::ios::in);
    std::string line;
    while(std::getline(reader, line))
    {
        data1.push_back(std::stoll(line));
    }
    std::cout<<data1.size()<<"\n";
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::cout<<exist_keys.size()<<"\n";

}

void ZIPH1()
{
    std::ifstream reader;
    reader.open("/home/student/2019/cs19resch11003/ZIPH1.txt", std::ios::in);
    std::string line;
    while(std::getline(reader, line))
    {
        data1.push_back(std::stoll(line));
    }
    std::cout<<data1.size()<<"\n";
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::cout<<exist_keys.size()<<"\n";

}

void ZIPH0_200()
{
    std::ifstream reader;
    reader.open("/home/student/2019/cs19resch11003/ZIPH1_200.txt", std::ios::in);
    std::string line;
    while(std::getline(reader, line))
    {
        data1.push_back(std::stoll(line));
    }
    std::cout<<data1.size()<<"\n";
    Config.exist_num = 100000000;
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::cout<<exist_keys.size()<<"\n";

}

void ZIPH1_200()
{
    std::ifstream reader;
    reader.open("/home/student/2019/cs19resch11003/ZIPH1_200.txt", std::ios::in);
    std::string line;
    while(std::getline(reader, line))
    {
        data1.push_back(std::stoll(line));
    }
    std::cout<<data1.size()<<"\n";
    Config.exist_num = 100000000;
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
    std::cout<<exist_keys.size()<<"\n";

}


void books_200M_uint64(){
    std::cout<<"books_200M_uint64\n";
    std::string filename = "/home/student/2019/cs19resch11003/Learned_Index/SOSD-master/data/books_200M_uint64";
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
    data1[0] = 1;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> index_dis(0, data1.size() - 1);
    exist_keys. reserve(Config.exist_num);
//    for (size_t i = 0; i < Config.exist_num; ++i) {
//        key_type a = index_dis(gen);
//        if(a<0) {
//            i--;
//            continue;
//        }
//        exist_keys. push_back(a);
//    }
    std::sample(
            data1.begin(),
            data1.end(),
            std::back_inserter(exist_keys),
            Config.exist_num,
            std::mt19937{std::random_device{}()}
    );
//    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(data1.begin(), data1.end(), g);
}

void fb_200M_uint64(){
    std::cout<<"fb_200M_uint64\n";
    std::string filename = "/home/student/2019/cs19resch11003/Learned_Index/SOSD-master/data/fb_200M_uint64";
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



void lognormal_200M_uint64(){
    std::cout<<"lognormal_200M_uint64\n";
    std::string filename = "/home/student/2019/cs19resch11003/Learned_Index/SOSD-master/data/lognormal_200M_uint64";
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
//    data.erase(data.begin() + 199999990);
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

void normal_200M_uint64(){
    std::cout<<"normal_200M_uint64\n";
    std::string filename = "/home/student/2019/cs19resch11003/Learned_Index/SOSD-master/data/normal_200M_uint64";
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
//    data.erase(data.begin() + 199999990);
//    for(int i = 1; i < data.size(); i++)
//    {
//        if(data[i] <= data[i-1])
//        {
//            std::cout<<data[i]<<"\n";
//            std::cout<<data[i-1]<<"\n";
//            std::cout<<i<<"\n";
//
//
//        }
//    }
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


void osm_cellids_200M_uint64(){
    std::cout<<"osm_cellids_200M_uint64\n";
    std::string filename = "/home/student/2019/cs19resch11003/Learned_Index/SOSD-master/data/osm_cellids_200M_uint64";
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
//    data.erase(data.begin() + 199999990);
//    for(int i = 1; i < data.size(); i++)
//    {
//        if(data[i] <= data[i-1])
//        {
//            std::cout<<data[i]<<"\n";
//            std::cout<<data[i-1]<<"\n";
//            std::cout<<i<<"\n";
//
//
//        }
//    }
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

void uniform_dense_200M_uint64(){
    std::cout<<"uniform_dense_200M_uint64\n";
    std::string filename = "/home/student/2019/cs19resch11003/Learned_Index/SOSD-master/data/uniform_dense_200M_uint64";
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
//    data.erase(data.begin() + 199999990);
//    for(int i = 1; i < data.size(); i++)
//    {
//        if(data[i] <= data[i-1])
//        {
//            std::cout<<data[i]<<"\n";
//            std::cout<<data[i-1]<<"\n";
//            std::cout<<i<<"\n";
//
//
//        }
//    }
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

void uniform_sparse_200M_uint64(){
    std::cout<<"uniform_sparse_200M_uint64\n";
    std::string filename = "/home/student/2019/cs19resch11003/Learned_Index/SOSD-master/data/uniform_sparse_200M_uint64";
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
//    data.erase(data.begin() + 199999990);
//    for(int i = 1; i < data.size(); i++)
//    {
//        if(data[i] <= data[i-1])
//        {
//            std::cout<<data[i]<<"\n";
//            std::cout<<data[i-1]<<"\n";
//            std::cout<<i<<"\n";
//
//
//        }
//    }
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

void wiki_ts_200M_uint64(){
    std::string filename = "/home/student/2019/cs19resch11003/Learned_Index/SOSD-master/data/wiki_ts_200M_uint64";
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
//    data.erase(data.begin() + 199999990);
//    for(int i = 1; i < data1.size(); i++)
//    {
//        if(data1[i] <= data1[i-1])
//        {
//            std::cout<<data1[i]<<"\n";
//            std::cout<<data1[i-1]<<"\n";
//            std::cout<<i<<"\n";
//
//
//        }
//    }
    std::sort(data1.begin(), data1.end());
    data1.erase(std::unique(data1.begin(), data1.end()), data1.end());
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

void lognormal_data(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::lognormal_distribution<double> rand_lognormal(0, 2);

    exist_keys.reserve(Config.exist_num);
    for (size_t i = 0; i < Config.exist_num; ++i) {
        key_type a = rand_lognormal(gen)*1000000000000;
        assert(a>0);
        exist_keys.push_back(a);
    }
    if (Config.insert_ratio > 0) {
        non_exist_keys.reserve(Config.item_num);
        for (size_t i = 0; i < Config.item_num; ++i) {
            key_type a = rand_lognormal(gen)*1000000000000;
            assert(a>0);
            non_exist_keys.push_back(a);
        }
    }
}
void timestamp_data(){
    //assert(Config.read_ratio == 1);
    //std::vector<key_type> read_keys = read_timestamp<key_type>("../FINEdex2/workloads/timestamp.sorted.200M");
    std::vector<key_type> read_keys = read_timestamp<key_type>("osm/osm_cellids_800M_uint64");
    size_t size = read_keys.size();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rand_int32(
            0, size);

    if(Config.insert_ratio > 0) {
        size_t train_size = size/10;
        exist_keys.reserve(train_size);
        for(int i=0; i<train_size; i++){
            exist_keys.push_back(read_keys[rand_int32(gen)]);
        }

        non_exist_keys.reserve(size);
        for(int i=0; i<size; i++){
            non_exist_keys.push_back(read_keys[i]);
        }
    } else {
        size_t num = Config.exist_num < read_keys.size()? Config.exist_num:read_keys.size();
        exist_keys.reserve(num);
        for(int i=0; i<num; i++){
            exist_keys.push_back(read_keys[rand_int32(gen)]);
        }
    }
}
void documentid_data(){
    //assert(Config.read_ratio == 1);
    std::vector<key_type> read_keys = read_timestamp<key_type>("../FINEdex2/workloads/document-id.sorted.10M");
    size_t size = read_keys.size();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rand_int32(
            0, size);

    if(Config.insert_ratio > 0) {
        size_t train_size = size/10;
        exist_keys.reserve(train_size);
        for(int i=0; i<train_size; i++){
            exist_keys.push_back(read_keys[rand_int32(gen)]);
        }

        non_exist_keys.reserve(size);
        for(int i=0; i<size; i++){
            non_exist_keys.push_back(read_keys[i]);
        }
    } else {
        size_t num = Config.exist_num < read_keys.size()? Config.exist_num:read_keys.size();
        exist_keys.reserve(num);
        for(int i=0; i<num; i++){
            exist_keys.push_back(read_keys[rand_int32(gen)]);
        }
    }
}
void lognormal_data_with_insert_factor(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::lognormal_distribution<double> rand_lognormal(0, 2);

    COUT_THIS("[gen_data] insert_factor: " <<Config.insert_factor);
    exist_keys.reserve(Config.exist_num);
    for (size_t i = 0; i < Config.exist_num; ++i) {
        key_type a = rand_lognormal(gen)*1000000000000;
        if(a<0) {
            i--;
            continue;
        }
        exist_keys.push_back(a);
    }

    int insert_num = Config.item_num*Config.insert_factor;
    non_exist_keys.reserve(insert_num);
    for (size_t i = 0; i < insert_num; ++i) {
        key_type a = rand_lognormal(gen)*1000000000000;
        if(a<0) {
            i--;
            continue;
        }
        non_exist_keys.push_back(a);
    }
}
void skew_data(){
    std::cout<<"uniform_sparse_200M_uint64\n";
    std::string filename = "/home/student/2019/cs19resch11003/Learned_Index/SOSD-master/data/uniform_sparse_200M_uint64";
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
//    YCSBconfig.operate_queue = (operation_item *) malloc(YCSBconfig.operate_num * sizeof(operation_item));

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
    std::cerr << "  scan: " << scan_i << std::endl;
    std::cout<<"Total Item::  "<<item_num<<"   "<<YCSBconfig.operate_num<<"\n";
//    assert(item_num == YCSBconfig.operate_num);
}

// ===== read data ======
#define BUF_SIZE 2048

std::vector<int> read_data(const char *path) {
    std::vector<int> vec;
    FILE *fin = fopen(path, "rb");
    int buf[BUF_SIZE];
    while (true) {
        size_t num_read = fread(buf, sizeof(int), BUF_SIZE, fin);
        for (int i = 0; i < num_read; i++) {
            vec.push_back(buf[i]);
        }
        if (num_read < BUF_SIZE) break;
    }
    fclose(fin);
    return vec;
}

template<typename key_type>
std::vector<key_type> read_timestamp(const char *path) {
    std::vector<key_type> vec;
    FILE *fin = fopen(path, "rb");
    key_type buf[BUF_SIZE];
    while (true) {
        size_t num_read = fread(buf, sizeof(key_type), BUF_SIZE, fin);
        for (int i = 0; i < num_read; i++) {
            vec.push_back(buf[i]);
        }
        if (num_read < BUF_SIZE) break;
    }
    fclose(fin);
    return vec;
}

template<typename key_type>
std::vector<key_type> read_document(const char *path){
    std::vector<key_type> vec;
    std::ifstream fin(path);
    if(!fin){
        std::cout << "Erro message: " << strerror(errno) << std::endl;
        exit(0);
    }

    key_type a;
    char buffer[50];
    while(!fin.eof()){
        fin.getline(buffer, 50);
        if(strlen(buffer) == 0) break;
        sscanf(buffer, "%ld", &a);
        vec.push_back(a);
    }
    std::cout<< "read data number: " << vec.size() << std::endl;
    fin.close();
    return vec;
}