sudo apt -y update
sudo apt -y install zstd python3-pip m4 cmake clang libboost-all-dev
pip3 install --user numpy scipy

## Preparing the Dataset
'./downlaod.sh' downloads and stores required data from the Internet

## Build and run
Run cmake -S ./
Run make

## This will create different executable file for each Datasstructure such as:

Kanva_benchmark, Kanva_ycsb, Finedex_benchmark, Finedex_YCSB, ABTree_Benchmark, ABTree_YCSB, CIST_benchmark, CIST_YCSB, ElimABtree_benchmark, ElimABtree_YCSB

## For example to run Kanva Benchmark

./Kanva_benchmark <Parameters>

##There are several parameters you can pass, such as benchmark, Number of threads, runtime, read, insert and delete ratio.
##example:
--thread_num 64 --benchmark 5 --read 50 --insert 30 --remove 20 --runtime 10

##Different Benchmarks::
0: Amazon Dataset
1: Facebook Dataset
2: Log-Normal
3: Normal
4: OSM 
5: Uniform
6: Wikipedia

## For YCSB workload one has to generate the YCSB workload using this https://github.com/brianfrankcooper/YCSB
## Path for the workload needs to be mocdified in the common/functions.h for benchmark 7,8,9,10,11,12.

