#!/bin/bash

#
# !! Configure Optane to App-Direct Mode !! 
# !! Refer to scripts/nvdimm.md !!
#

# !! Install libpmem library !!

# Make XPGraph libary and executable main
# see README.md
make
export LD_LIBRARY_PATH=./src/api/lib/:$LD_LIBRARY_PATH
echo "LD_LIBRARY_PATH = " $LD_LIBRARY_PATH

# Make GraphOne-D and GraphOne-P
# see baselines/README.md

# config the path below according to your host
dataset_path="/mnt/nvme1/wr/dataset/"   # path to the dataset directory
pmem0="/pmem/wr/GraphDB0/"            # path to the graph database information stored in PMEM0
pmem1="/mnt/pmem1/wr/GraphDB1/"         # path to the graph database information stored in PMEM1
recovery_path="/mnt/pmem1/wr/Recovery/" # path to save recovery info for XPGraph
god_path="./baselines/GraphOne-D/"       # path to the baseline GraphOne-D
gop_path="./baselines/GraphOne-P/"       # path to the baseline GraphOne-P 

mkdir ${pmem0}
mkdir ${pmem1}

mkdir results
mkdir results/fig11
mkdir results/fig12
mkdir results/fig14
mkdir results/fig15

# Figure 11 + Figure 14
cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Test GraphOne-P performance, including Figure 11 (Graph ingest time cost for non-volatile systems), Figure 14 (Graph query performance)..." >> scripts/progress.txt
echo "[ Expected completion time: around 17.2 hours ]" >> scripts/progress.txt
bash scripts/GraphOne-P.sh ${gop_path} ${pmem0} ${pmem1} ${dataset_path}
sleep 10s

# Figure 11 + Figure 14 + Figure 15
cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Test XPGraph performance, including Figure 11 (Graph ingest time cost for non-volatile systems), Figure 14 (Graph query performance), as well as Figure 15 (Graph recovery performance)..." >> scripts/progress.txt
echo "[ Expected completion time: around 12.9 hours ]" >> scripts/progress.txt
bash scripts/XPGraph.sh ${pmem0} ${pmem1} ${dataset_path} ${recovery_path}
sleep 10s

# Figure 11
cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Test XPGraph-B performance, including Figure 11 (Graph ingest time cost for non-volatile systems)..." >> scripts/progress.txt
echo "[ Expected completion time: around 5.5 hours ]" >> scripts/progress.txt
bash scripts/XPGraph-B.sh ${pmem0} ${pmem1} ${dataset_path}
sleep 10s

# Figure 15
cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Test GraphOne-D performance, including Figure 15 (Graph recovery performance)..." >> scripts/progress.txt
echo "[ Expected completion time: around 1.6 hours ]" >> scripts/progress.txt
bash scripts/GraphOne-D-RE.sh ${god_path} ${dataset_path}
sleep 10s

# Figure 12
cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Test GraphOne-D performance in DRAM-Only system, including Figure 12 (Graph ingest time cost for volatile systems)..." >> scripts/progress.txt
echo "[ Expected completion time: around 1.2 hours ]" >> scripts/progress.txt
bash scripts/GraphOne-D-DO.sh ${god_path} ${dataset_path}
sleep 10s

# Figure 12
cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Test XPGraph-D performance in DRAM-Only system, including Figure 12 (Graph ingest time cost for volatile systems)..." >> scripts/progress.txt
echo "[ Expected completion time: around 0.9 hours ]" >> scripts/progress.txt
bash scripts/XPGraph-D-DO.sh ${dataset_path}

# 
# !! Switch Optane to Memory Mode !! 
# !! Refer to scripts/nvdimm.md !!
# 

# Figure 12
cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Test GraphOne-D performance under Memory Mode of Optane, including Figure 12 (Graph ingest time cost for volatile systems)..." >> scripts/progress.txt
echo "[ Expected completion time: around 6.6 hours ]" >> scripts/progress.txt
bash scripts/GraphOne-D-MM.sh ${god_path} ${dataset_path}
sleep 10s

# Figure 12
cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Test XPGraph-D performance under Memory Mode of Optane, including Figure 12 (Graph ingest time cost for volatile systems)..." >> scripts/progress.txt
echo "[ Expected completion time: around 4.0 hours ]" >> scripts/progress.txt
bash scripts/XPGraph-D-MM.sh ${dataset_path}
