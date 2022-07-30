#!/bin/bash

## Test graph ingest time of GraphOne-P, then conduct graph query of each algorithm one by one. 

gop_path=$1
pmem0=$2
pmem1=$3
dataset_path=$4

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Twitter/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> gop_update.csv
echo $cur_time "Graph query time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Twitter/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_query.csv
echo "time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> gop_query.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Twitter/bin -v 61578415 -j 0 -s 1 -d 1 -t 16
done
echo >> gop_update.csv
echo >> gop_query.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Friendster/all/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> gop_update.csv
echo $cur_time "Graph query time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Friendster/all/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_query.csv
echo "time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> gop_query.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Friendster/all/bin -v 68349467 -j 0 -s 1 -d 1 -t 16
done
echo >> gop_update.csv
echo >> gop_query.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Ukdomain/shuffle -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> gop_update.csv
echo $cur_time "Graph query time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Ukdomain/shuffle -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_query.csv
echo "time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> gop_query.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Ukdomain/shuffle -v 105153953 -j 0 -s 1 -d 1 -t 16
done
echo >> gop_update.csv
echo >> gop_query.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Kron28/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> gop_update.csv
echo $cur_time "Graph query time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Kron28/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_query.csv
echo "time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> gop_query.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Kron28/bin -v 268435456 -j 0 -s 1 -d 1 -t 16
done
echo >> gop_update.csv
echo >> gop_query.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Kron29/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> gop_update.csv
echo $cur_time "Graph query time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Kron29/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_query.csv
echo "time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> gop_query.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Kron29/bin -v 536870912 -j 0 -s 1 -d 1 -t 16
done
echo >> gop_update.csv
echo >> gop_query.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Kron30/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> gop_update.csv
echo $cur_time "Graph query time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Kron30/bin -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_query.csv
echo "time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> gop_query.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Kron30/bin -v 1073741823 -j 0 -s 1 -d 1 -t 16
done
echo >> gop_update.csv
echo >> gop_query.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Yahoo/bin64 -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_update.csv
echo "make_graph_time(s),VIRT(GB),RSS(GB),vunit_size(GB),snap_size(GB),global_range_size(GB),global_range_in_size(GB),elog_size(GB),adjlist_size(GB),local_buf_size(GB)" >> gop_update.csv
echo $cur_time "Graph query time of GraphOne-P, with command: ${gop_path}build/graphone32 -a ${pmem0} -b ${pmem1}  -i ${dataset_path}Yahoo/bin64 -v 61578415 -j 0 -s 1 -d 1 -t 16" >> gop_query.csv
echo "time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> gop_query.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${gop_path}build/graphone64 -a ${pmem0} -b ${pmem1} -i ${dataset_path}Yahoo/bin64 -v 1413511394 -j 0 -s 1 -d 1 -t 16
done
echo >> gop_update.csv
echo >> gop_query.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of GraphOne-P end." >> gop_update.csv
echo $cur_time "Graph query time of GraphOne-P end." >> gop_query.csv

mv gop_update.csv results/fig11/GraphOne-P.csv
mv gop_query.csv results/fig14/GraphOne-P.csv
