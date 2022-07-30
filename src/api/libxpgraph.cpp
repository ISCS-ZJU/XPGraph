#include "libxpgraph.h"
#include "graph/levelgraph.hpp"
#include "graph/importgraph.hpp"
#include "graph/blocks.hpp"
#include "config/args_config.hpp"

metrics m("XPGraph");

/* -------------------------------------------------------------- */
// Graph Configure
XPGraph::XPGraph(int argc, const char ** argv){
    m.start_time("XPGraph");

    m.start_time("1-init_alloc_XPGraph");
    config_graph(argc, argv);
    prinf_config();

    if(numa_available() < 0) NUM_SOCKETS = 1;
    assert(NUM_SOCKETS);
    mkdir(NVMPATH0.c_str(), 0777);
    if(NUM_SOCKETS > 1) mkdir(NVMPATH1.c_str(), 0777);
    vbulk_size = 0;
    snap_bulk_size = 0;
    temp_ranged_edgelist_size = 0;

    levelgraph = new levelgraph_t(nverts, m);
    levelgraph->init_elog(); 
    levelgraph->init_graph();
    m.stop_time("1-init_alloc_XPGraph");
}

XPGraph::~XPGraph(){
    levelgraph->free_graph();
    levelgraph->free_elog();
    delete levelgraph;
    
    if (!PERSIST) {
        rm_dir(NVMPATH0.c_str());
        rm_dir(NVMPATH1.c_str());
    }
    m.stop_time("XPGraph");
    metrics_report(m);
}

void XPGraph::config_graph(int argc, const char ** argv) {
    args_config(argc, argv);
}

void XPGraph::prinf_graph_config(){
    prinf_config();
}

void XPGraph::print_time_cost(double time){
    prinf_timecost_breakdown(m, time);
}

void XPGraph::print_mem_usage(){
    print_mempool_usage(levelgraph);
}

/* -------------------------------------------------------------- */
// Graph Update
bool XPGraph::add_edge(vid_t src, vid_t dst) {
    // edge_t edge(src, dst);
    edge_t edge;
    edge.src = src;
    edge.dst = dst;
    levelgraph->batch_edge(edge);
    return true;
}

index_t XPGraph::add_edges(char* buf, size_t size, index_t count) {
    index_t edge_count = size / sizeof(edge_t);
    edge_t* edges = (edge_t*)buf;
    if(count > 0) edge_count = count;

    for (index_t i = 0; i < edge_count; ++i) {
        levelgraph->batch_edge(edges[i]);
    }
    return edge_count;   
}

index_t XPGraph::buffer_edges(char* buf, size_t size, index_t count) {
    // TODO
    index_t edge_count = size / sizeof(edge_t);
    // edge_t* edges = (edge_t*)buf;
    // if(count > 0) edge_count = count;

    // for (index_t i = 0; i < edge_count; ++i) {
    //     levelgraph->batch_edge(edges[i]);
    // }
    return edge_count;   
}

bool XPGraph::del_edge(vid_t src, vid_t dst) {
    edge_t edge;
    edge.src = -src;
    edge.dst = -dst;
    levelgraph->batch_edge(edge);
    return true;
}

void XPGraph::ingest_edges_from_files(std::string idirname, index_t count) {
    m.start_time("2-ingest_edges_from_files");  
    std::string odirname = "";
    if(LEBUF_INPM) odirname = NVMPATH0;
    graph_ingestion(levelgraph, idirname, odirname, count, m);
    m.stop_time("2-ingest_edges_from_files");  
}

void XPGraph::log_edges_from_files(std::string idirname, index_t count) {
    m.start_time("2-log_edges_from_files");  
    std::string odirname = "";
    if(LEBUF_INPM) odirname = NVMPATH0;
    graph_logging(levelgraph, idirname, odirname, count, m);
    m.stop_time("2-log_edges_from_files");  
}

void XPGraph::archive_edges_from_files(std::string idirname, int residue, index_t count) {
    m.start_time("2-archive_edges_from_files");  
    std::string odirname = "";
    if(LEBUF_INPM) odirname = NVMPATH0;
    graph_archiving(levelgraph, idirname, odirname, count, residue, m);
    m.stop_time("2-archive_edges_from_files");  
}

void XPGraph::import_graph_by_config(){
    switch(JOB){
        case 0: ingest_edges_from_files(filepath, nedges); break;
        case 1: log_edges_from_files(filepath, nedges); break;
        case 2: archive_edges_from_files(filepath, residue, nedges); break;
        case 3: graph_recover(RECYPATH, levelgraph, m); break;
        default: 
            std::cout << "invalid job id " << std::endl;
            // print_usage();
    }
}

// index_t XPGraph::log_edges(char* buf, size_t size, index_t count) {
//     index_t edge_count = size / sizeof(edge_t);
//     edge_t* edges = (edge_t*)buf;
//     if(count > 0) edge_count = count;

//     for (index_t i = 0; i < edge_count; ++i) {
//         levelgraph->batch_edge_no_archive(edges[i]);
//     }
//     return edge_count;
// }

// void XPGraph::archive_edges(int residue, index_t count) {
//     index_t batch_size = (1L << residue);
//     cout << "batch_size = " << batch_size << endl;
//     ELOG_MASK = 0xFFFFFFFF;
//     index_t marker = 0, head = count;
//     while (marker < head) {
//         marker = std::min(head, marker+batch_size);
//         levelgraph->create_snapshot(marker);
//     }
// }

// index_t XPGraph::set_elog(char* buf, size_t size, index_t count) {
//     index_t line = size / sizeof(edge_t);
//     if(count > 0) line = count;
//     levelgraph->set_elog(buf, line);
//     return line;
// }

// size_t XPGraph::load_edgelist(char** buf, std::string idirname) {
//     std::string odirname = "";
//     if(LEBUF_INPM) odirname = NVMPATH0;
//     size_t total_size = alloc_and_read_dir(idirname, odirname, buf);
//     return total_size;
// }

/*-------------------------------------------------------------- */
// Graph save and recover
void XPGraph::save_gragh() {
    m.start_time("6-save_graph");
    if(PERSIST) {
        std::string odirname = RECYPATH;
        mkdir(odirname.c_str(), 0777);
        save_graph_pmem(odirname, levelgraph); 
    }
    m.stop_time("6-save_graph");
}

void XPGraph::load_graph(std::string idirname) {
    m.start_time("2-recover_graph");
    load_graph_pmem(idirname, levelgraph); 
    m.stop_time("2-recover_graph");
}

/* -------------------------------------------------------------- */
// Graph Query
vid_t XPGraph::get_vcount(){
    return levelgraph->get_vcount();
}

edgelog_t* XPGraph::get_elog(){
    return levelgraph->get_elog();
}

edge_t* XPGraph::get_loggeg_edges(index_t &count){
    edgelog_t* elog = levelgraph->get_elog();
    count = elog->head - elog->marker;
    return elog->data;
}

graph_t* XPGraph::get_out_graph(){
    return levelgraph->get_out_graph();
}

graph_t* XPGraph::get_in_graph(){
    return levelgraph->get_in_graph();
}

degree_t XPGraph::get_out_degree(vid_t vid){
    return levelgraph->get_degree(vid, 0);
}

degree_t XPGraph::get_in_degree(vid_t vid){
    return levelgraph->get_degree(vid, 1);
}

void XPGraph::print_out_nebrs(vid_t vid){
    degree_t degree = levelgraph->get_degree(vid, 0);
    vid_t* local_adjlist = new vid_t[degree];
    levelgraph->query_nebrs(local_adjlist, vid, 0);
    for (degree_t i = 0; i < degree; ++i)
        logstream(LOG_INFO) << local_adjlist[i] << ", ";
    logstream(LOG_INFO) << "degree = " << degree << std::endl;
    delete [] local_adjlist;
}

void XPGraph::print_in_nebrs(vid_t vid){
    degree_t degree = levelgraph->get_degree(vid, 1);
    vid_t* local_adjlist = new vid_t[degree];
    levelgraph->query_nebrs(local_adjlist, vid, 1);
    for (degree_t i = 0; i < degree; ++i)
        logstream(LOG_INFO) << local_adjlist[i] << ", ";
    logstream(LOG_INFO) << "degree = " << degree << std::endl;
    delete [] local_adjlist;
}

degree_t XPGraph::get_out_nebrs(vid_t vid, vid_t* local_adjlist) {
    return levelgraph->query_nebrs(local_adjlist, vid, 0);
}

degree_t XPGraph::get_in_nebrs(vid_t vid, vid_t* local_adjlist) {
    return levelgraph->query_nebrs(local_adjlist, vid, 1);
}

degree_t XPGraph::get_out_nebrs_logged(vid_t vid, vid_t* local_adjlist) {
    return levelgraph->query_nebrs_logged_mt(local_adjlist, vid, 0);
}

degree_t XPGraph::get_in_nebrs_logged(vid_t vid, vid_t* local_adjlist) {
    return levelgraph->query_nebrs_logged_mt(local_adjlist, vid, 1);
}

degree_t XPGraph::get_out_nebrs_buffered(vid_t vid, vid_t* local_adjlist) {
    return levelgraph->query_nebrs_buffered(local_adjlist, vid, 0);
}

degree_t XPGraph::get_in_nebrs_buffered(vid_t vid, vid_t* local_adjlist) {
    return levelgraph->query_nebrs_buffered(local_adjlist, vid, 1);
}

degree_t XPGraph::get_out_nebrs_flushed(vid_t vid, vid_t* local_adjlist) {
    return levelgraph->query_nebrs_flushed(local_adjlist, vid, 0);
}

degree_t XPGraph::get_in_nebrs_flushed(vid_t vid, vid_t* local_adjlist) {
    return levelgraph->query_nebrs_flushed(local_adjlist, vid, 1);
}

/* -------------------------------------------------------------- */
// Graph Arrange
index_t XPGraph::buffer_all_edges(){
    index_t head = levelgraph->get_elog()->head;
    index_t tail = levelgraph->get_elog()->tail;
    if(head != tail) levelgraph->inform_wait_buffer_all();
    return head - tail;
}

index_t XPGraph::flush_all_vbufs(){
    index_t tail = levelgraph->get_elog()->tail;
    index_t efree = levelgraph->get_elog()->efree;
    if(tail != efree) levelgraph->flush_all_bufs(tail);
    return tail - efree;
}

index_t XPGraph::archive_all_vbufs(){
    index_t head = levelgraph->get_elog()->head;
    index_t tail = levelgraph->get_elog()->tail;
    index_t efree = levelgraph->get_elog()->efree;
    if(head != efree) {
        levelgraph->inform_wait_buffer_all();
        levelgraph->flush_all_bufs(tail);
    }
    return head - efree;
}

bool XPGraph::compact_out_adjs(vid_t vid){
    return levelgraph->compact_adjlists(vid, 0);
}

bool XPGraph::compact_in_adjs(vid_t vid){
    return levelgraph->compact_adjlists(vid, 1);
}

vid_t XPGraph::compact_all_adjs(){
    return levelgraph->compact_all_adjlists();
}

/* -------------------------------------------------------------- */
// Others
uint8_t XPGraph::get_socket_num(){
    return NUM_SOCKETS;
}

uint8_t XPGraph::get_socket_id(vid_t vid){
    return GET_SOCKETID(vid);
}

void XPGraph::bind_cpu(tid_t tid, uint8_t socket_id){
    bind_thread_to_socket(tid, socket_id);
}

void XPGraph::cancel_bind_cpu(){
    cancel_thread_bind();
}

uint8_t XPGraph::get_query_count(){
    return QUERY;
}
