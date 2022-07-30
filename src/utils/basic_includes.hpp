#pragma once

#include <omp.h>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <numa.h>
#include <vector>

#include "utils/file.hpp"
#include "utils/cmdopts.hpp"
#include "utils/logstream.hpp"
#include "config/args.hpp"

using std::cout;
using std::endl;
using std::string;

/* ---------------------------------------------------------------------- */
#define KB      (1024)
#define MB      (1024*KB)
#define GB      (1024*MB)
#define PAGE_SIZE          (4*KB)

inline size_t ALIGNMENT(size_t x, size_t align){
    return ceil(1.0 * x / align) * align;
}

inline double B2GB(size_t x){
    return (double)(x * 1.0 / GB);
}

/* ---------------------------------------------------------------------- */
#ifdef B64
typedef uint64_t vid_t; //vertex id
typedef uint32_t blk_deg_t; //vertex degree in a block
#else
typedef uint32_t vid_t; //vertex id
typedef uint16_t blk_deg_t; //vertex degree in a block
#endif
typedef uint64_t index_t;
typedef int64_t offset_t;
typedef uint32_t degree_t; //vertex degree
typedef uint32_t sid_t; //snap id
typedef uint16_t rid_t; //range id
typedef uint8_t tid_t; //thread id
typedef uint8_t sktid_t; //socket id

// #define VBIT 32 // bits of size of vertex id
// #define VMASK 0xFFFFFFFF
// #define GET_SRC(edge)  (edge & VMASK)
// #define GET_DST(edge)  (edge >> VBIT) 
// #define GET_SRC(edge)  (edge >> VBIT)
// #define GET_DST(edge)  (edge & VMASK) 

/* ---------------------------------------------------------------------- */
// transform between 'level' and 'size'
inline index_t GET_SIZE_LEVEL(size_t size){
    // index_t level = log(size >> 4)/log(2);
    // index_t level = log2(size >> MIN_VBUF_BIT);
    index_t level = ceil(log2(size >> MIN_VBUF_BIT));
    if(level >= LEVEL_COUNT) std::cout << size << " " << (uint32_t)level << std::endl;
    assert(level < LEVEL_COUNT);
    return level;
}

inline size_t GET_LEVEL_SIZE(index_t level){
    // size_t size = MIN_VBUF_SIZE * pow(2,level);
    // size_t size = MIN_VBUF_SIZE * (1L << level);
    size_t size = 1L << (MIN_VBUF_BIT + level);
    if(size < MIN_VBUF_SIZE || size > MAX_VBUF_SIZE)logstream(LOG_FATAL) << size << " " << (uint32_t)level << endl;
    assert(size >= MIN_VBUF_SIZE && size <= MAX_VBUF_SIZE);
    return size;
}

/* ---------------------------------------------------------------------- */
// used for memory usage
size_t vbulk_size = 0;
size_t snap_bulk_size = 0;
size_t temp_ranged_edgelist_size = 0;


index_t  SNAP_COUNT  = (3);

// used for storing temp edges

/* ---------------------------------------------------------------------- */
// Used for recover graph
int64_t FIRST_BLOCK_MARK = 0;

/* ---------------------------------------------------------------------- */
#include "utils/metrics/metrics.hpp"
#include "utils/metrics/reps/basic_reporter.hpp"
#include "utils/metrics/reps/file_reporter.hpp"
#include "utils/metrics/reps/html_reporter.hpp"       
/**
 * Helper for metrics.
 */
static VARIABLE_IS_NOT_USED void metrics_report(metrics &m);
static VARIABLE_IS_NOT_USED void metrics_report(metrics &m) {
    // std::string reporters = get_option_string("metrics.reporter", "console, file, html");
    std::string reporters = "console,file";
    char * creps = (char*)reporters.c_str();
    const char * delims = ",";
    char * t = strtok(creps, delims);

    while(t != NULL) {            
        std::string repname(t);
        if (repname == "basic" || repname == "console") {
            basic_reporter rep;
            m.report(rep);
        } else if (repname == "file") {
            file_reporter rep(get_option_string("metrics.reporter.filename", "xpgraph_metrics.txt"));
            m.report(rep);
        } else if (repname == "html") {
            html_reporter rep(get_option_string("metrics.reporter.htmlfile", "xpgraph_metrics.html"));
            m.report(rep);
        } else {
            logstream(LOG_WARNING) << "Could not find metrics reporter with name [" << repname << "], ignoring." << std::endl;
        }
        t = strtok(NULL, delims);
    }  
}

#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
#include <thread>

inline void bind_thread_to_cpu(int cpu_id){
    cpu_set_t cpu_mask;
    CPU_ZERO(&cpu_mask);
    CPU_SET(cpu_id, &cpu_mask);
    // sched_setaffinity(gettid(), sizeof(cpu_mask), &cpu_mask);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_mask), &cpu_mask);
}

inline void cancel_thread_bind(){
    uint8_t cpu_num = std::thread::hardware_concurrency();
    // std::cout << "In cancel_thread_bind(), cpu_num = " << (uint32_t)cpu_num << std::endl; // 96
    cpu_set_t cpu_mask;
    CPU_ZERO(&cpu_mask);
    for(uint8_t cpu_id = 0; cpu_id < cpu_num; cpu_id++)
        CPU_SET(cpu_id, &cpu_mask);
    // sched_setaffinity(gettid(), sizeof(cpu_mask), &cpu_mask);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_mask), &cpu_mask);
}

inline void bind_thread_to_socket(tid_t tid, int socket_id){
    assert(NUM_SOCKETS == 2);
    if(socket_id == 0){
        if(tid >= 0 && tid < 24) bind_thread_to_cpu(tid);
        else if(tid < 48) bind_thread_to_cpu(tid + 24);
        // else cancel_thread_bind(); // only 48 cores available, other threads need to access PM across NUMA node
    } else if(socket_id == 1){
        if(tid >= 0 && tid < 24) bind_thread_to_cpu(tid + 72);
        else if(tid < 48) bind_thread_to_cpu(tid);
        else cancel_thread_bind(); // only 48 cores available, other threads need to access PM across NUMA node
    } else {
        std::cout << "Wrong socket id: " << socket_id << std::endl;
        assert(0);
    }
}

inline void bind_thread_to_socket0(tid_t tid, int socket_id){
    assert(NUM_SOCKETS == 2);
    if(socket_id == 0){
        if(tid < 24 || (tid >= 48 && tid < 72)) bind_thread_to_cpu(tid);
        else bind_thread_to_cpu(tid-24);
    } else if(socket_id == 1){
        if(tid < 24 || (tid >= 48 && tid < 72)) bind_thread_to_cpu(tid+24);
        else bind_thread_to_cpu(tid);
    } else {
        std::cout << "Wrong socket id: " << socket_id << std::endl;
        assert(0);
    }
}

inline uint8_t GET_SOCKETID(vid_t vid){
    // assert(NUM_SOCKETS == 2);
    return vid & 1;
    // return vid % NUM_SOCKETS;
}

//////////////////////////////////////////////////////////////////////////////
//
// process_mem_usage(size_t &, size_t &) - takes two doubles by reference,
// attempts to read the system-dependent data for a process' virtual memory
// size and resident set size, and return the results in byte.
// On failure, returns 0.0, 0.0
void process_mem_usage(pid_t proc_id, size_t& vm_usage, size_t& resident_set)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0;
   resident_set = 0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/"+std::to_string(proc_id)+"/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size = sysconf(_SC_PAGE_SIZE); // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize;
   resident_set = rss * page_size;
}
