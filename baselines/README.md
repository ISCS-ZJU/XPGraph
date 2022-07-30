# Baseline
We take GraphOne, the state-of-the-art single-machine in-memory graph storage system, as a baseline for overall performance comparison. Here we provide two versions of GraphOne, i.e., (1) the original GraphOne (abbreviated as GraphOne-D) that stores all data on DRAM. (2) PMEM-based GraphOne (abbreviated as GraphOne-P) which stores the edge log and adjacency lists on PMEM by mmap-based implementation and keeps the meta-data and the intermediate data on DRAM. 

## Building
You can run following command to build GraphOne-D. Refer to https://github.com/ZoRax-A5/GraphOne/blob/master/README.md for detailed description. For the sake of comparision, we just modified program output based on the original GraphOne.
```bash
$ git clone https://github.com/ZoRax-A5/GraphOne.git
$ mkdir build
$ cd build
$ cmake ../
$ make
```

Before you build GraphOne-P, we will install libpmem on your machine. The way to build GraphOne-P is similar to build GraphOne-D. 
```bash
## TODO install libpmem
$ git clone https://github.com/ZoRax-A5/GraphOne.git -b PMGraphOne
$ mkdir build
$ cd build
$ cmake ../
$ make
```

### Running

To run test on the baseline, you can refer to https://github.com/ZoRax-A5/GraphOne/blob/master/README.md for related command-line options. Note that for GraphOne-P, We add two additional options to specific path for pmem. 
```bash
 -a: path for pmem0
 -b: path for pmem1
```

**Example1: Ingest all edges in Friendster dataset(directed, binary input) on GraphOne-D.**

```bash
$ cd baselines/GraphOne-D
$ ./build/graphone32 -i ./Dataset/Friendster/xx -v 68349467 -j 0 -s 1 -d 1 -t 16

```

**Example2: Ingest all edges in Friendster dataset(directed, binary input) on GraphOne-P, and then execute each implemanted graph algorithm one time.**

```bash
$ cd baselines/GraphOne-P
$ ./build/graphone32 -i ./Dataset/Friendster/xx -v 68349467 -j 0 -s 1 -d 1 -t 16 -a *path_to_pmem0*/XPGraphDB0/ -b *path_to_pmem1*/XPGraphDB1/
```