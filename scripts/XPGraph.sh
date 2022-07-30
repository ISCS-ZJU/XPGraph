#!/bin/bash

## Test graph ingest time of XPGraph, then conduct graph query of each algorithm one by one. 
## After that, test graph recovery time of XPGraph

pmem0=$1
pmem1=$2
dataset_path=$3
recovery_path=$4

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Twitter/bin -v 61588415 --vbuf_pool 8 --pblk_pools 32 -q 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size," >> xpgraph_update.csv
echo $cur_time "Graph query time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Twitter/bin -v 61588415 --vbuf_pool 8 --pblk_pools 32 -q 1" >> xpgraph_query.csv
echo "[QueryTimings]:time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)," >> xpgraph_query.csv
echo $cur_time "Graph recover time of XPGraph, with command: ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path} (Recover Twitter)" >> xpgraph_recover.csv
echo "[RecoverTimings]:recover_time(load+recover)," >> xpgraph_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Twitter/bin -v 61588415 --vbuf_pool 8 --pblk_pools 32 -q 1
    ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path}
done
echo >> xpgraph_update.csv
echo >> xpgraph_query.csv
echo >> xpgraph_recover.csv


cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Friendster/all/bin -v 68349467 --vbuf_pool 8 --pblk_pools 64 -q 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size," >> xpgraph_update.csv
echo $cur_time "Graph query time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Friendster/all/bin -v 68349467 --vbuf_pool 8 --pblk_pools 64 -q 1" >> xpgraph_query.csv
echo "[QueryTimings]:time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> xpgraph_query.csv
echo $cur_time "Graph recover time of XPGraph, with command: ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path} (Recover Friendster)" >> xpgraph_recover.csv
echo "[RecoverTimings]:recover_time(load+recover)," >> xpgraph_recover.csv

for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Friendster/all/bin -v 68349467 --vbuf_pool 8 --pblk_pools 64 -q 1
    ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path}
done
echo >> xpgraph_update.csv
echo >> xpgraph_query.csv
echo >> xpgraph_recover.csv



cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Ukdomain/shuffle -v 105153953 --vbuf_pool 8 --pblk_pools 64 -q 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size" >> xpgraph_update.csv
echo $cur_time "Graph query time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Ukdomain/shuffle -v 105153953 --vbuf_pool 8 --pblk_pools 64 -q 1" >> xpgraph_query.csv
echo "[QueryTimings]:time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> xpgraph_query.csv
echo $cur_time "Graph recover time of XPGraph, with command: ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path} (Recover UKDomain)" >> xpgraph_recover.csv
echo "[RecoverTimings]:recover_time(load+recover)," >> xpgraph_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Ukdomain/shuffle -v 105153953 --vbuf_pool 8 --pblk_pools 64 -q 1
    ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path}
done
echo >> xpgraph_update.csv
echo >> xpgraph_query.csv
echo >> xpgraph_recover.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Kron28/bin -v 268435456 --vbuf_pool 16 --pblk_pools 64 -q 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size," >> xpgraph_update.csv
echo $cur_time "Graph query time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Kron28/bin -v 268435456 --vbuf_pool 16 --pblk_pools 64 -q 1" >> xpgraph_query.csv
echo "[QueryTimings]:time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> xpgraph_query.csv
echo $cur_time "Graph recover time of XPGraph, with command: ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path} (Recover Kron28)" >> xpgraph_recover.csv
echo "[RecoverTimings]:recover_time(load+recover)," >> xpgraph_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Kron28/bin -v 268435456 --vbuf_pool 16 --pblk_pools 64 -q 1
    ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path}
done
echo >> xpgraph_update.csv
echo >> xpgraph_query.csv
echo >> xpgraph_recover.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Yahoo/bin32 -v 1413511394 --vbuf_pool 16 --pblk_pools 192 -q 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size," >> xpgraph_update.csv
echo $cur_time "Graph query time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Yahoo/bin32 -v 1413511394 --vbuf_pool 16 --pblk_pools 192 -q 1" >> xpgraph_query.csv
echo "[QueryTimings]:time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> xpgraph_query.csv
echo $cur_time "Graph recover time of XPGraph, with command: ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path} (Recover Yahoo)" >> xpgraph_recover.csv
echo "[RecoverTimings]:recover_time(load+recover)," >> xpgraph_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Yahoo/bin32 -v 1413511394 --vbuf_pool 16 --pblk_pools 192 -q 1
    ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path}
done
echo >> xpgraph_update.csv
echo >> xpgraph_query.csv
echo >> xpgraph_recover.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Kron29/bin -v 536870912 --vbuf_pool 32 --pblk_pools 128 -q 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size," >> xpgraph_update.csv
echo $cur_time "Graph query time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Kron29/bin -v 536870912 --vbuf_pool 32 --pblk_pools 128 -q 1" >> xpgraph_query.csv
echo "[QueryTimings]:time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> xpgraph_query.csv
echo $cur_time "Graph recover time of XPGraph, with command: ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path} (Recover Kron29)" >> xpgraph_recover.csv
echo "[RecoverTimings]:recover_time(load+recover)," >> xpgraph_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Kron29/bin -v 536870912 --vbuf_pool 32 --pblk_pools 128 -q 1
    ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path}
done
echo >> xpgraph_update.csv
echo >> xpgraph_query.csv
echo >> xpgraph_recover.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Kron30/bin -v 1073741824 --vbuf_pool 64 --pblk_pools 192 -q 1" >> xpgraph_update.csv
echo "[UpdateTimings]: ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size" >> xpgraph_update.csv
echo $cur_time "Graph query time of XPGraph, with command: ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Kron30/bin -v 1073741824 --vbuf_pool 64 --pblk_pools 192 -q 1" >> xpgraph_query.csv
echo "[QueryTimings]:time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)" >> xpgraph_query.csv
echo $cur_time "Graph recover time of XPGraph, with command: ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path} (Recover Kron30)" >> xpgraph_recover.csv
echo "[RecoverTimings]:recover_time(load+recover)," >> xpgraph_recover.csv
for(( times = 0; times < 10; times++))
do
    echo "times = " $times " from echo"
    sleep 10s
    ./bin/main -j 0 -p0 $pmem0 -p1 $pmem1 -f ${dataset_path}Kron30/bin -v 1073741824 --vbuf_pool 64 --pblk_pools 192 -q 1
    ./bin/main -j 3 -p0 $pmem0 -p1 $pmem1 --recovery ${recovery_path}
done
echo >> xpgraph_update.csv
echo >> xpgraph_query.csv
echo >> xpgraph_recover.csv

cur_time=$(date "+%Y-%m-%d %H:%M:%S")
echo $cur_time "Graph ingest time of XPGraph end." >> xpgraph_update.csv
echo $cur_time "Graph query time of XPGraph end." >> xpgraph_query.csv
echo $cur_time "Graph recovery time of XPGraph end." >> xpgraph_recover.csv

mv xpgraph_update.csv results/fig11/XPGraph.csv
mv xpgraph_query.csv results/fig14/XPGraph.csv
mv xpgraph_recover.csv results/fig15/XPGraph.csv
