#!/bin/bash

## Graph recovery time of GraphOne-D in DRAM-only system

# god_path="../GraphOne/"
god_path=$1
dataset_path=$2

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph recovery time of GraphOne-D, with command: ${god_path}build/graphone32 -i ${dataset_path}Twitter/bin -v 61578415 -j 2 -s 1 -d 1 -t 16 -r 27" >> god_recover.csv
echo "graph_recovery_time(s)" >> god_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${god_path}build/graphone32 -i ${dataset_path}Twitter/bin -v 61578415 -j 2 -s 1 -d 1 -t 16 -r 27
done
echo >> god_recover.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph recovery time of GraphOne-D, with command: ${god_path}build/graphone32 -i ${dataset_path}Friendster/all/bin -v 61578415 -j 2 -s 1 -d 1 -t 16 -r 27" >> god_recover.csv
echo "graph_recovery_time(s)" >> god_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${god_path}build/graphone32 -i ${dataset_path}Friendster/all/bin -v 68349467 -j 2 -s 1 -d 1 -t 16 -r 27
done
echo >> god_recover.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph recovery time of GraphOne-D, with command: ${god_path}build/graphone32 -i ${dataset_path}Ukdomain/shuffle -v 61578415 -j 2 -s 1 -d 1 -t 16 -r 27" >> god_recover.csv
echo "graph_recovery_time(s)" >> god_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${god_path}build/graphone32 -i ${dataset_path}Ukdomain/shuffle -v 105153953 -j 2 -s 1 -d 1 -t 16 -r 27
done
echo >> god_recover.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph recovery time of GraphOne-D, with command: ${god_path}build/graphone32 -i ${dataset_path}Kron28/bin -v 61578415 -j 2 -s 1 -d 1 -t 16 -r 27" >> god_recover.csv
echo "graph_recovery_time(s)" >> god_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ${god_path}build/graphone32 -i ${dataset_path}Kron28/bin -v 268435456 -j 2 -s 1 -d 1 -t 16 -r 27
done
echo >> god_recover.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph recovery time of GraphOne-D end." >> god_recover.csv

mv god_recover.csv results/fig15/GraphOne-D-RE.csv