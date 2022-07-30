#pragma once

#include <omp.h>
#include <getopt.h>
#include <stdlib.h>
#include <string>
#include <algorithm>

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

class edge_t;
class buffer_t;
class graph_t;
class edgelog_t;
class levelgraph_t;
class metrics;

/* ---------------------------------------------------------------------- */
class XPGraph{
private:
    levelgraph_t *levelgraph;

    void config_graph(int argc, const char ** argv);
    void prinf_graph_config();
    void print_time_cost(double time);
    void print_mem_usage();

public:
    // Graph Initialize
    XPGraph(int argc, const char ** argv);
    ~XPGraph();
    // index_t set_elog(char* buf, size_t size, index_t count);
    // size_t load_edgelist(char** buf, std::string idirname);

    // Graph Update
    bool add_edge(vid_t src, vid_t dst);
    index_t add_edges(char* buf, size_t size, index_t count = 0);
    index_t buffer_edges(char* buf, size_t size, index_t count = 0);
    bool del_edge(vid_t src, vid_t dst);
    void ingest_edges_from_files(std::string idirname, index_t count = 0);
    void log_edges_from_files(std::string idirname, index_t count = 0);
    void archive_edges_from_files(std::string idirname, int residue, index_t count = 0);
    void import_graph_by_config();
    // index_t log_edges(char* buf, size_t size, index_t count = 0);
    // void archive_edges(int residue, index_t count);
    void archive_all_edges();

    // Graph Save/Load
    void save_gragh();
    void load_graph(std::string idirname);

    // Graph Query
    vid_t get_vcount();
    edgelog_t* get_elog();
    edge_t* get_loggeg_edges(index_t &count);
    graph_t* get_out_graph();
    graph_t* get_in_graph();
    degree_t get_out_degree(vid_t vid);
    degree_t get_in_degree(vid_t vid);
    void print_out_nebrs(vid_t vid);
    void print_in_nebrs(vid_t vid);
    degree_t get_out_nebrs(vid_t vid, vid_t* local_adjlist);
    degree_t get_in_nebrs(vid_t vid, vid_t* local_adjlist);
    degree_t get_out_nebrs_logged(vid_t vid, vid_t* local_adjlist);
    degree_t get_in_nebrs_logged(vid_t vid, vid_t* local_adjlist);
    degree_t get_out_nebrs_buffered(vid_t vid, vid_t* local_adjlist);
    degree_t get_in_nebrs_buffered(vid_t vid, vid_t* local_adjlist);
    degree_t get_out_nebrs_flushed(vid_t vid, vid_t* local_adjlist);
    degree_t get_in_nebrs_flushed(vid_t vid, vid_t* local_adjlist);

    // Graph Arrange
    index_t buffer_all_edges();
    index_t flush_all_vbufs();
    index_t archive_all_vbufs(); // buffer, then flush
    bool compact_out_adjs(vid_t vid);
    bool compact_in_adjs(vid_t vid);
    vid_t compact_all_adjs();

    // Others
    uint8_t get_socket_num();
    uint8_t get_socket_id(vid_t vid);
    void bind_cpu(tid_t tid, uint8_t socket_id);
    void cancel_bind_cpu();
    uint8_t get_query_count();
};
