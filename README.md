# XPGraph

This respository is for following paper:

**MICRO'22 paper:** XPGraph: XPline-Friendly Persistent Memory Graph Stores for Large-Scale Evolving Graphs

## Abstract

Traditional in-memory graph storage systems have limited
scalability due to the limited capacity and volatility of DRAM.
Emerging persistent memory (PMEM), with large capacity
and non-volatility, provides us an opportunity to realize the
scalable and high-performance graph stores. However, directly moving existing DRAM-based graph storage systems
to PMEM would cause serious PMEM access inefficiency
issues, including high read and write amplification in PMEM
and costly remote PMEM accesses across NUMA nodes,
thus leading to the performance bottleneck. 
<!-- In this paper, -->
We propose XPGraph, a PMEM-based graph storage system for managing large-scale evolving graphs, by developing
an XPLine-friendly graph access model with vertex-centric
graph buffering, hierarchical vertex buffer managing, and
NUMA-friendly graph accessing. Experimental results show
that XPGraph achieves 3.01× to 3.95× higher update performance, 
as well as up to 4.46× higher query performance 
compared with the state-of-the-art in-memory graph storage system
implemented on a PMEM-based system.

## Try out XPGraph

### Hardware requirements
* Intel Optane Persistent Memory 200 Series.
* Two processors configured in a non-uniform memory access (NUMA) architecture.

### Compiling 

Using the following commands, one can easily compile the `XPGraph`. 
The generated libary file is located at `src/api/lib/libxpgraph.so`, 
and the compiled executable file is located at `bin/main`. 

```bash
## Get source code
$ git clone 
$ cd XPGraph

## Make libary file
$ make lib

## Set head file and LD_LIBRARY_PATH
$ sudo cp ./src/api/libxpgraph.h /usr/include/
$ export LD_LIBRARY_PATH=./src/api/lib/:$LD_LIBRARY_PATH

## Make executable file
$ make main

# ## Make all (libary and executable file)
# $ make
```

### Dataset preparing

XPGraph provides interface with edge list data in binary format from a file. For convenience, we provide a script to convert input data from text format to binary format.

```bash
## Change to binary format
$ cd preprocess
$ g++ text2bin.cpp -o text2bin
$ ./text2bin [path_to_txt/data.txt] [path_to_binary/data.bin]
```

**Input Graph Data:** Edge list in binary format. 

**Example1: LiveJournal (small graph for function test):**
```bash
## Download and unzip
$ mkdir Dataset && cd Dataset
$ mkdir LiveJournal && cd LiveJournal
$ mkdir txt && cd txt
$ wget https://snap.stanford.edu/data/soc-LiveJournal1.txt.gz
$ gunzip soc-LiveJournal1.txt.gz

$ cd .. && mkdir bin
$ *preprocess_path*/text2bin txt/soc-LiveJournal1.txt bin/out.bin
```

**Example2: Friendster (Medium graph, one of our evaluation tested dataset):**
```bash
## Download and unzip
$ mkdir Dataset && cd Dataset
$ mkdir Friendster && cd Friendster
$ mkdir txt && cd txt
$ wget http://konect.cc/files/download.tsv.friendster.tar.bz2 
$ tar -jxvf friendster.tar.bz2

## Change to binary format
$ cd .. && mkdir bin
$ *preprocess_path*/text2bin txt/out.friendster bin/out.bin
```

### Running

**Arguments setup:**
You can set up the arguments in the `src/config/args_config.hpp` file, 
or use following command-line options.

```bash
./bin/main
./exe options.
 -h: This message.
 -f: Input dataset file.
 -v: Vertex count.
 -e: Edge count need for import. Default: 0 for importing all edges in the dataset file.
 -j: Job id (0 for ingest, 1 for log, 2 for archive, 3 for recover). Default: 0.
 -r: Archive threshold for job2.
 -q: The number of executions for each graph query algorithm.
 -t: Thread count for buffering and flushing. Default: 16.
 -s: Sockets count. Default: 2.
 -p0: Path of pmem0 of NUMA node0.
 -p1: Path of pmem1 of NUMA node1.
 -numa: Implementation of numa optimization: 0 for closing NUMA optimization, 1 for out/in-graph based implementation, 2 for sub-graph based implementation. Default: 2.
 --elogsize: Bits of maximum number of edges in edge log. Default: 30 for 1 billion edges, i.e., edge size equals 8GB.
 --leveled_buf: 0 for fixed vertex buffer size setting, 1 for hierarchical vertex buffer size setting. Default: 1.
 --minvbuf: Minimum per-vertex buffer size in bytes. Default: 16.
 --maxvbuf: Maximum per-vertex buffer size in bytes, or fixed per-vertex buffer size when leveled_buf = 0. Default: 256.
 --mempool: 0 for malloc/free-based vertex buffer managing, 1 for memory pool based vertex buffer managing. Default: 1.
 --membulk: Memory bulk size in MegaBytes. Default: 16.
 --vbuf_pool: Total vertex buffer pool size threshold in GigaByte. Default: 16.
 --pblk_pools: Total persistent PMEM block pool size threshold in GigaByte. Default: 64.
 --battery: 1 for XPGraph-B by allowing overwritting buffered edges. Default: 0.
 --dram_only: 1 for XPGraph-D by storing all data structure in DRAM and set the per-vertex buffer size as fixed 64B. Default: 0.
 --persist: 1 for saving XPGraph information, used for recovery. Default: 1.
 --recovery: Path of recovery data.
```
**Example1: Ingest all edges in Friendster dataset, and then execute each implemanted graph algorithm one time.**

```bash
$ cd XPGraph
$ ./bin/main -f ./Dataset/Friendster/xx -v 68349467 -j 0 -q 1 -p0 *path_to_pmem0*/XPGraphDB/ -p1 *path_to_pmem1*/XPGraphDB/ --vbuf_pool 16 pblk_pools 64
```

The key statistic data would be recorded to `xpgraph_update.csv` file, e.g.
```bash
# [UpdateTimings]:ingest_time(s),archive_count,archive_time(classify+buffer),flush_all_time(s),make_graph_time(s),[Memory]: vbuf_pool_size,pblk_pool_size,[QueryTimings]:time_1hop(s),time_2hop(s),time_bfs(s),time_pagerank(s)
[UpdateTimings]:34.5745,67,31.4394(7.71303+23.726),3.51127,34.9569,[Memory]:5.17188,13.6719(7.32812+6.34375),[QueryTimings]:2.31569,12.6733,2.35859,3.31254,
```

**Example2: Recover graph based on XPGraph data stored on pmem of last graph ingestion.**

```bash
$ cd XPGraph
$ ./bin/main -j 3 -p0 *path_to_pmem0*/XPGraphDB/ -p1 *path_to_pmem1*/XPGraphDB/ --recovery *path_to_recovery*
```

The key statistic data would be recorded to `xpgraph_update.csv` file, e.g.
```bash
# [RecoverTimings]:recover_time(load+recover),
[RecoverTimings]:6.93418(3.18781+3.74637),
```