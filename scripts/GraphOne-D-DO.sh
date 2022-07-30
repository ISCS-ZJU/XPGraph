#!/bin/bash

## Graph ingest time of GraphOne-D in DRAM-only system

# god_path="../GraphOne/"
god_path=$1
dataset_path=$2

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-D, with command: ${god_path}build/graphone32 -i ${dataset_path}Twitter/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> god_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> god_update.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${god_path}build/graphone32 -i ${dataset_path}Twitter/bin -v 61578415 -j 0 -s 1 -d 1 -t 16
done
echo >> god_update.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-D, with command: ${god_path}build/graphone32 -i ${dataset_path}Friendster/all/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> god_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> god_update.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${god_path}build/graphone32 -i ${dataset_path}Friendster/all/bin -v 68349467 -j 0 -s 1 -d 1 -t 16
done
echo >> god_update.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-D, with command: ${god_path}build/graphone32 -i ${dataset_path}Ukdomain/shuffle -v 61578415 -j 0 -s 1 -d 1 -t 16" >> god_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> god_update.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${god_path}build/graphone32 -i ${dataset_path}Ukdomain/shuffle -v 105153953 -j 0 -s 1 -d 1 -t 16
done
echo >> god_update.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-D, with command: ${god_path}build/graphone32 -i ${dataset_path}Kron28/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> god_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> god_update.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${god_path}build/graphone32 -i ${dataset_path}Kron28/bin -v 268435456 -j 0 -s 1 -d 1 -t 16
done
echo >> god_update.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-D (DRAM Only), end." >> god_update.csv

mv god_update.csv results/fig12/GraphOne-D-DO.csv