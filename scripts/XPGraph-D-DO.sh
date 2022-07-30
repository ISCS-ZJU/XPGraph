#!/bin/bash

## Graph ingest time of XPGraph-D in DRAM-only system

dataset_path=$1

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph-D, with command: ./bin/main -j 0 -f ${dataset_path}/Twitter/bin -v 61588415 --vbuf_pool 8 --pblk_pools 32 --dram_only 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size," >> xpgraph_update.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -f ${dataset_path}/Twitter/bin -v 61588415 --vbuf_pool 8 --pblk_pools 32 --dram_only 1
done
echo >> xpgraph_update.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph-D, with command: ./bin/main -j 0 -f ${dataset_path}/Friendster/all/bin -v 68349467 --vbuf_pool 8 --pblk_pools 64 --dram_only 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size," >> xpgraph_update.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -f ${dataset_path}/Friendster/all/bin -v 68349467 --vbuf_pool 8 --pblk_pools 64 --dram_only 1
done
echo >> xpgraph_update.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph-D, with command: ./bin/main -j 0 -f ${dataset_path}/Ukdomain/shuffle -v 105153953 --vbuf_pool 8 --pblk_pools 64 --dram_only 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size," >> xpgraph_update.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -f ${dataset_path}/Ukdomain/shuffle -v 105153953 --vbuf_pool 8 --pblk_pools 64 --dram_only 1
done
echo >> xpgraph_update.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph-D, with command: ./bin/main -j 0 -f ${dataset_path}/Kron28/bin -v 268435456 --vbuf_pool 16 --pblk_pools 64 --dram_only 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size," >> xpgraph_update.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -f ${dataset_path}/Kron28/bin -v 268435456 --vbuf_pool 16 --pblk_pools 64 --dram_only 1
done
echo >> xpgraph_update.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph-D (DRAM Only), end." >> xpgraph_update.csv

mv xpgraph_update.csv results/fig12/XPGraph-D-DO.csv