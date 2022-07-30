#pragma once

#include "graph/edgelog.hpp"
#include "graph/graph.hpp"

//for each rid
class global_range_t {
  public:
      index_t* count;
      edge_t** edges;
};

//for each thread 
class thd_local_t {
  public:
      vid_t** vid_range; //number of edges in each rid
      rid_t*  range_end;
};

class edge_shard_t {
 public:
    // elog in levelgraph
    edgelog_t* elog; 
    // metric info
    metrics &m;

    // used for classify edges
    rid_t RANGE_COUNT = 256;

    //intermediate classification buffer
    edge_t* edge_buf_out;
    edge_t* edge_buf_in;
    index_t edge_buf_count;
    
    sktid_t num_subgraphs;
    global_range_t* global_range_out;
    global_range_t* global_range_in;
    // thd_local_t* thd_local_out;
    // thd_local_t* thd_local_in;

    // thd_local_t
    index_t*** thd_ecount_out;
    index_t*** thd_ecount_in;
    rid_t** range_end_out;
    rid_t** range_end_in;

    index_t* out_edgesum_per_thread;
    index_t* in_edgesum_per_thread;

 public:
    edge_shard_t(edgelog_t* _elog, metrics &_m):elog(_elog), m(_m){
        edge_buf_count = BATCH_CAP;
        edge_buf_out = (edge_t*)calloc(edge_buf_count, sizeof(edge_t));
        edge_buf_in = (edge_t*)calloc(edge_buf_count, sizeof(edge_t));

        if(NUMA_OPT == 2){
            num_subgraphs = NUM_SOCKETS;
        } else {
            num_subgraphs = 1;
        }

        global_range_out = (global_range_t*)calloc(RANGE_COUNT, sizeof(global_range_t));
        global_range_in = (global_range_t*)calloc(RANGE_COUNT, sizeof(global_range_t));
        for(rid_t rid = 0; rid < RANGE_COUNT; ++rid){
            global_range_out[rid].count = (index_t*)calloc(num_subgraphs, sizeof(index_t));
            global_range_out[rid].edges = (edge_t**)calloc(num_subgraphs, sizeof(edge_t*));
            global_range_in[rid].count = (index_t*)calloc(num_subgraphs, sizeof(index_t));
            global_range_in[rid].edges = (edge_t**)calloc(num_subgraphs, sizeof(edge_t*));
        }

        thd_ecount_out = (index_t***) calloc(THD_COUNT, sizeof(index_t**)); 
        thd_ecount_in = (index_t***) calloc(THD_COUNT, sizeof(index_t**));
        for(tid_t tid = 0; tid < THD_COUNT; ++tid){
            thd_ecount_out[tid] = (index_t**)calloc(num_subgraphs, sizeof(index_t*)); 
            thd_ecount_in[tid] = (index_t**)calloc(num_subgraphs, sizeof(index_t*)); 
            for(sktid_t sid = 0; sid < num_subgraphs; ++sid){
                thd_ecount_out[tid][sid] = (index_t*)calloc(RANGE_COUNT, sizeof(index_t)); 
                thd_ecount_in[tid][sid] = (index_t*)calloc(RANGE_COUNT, sizeof(index_t)); 
            }
        }

        range_end_out = (rid_t**) calloc(THD_COUNT, sizeof(rid_t*)); 
        range_end_in = (rid_t**) calloc(THD_COUNT, sizeof(rid_t*)); 
        for(tid_t tid = 0; tid < THD_COUNT; ++tid){
            range_end_out[tid] = (rid_t*)calloc(num_subgraphs, sizeof(rid_t)); 
            range_end_in[tid] = (rid_t*)calloc(num_subgraphs, sizeof(rid_t)); 
        }

        out_edgesum_per_thread = (index_t*) calloc(THD_COUNT, sizeof(index_t)); 
        in_edgesum_per_thread = (index_t*) calloc(THD_COUNT, sizeof(index_t)); 
    }

    ~edge_shard_t(){
        if(out_edgesum_per_thread) free(out_edgesum_per_thread);
        if(in_edgesum_per_thread) free(in_edgesum_per_thread);

        for(tid_t tid = 0; tid < THD_COUNT; ++tid){
            if(range_end_out[tid]) free(range_end_out[tid]);
            if(range_end_in[tid]) free(range_end_in[tid]);
        }
        if(range_end_out) free(range_end_out);
        if(range_end_in) free(range_end_in);

        for(tid_t tid = 0; tid < THD_COUNT; ++tid){
            for(sktid_t sid = 0; sid < num_subgraphs; ++sid){
                if(thd_ecount_out[tid][sid]) free(thd_ecount_out[tid][sid]);
                if(thd_ecount_in[tid][sid]) free(thd_ecount_in[tid][sid]);
            }
            if(thd_ecount_out[tid]) free(thd_ecount_out[tid]);
            if(thd_ecount_in[tid]) free(thd_ecount_in[tid]);
        }
        if(thd_ecount_out) free(thd_ecount_out);
        if(thd_ecount_in) free(thd_ecount_in);

        for(rid_t rid = 0; rid < RANGE_COUNT; ++rid){
            if(global_range_out[rid].count) free(global_range_out[rid].count);
            if(global_range_out[rid].edges) free(global_range_out[rid].edges);
            if(global_range_in[rid].count) free(global_range_in[rid].count);
            if(global_range_in[rid].edges) free(global_range_in[rid].edges);
        }
        if(global_range_out) free(global_range_out);
        if(global_range_in) free(global_range_in);

        if(edge_buf_out) free(edge_buf_out);
        if(edge_buf_in) free(edge_buf_in);
    }

    void classify_store_and_divide(vid_t v_count, tid_t thd_count);
    void classify_and_buffer_d(index_t snap_id, graph_t* out_graph, graph_t* in_graph); // out + in
    void archive_all_d(graph_t* out_graph, graph_t* in_graph);
    void cleanup();
    void print_edgesum_per_thread();

 private:
    // void count_range_edges(vid_t bit_shift);
    void count_range_edges(vid_t bit_shift, index_t** ecount_out, index_t** ecount_in);
    void prefix_sum(global_range_t* global_range, index_t*** thd_ecount, tid_t thd_count, bool is_in_graph);
    void alloc_temp_edges(global_range_t* global_range, bool is_in_graph);
    void point_to_temp_edges(global_range_t* global_range, bool is_in_graph);
    void store_to_global_range(vid_t bit_shift, index_t** ecount_out, index_t** ecount_in);
    void work_division(global_range_t* global_range, rid_t** range_end, sktid_t sid, index_t equal_work);
    // void work_division(global_range_t* global_range, thd_local_t* thd_local, sktid_t sid, index_t equal_work);
    void work_division_GO(global_range_t* global_range, rid_t** range_end, sktid_t sid, index_t equal_work);
    // void work_division_GO(global_range_t* global_range, thd_local_t* thd_local, sktid_t sid, rid_t range_count, tid_t thd_count, index_t equal_work);
    // void classify_store_and_divide(vid_t v_count);
    index_t buffer_ranges(index_t snap_id, graph_t* graph, global_range_t* global_range, sktid_t sid, rid_t r_start, rid_t r_end);
};

// count the number of edges in each rid (total 256 ranges)
void edge_shard_t::count_range_edges(vid_t bit_shift, index_t** ecount_out, index_t** ecount_in) {
    assert(ecount_out || ecount_in);
    vid_t src, dst;
    sktid_t sid; // socket id
    rid_t rid;
    edge_t* edges = elog->data;
    index_t index;

    if(NUMA_OPT == 2){
        #pragma omp for schedule (static) nowait // nowait 子句用于消除隐式的 barrier
        for (index_t eid = elog->tail; eid < elog->marker; ++eid) {
            index = eid & ELOG_MASK;
            src = edges[index].src;
            dst = edges[index].dst;
            if(ecount_out){
                sid = GET_SOCKETID(src);
                rid = (src >> bit_shift);
                // assert(rid < RANGE_COUNT);
                ecount_out[sid][rid] += 1;
            }
            if(ecount_in){
                sid = GET_SOCKETID(dst);
                rid = (dst >> bit_shift);
                // assert(rid < RANGE_COUNT);
                ecount_in[sid][rid] += 1;
            }
        }
    } else {
        sid = 0;
        #pragma omp for schedule (static) nowait
        for (index_t eid = elog->tail; eid < elog->marker; ++eid) {
            index = eid & ELOG_MASK;
            src = edges[index].src;
            dst = edges[index].dst;
            if(ecount_out){
                rid = (src >> bit_shift);
                // assert(rid < RANGE_COUNT);
                ecount_out[sid][rid] += 1;
            }
            if(ecount_in){
                rid = (dst >> bit_shift);
                // assert(rid < RANGE_COUNT);
                ecount_in[sid][rid] += 1;
            }
        }
    }
}

void edge_shard_t::prefix_sum(global_range_t* global_range, index_t*** thd_ecount, tid_t thd_count, bool is_in_graph){
    #pragma omp for schedule(static) nowait
    for (rid_t rid = 0; rid < RANGE_COUNT; ++rid) {
        for(sktid_t sid = 0; sid < num_subgraphs; ++sid){
            index_t total = 0;
            index_t value = 0;
            for (tid_t tid = 0; tid < thd_count; ++tid) {
                value = thd_ecount[tid][sid][rid];
                thd_ecount[tid][sid][rid] = total;
                total += value;
            }       
            global_range[rid].count[sid] = total;     
        }
    }
}

void edge_shard_t::alloc_temp_edges(global_range_t* global_range, bool is_in_graph){
    #pragma omp for schedule(static) nowait
    for (rid_t rid = 0; rid < RANGE_COUNT; ++rid){
        for(sktid_t sid = 0; sid < num_subgraphs; ++sid){
            index_t total = global_range[rid].count[sid];
            global_range[rid].edges[sid] = 0;
            if (total != 0) {
                if(TEDGE_INPM){
                    std::string global_range_file;
                    if(!is_in_graph) global_range_file = NVMPATH0 + "global_range_pool_" + std::to_string(rid) + ".txt";
                    else global_range_file = NVMPATH1 + "global_range_pool_" + std::to_string(rid) + ".txt";
                    global_range[rid].edges[sid] = (edge_t*)pmem_alloc(global_range_file.c_str(), total * sizeof(edge_t));
                } else {
                    global_range[rid].edges[sid] = (edge_t*)calloc(total, sizeof(edge_t));
                }
                if (0 == global_range[rid].edges[sid]) {
                    std::cout << total << " bytes of allocation failed" << std::endl;
                    assert(0);
                }
            }
        }
    }
}

void edge_shard_t::point_to_temp_edges(global_range_t* global_range, bool is_in_graph){
    // compute rid
    index_t sum = 0, count = 0;
    edge_t* edge_buf = 0;
    if(is_in_graph == 0) edge_buf = edge_buf_out;
    else edge_buf = edge_buf_in;

    for (rid_t rid = 0; rid < RANGE_COUNT; ++rid){
        for(sktid_t sid = 0; sid < num_subgraphs; ++sid){
            count = global_range[rid].count[sid];
            if(count > 0){
                global_range[rid].edges[sid] = edge_buf + sum;
                sum += count;
            } else {
                global_range[rid].edges[sid] = 0;
            }
        }
    }
    assert(sum <= edge_buf_count);
}

// store edges to their corresponding global_range[rid].edges
void edge_shard_t::store_to_global_range(vid_t bit_shift, index_t** ecount_out, index_t** ecount_in){
    assert(ecount_out || ecount_in);
    vid_t src, dst;
    sktid_t sid;
    rid_t rid;
    edge_t* edge = 0;
    edge_t* edges = elog->data;
    index_t index;

    if(NUMA_OPT == 2){
        #pragma omp for schedule (static) nowait
        for (index_t eid = elog->tail; eid < elog->marker; ++eid) {
            index = eid & ELOG_MASK;
            src = edges[index].src;
            dst = edges[index].dst;
            if(ecount_out){
                sid = GET_SOCKETID(src);
                rid = (src >> bit_shift);
                edge = global_range_out[rid].edges[sid] + ecount_out[sid][rid]; 
                ecount_out[sid][rid] += 1;
                edge->src = src;
                edge->dst = dst;
                // cout << edge->src << "->" << edge->dst << endl;
            }
            if(ecount_in){
                sid = GET_SOCKETID(dst);
                rid = (dst >> bit_shift);
                edge = global_range_in[rid].edges[sid] + ecount_in[sid][rid]; 
                ecount_in[sid][rid] += 1;
                edge->src = dst;
                edge->dst = src;
                // cout << edge->src << "->" << edge->dst << endl;
            }
        }
    } else {
        sid = 0;
        #pragma omp for schedule (static) nowait
        for (index_t eid = elog->tail; eid < elog->marker; ++eid) {
            index = eid & ELOG_MASK;
            src = edges[index].src;
            dst = edges[index].dst;
            if(ecount_out){
                rid = (src >> bit_shift);
                edge = global_range_out[rid].edges[sid] + ecount_out[sid][rid]; 
                ecount_out[sid][rid] += 1;
                edge->src = src;
                edge->dst = dst;
            }
            if(ecount_in){
                rid = (dst >> bit_shift);
                edge = global_range_in[rid].edges[sid] + ecount_in[sid][rid]; 
                ecount_in[sid][rid] += 1;
                edge->src = dst;
                edge->dst = src;
            }
        }
    }
}

// allcate ranges to threads
void edge_shard_t::work_division(global_range_t* global_range, rid_t** range_end, sktid_t sid, index_t equal_work){
    // cleanup 
    for(tid_t tid = 0; tid < THD_COUNT; ++tid){
        range_end[tid][sid] = 0;
    }

    index_t my_portion = 0;
    tid_t tid = 0;
    for (rid_t rid = 0; rid < RANGE_COUNT && tid < THD_COUNT; ++rid) {
        my_portion += global_range[rid].count[sid];
        if (my_portion > equal_work) {
            range_end[tid++][sid] = rid+1;
            my_portion = 0;
        }
    }
    if (tid == THD_COUNT)
        range_end[tid-1][sid] = RANGE_COUNT;
    else 
        range_end[tid++][sid] = RANGE_COUNT;
}

// allcate ranges to threads used in GraphOne[FAST19]
// void edge_shard_t::work_division_GO(global_range_t* global_range, thd_local_t* thd_local, sktid_t sid, rid_t range_count, tid_t thd_count, index_t equal_work){
void edge_shard_t::work_division_GO(global_range_t* global_range, rid_t** range_end, sktid_t sid, index_t equal_work){
    // cleanup 
    for(tid_t tid = 0; tid < THD_COUNT; ++tid){
        range_end[tid][sid] = 0;
    }
    
    index_t my_portion = global_range[0].count[sid];
    index_t value;
    tid_t tid = 0;
    for (rid_t rid = 1; rid < RANGE_COUNT && tid < THD_COUNT; ++rid) {
        value = global_range[rid].count[sid];
        if (my_portion && my_portion + value > equal_work) {
            range_end[tid++][sid] = rid;
            my_portion = 0;
        }
        my_portion += value;
    }
    if (tid == THD_COUNT)
        range_end[tid-1][sid] = RANGE_COUNT;
    else 
        range_end[tid++][sid] = RANGE_COUNT;
}

// classify ranges and divide ranges to threads
void edge_shard_t::classify_store_and_divide(vid_t v_count, tid_t thd_count){
    tid_t tid = omp_get_thread_num();
    // std::string t_str;
    // if(tid < 10) t_str = "0" + std::to_string(tid);
    // else t_str = std::to_string(tid);
    // m.start_time("3.1-3.3  -classify_store_and_divide_t" + t_str);

    // m.start_time("3.1  -count_range_edges_t" + t_str);
    vid_t  base_vid = ((v_count -1)/RANGE_COUNT);
    if (base_vid == 0) { base_vid = RANGE_COUNT;}
    //find the number of bits to do shift to find the rid
    vid_t bit_shift = __builtin_clzl(base_vid);
    bit_shift = 64 - bit_shift; 

    //cleanup here
    for(sktid_t sid = 0; sid < num_subgraphs; ++sid){
        memset(thd_ecount_out[tid][sid], 0, RANGE_COUNT * sizeof(index_t));
        memset(thd_ecount_in[tid][sid], 0, RANGE_COUNT * sizeof(index_t));
    }
    index_t** ecount_out = thd_ecount_out[tid];
    index_t** ecount_in = thd_ecount_in[tid];

    count_range_edges(bit_shift, ecount_out, ecount_in);

    // m.start_time("3.1.1  -count_range_edges_barrier_t" + t_str);
    #pragma omp barrier // important barrier
    // m.stop_time("3.1.1  -count_range_edges_barrier_t" + t_str);

    prefix_sum(global_range_out, thd_ecount_out, thd_count, 0);
    prefix_sum(global_range_in, thd_ecount_in, thd_count, 1);

    // m.stop_time("3.1  -count_range_edges_t" + t_str);

    // m.start_time("3.2  -store_to_global_range_t" + t_str);
    // m.start_time("3.2.1  -alloc_temp_edges_barrier_t" + t_str);
    index_t total_edge_count = elog->marker - elog->tail;
    #pragma omp single nowait
    {
        size_t total_edge_size = total_edge_count * sizeof(edge_t);
        if(temp_ranged_edgelist_size < total_edge_size) temp_ranged_edgelist_size = total_edge_size;
        if(edge_buf_count < total_edge_count){
            edge_buf_count = total_edge_count;
            edge_buf_out = (edge_t*)realloc(edge_buf_out, edge_buf_count * sizeof(edge_t));
            edge_buf_in = (edge_t*)realloc(edge_buf_in, edge_buf_count * sizeof(edge_t));
        }
    }
    #pragma omp barrier

    #pragma omp single nowait
    {
        // m.start_time("3.2.0  -point_to_temp_edges_t" + t_str);
        point_to_temp_edges(global_range_out, 0);
        // m.stop_time("3.2.0  -point_to_temp_edges_t" + t_str);
    }
    #pragma omp single nowait
    {
        // m.start_time("3.2.0  -point_to_temp_edges_t" + t_str);
        point_to_temp_edges(global_range_in, 1);
        // m.stop_time("3.2.0  -point_to_temp_edges_t" + t_str);
    }

    #pragma omp barrier // important barrier
    // m.stop_time("3.2.1  -alloc_temp_edges_barrier_t" + t_str);

    store_to_global_range(bit_shift, ecount_out, ecount_in);
    // m.stop_time("3.2  -store_to_global_range_t" + t_str);

    // m.start_time("3.3  -work_division_t" + t_str);
    index_t edge_count = (total_edge_count*1.15)/(THD_COUNT)/(num_subgraphs); // --> work_division_GO()
    // index_t edge_count = (total_edge_count*0.85)/(THD_COUNT)/(num_subgraphs); // --> work_division()

    for(sktid_t sid = 0; sid < num_subgraphs; ++sid){
        // if (tid == sid || (tid == 0 && THD_COUNT < num_subgraphs)) {
        // 将各个区间的出边分给多线程，由任意一个线程执行
        #pragma omp single nowait
        {
            // m.start_time("3.3.1  -work_division_t" + t_str);
            work_division_GO(global_range_out, range_end_out, sid, edge_count);
            // m.stop_time("3.3.1  -work_division_t" + t_str);
        }
        // if (tid == THD_COUNT - id || (tid == 0 && THD_COUNT == 1)) {
        // if (tid == ((id + num_subgraphs) & THD_COUNT) || (tid == 0 && THD_COUNT == 1)) {
        // if (tid == sid || (tid == 0 && THD_COUNT < num_subgraphs)) {
        #pragma omp single nowait
        {
            // m.start_time("3.3.1  -work_division_t" + t_str);
            work_division_GO(global_range_in, range_end_in, sid, edge_count);
            // m.stop_time("3.3.1  -work_division_t" + t_str);
        }
    }
    // m.stop_time("3.1-3.3  -classify_store_and_divide_t" + t_str);
}

// buffer ranged edges to vertex buffers
index_t edge_shard_t::buffer_ranges(index_t snap_id, graph_t* graph, global_range_t* global_range, sktid_t sid, rid_t r_start, rid_t r_end){
    if(r_start >= r_end) return 0;
    edge_t* edges = 0;
    index_t count = 0, total_count = 0;
    for(rid_t rid = r_start; rid < r_end; rid++){
        edges = global_range[rid].edges[sid];
        count = global_range[rid].count[sid];
        if(count != 0){
            // logstream(LOG_INFO) << "  - buffer_range_edges: range_id = " << rid << ", total_edges = " << count << ", src_index = " << (int)0 << ", snap_id = " << snap_id << ", tid = " << omp_get_thread_num() << std::endl;
            graph->buffer_range_edges(edges, count, snap_id);
        }
        total_count += count;
    }
    // logstream(LOG_WARNING) << "buffer_ranges: [" << r_start << ", " << r_end << "), total edge count = " << total_count << ", tid = " << tid << ", snap_id = " << snap_id << std::endl;
    return total_count;
    // tid_t tid = omp_get_thread_num();
    // if(graph->get_is_in_graph() == 0) out_edgesum_per_thread[tid] += total_count;
    // else in_edgesum_per_thread[tid] += total_count;
}

void edge_shard_t::classify_and_buffer_d(index_t snap_id, graph_t* out_graph, graph_t* in_graph){
    tid_t tid = omp_get_thread_num();
    // std::string t_str;
    // if(tid < 10) t_str = "0" + std::to_string(tid);
    // else t_str = std::to_string(tid);

    // vid_t v_count = out_graph->get_vcount();
    // classify_store_and_divide(v_count, THD_COUNT);
    // #pragma omp barrier

    // m.start_time("3.4  -buffer_ranges_t" + t_str);
    if(NUMA_OPT == 2){ // Sub-graph based NUMA optimization
        rid_t r_start, r_end;
        for(sktid_t sid = 0; sid < num_subgraphs; ++sid){
            bind_thread_to_socket(tid, sid);
            // buffering out-graph
            if (tid == 0) { 
                r_start = 0; 
            } else { 
                r_start = range_end_out[tid - 1][sid];
            }
            r_end = range_end_out[tid][sid];
            if(r_end > r_start){
                // m.start_time("3.4.1  -buffer_ranges_out_t" + t_str);
                index_t total_count = buffer_ranges(snap_id, out_graph, global_range_out, sid, r_start, r_end);
                out_edgesum_per_thread[tid] += total_count;
                // m.stop_time("3.4.1  -buffer_ranges_out_t" + t_str);
            }

            // buffering in-graph
            if (tid == THD_COUNT - 1) { 
                r_start = 0; 
            } else { 
                r_start = range_end_in[THD_COUNT - 2 - tid][sid];
            }
            r_end = range_end_in[THD_COUNT - 1 - tid][sid];
            
            if(r_end > r_start){
                // m.start_time("3.4.2  -buffer_ranges_in_t" + t_str);
                index_t total_count = buffer_ranges(snap_id, in_graph, global_range_in, sid, r_start, r_end);
                in_edgesum_per_thread[tid] += total_count;
                // m.stop_time("3.4.2  -buffer_ranges_in_t" + t_str);
            }
            
            // //cleanup here
            // thd_local_out[id][tid].range_end = 0;
            // thd_local_in[id][tid].range_end = 0; 
        }
        cancel_thread_bind();
    } else { // Out/in-graph based NUMA optimization or No NUMA optimization
        assert(num_subgraphs == 1);
        rid_t r_start_out, r_end_out;
        if (tid == 0) { 
            r_start_out = 0; 
        } else { 
            r_start_out = range_end_out[tid - 1][0];
        }
        r_end_out = range_end_out[tid][0];
        if(NUMA_OPT == 1) bind_thread_to_socket(tid, 0); // bind to socket 0 for Out-graph
        if(r_end_out > r_start_out){
            index_t total_count = buffer_ranges(snap_id, out_graph, global_range_out, 0, r_start_out, r_end_out);
            out_edgesum_per_thread[tid] += total_count;
        }

        rid_t r_start_in, r_end_in;
        if (tid == THD_COUNT - 1) { 
            r_start_in = 0; 
        } else { 
            r_start_in = range_end_in[THD_COUNT - 2 - tid][0];
        }
        r_end_in = range_end_in[THD_COUNT - 1 - tid][0];
        if(NUMA_OPT == 1) bind_thread_to_socket(tid, 1); // bind to socket 1 for In-graph
        if(r_end_in > r_start_in){
            index_t total_count = buffer_ranges(snap_id, in_graph, global_range_in, 0, r_start_in, r_end_in);
            in_edgesum_per_thread[tid] += total_count;
        }

        cancel_thread_bind();
    }
    // m.stop_time("3.4  -buffer_ranges_t" + t_str);

    // m.start_time("3.5  -cleanup" + t_str);
    // #pragma omp barrier 
    // cleanup();
    // m.stop_time("3.5  -cleanup" + t_str);
}

void edge_shard_t::archive_all_d(graph_t* out_graph, graph_t* in_graph){
     vid_t v_count = out_graph->get_vcount();
    vid_t  base_vid = ((v_count -1)/THD_COUNT);
    if (base_vid == 0) { base_vid = THD_COUNT;}
    //find the number of bits to do shift to find the rid
    vid_t bit_shift = __builtin_clzl(base_vid);
    bit_shift = 64 - bit_shift; 
    tid_t tid = omp_get_thread_num();

    vid_t v_start_out, v_end_out;
    if (tid == 0) { 
        v_start_out = 0; 
    } else { 
        v_start_out = (tid << bit_shift);
    }
    v_end_out = ((tid + 1)  << bit_shift);
    if(v_start_out > v_count) v_start_out = v_count;
    if(v_end_out > v_count) v_end_out = v_count;
    for(sktid_t id = 0; id < num_subgraphs; ++id){
        bind_thread_to_socket(tid, id);
        out_graph->archive_range_vbufs(v_start_out + id, v_end_out, num_subgraphs);
    }

    vid_t v_start_in, v_end_in;
    if (tid == THD_COUNT - 1) { 
        v_start_in = 0; 
    } else { 
        v_start_in = (THD_COUNT - 1 - tid) << bit_shift;
    }
    v_end_in = (THD_COUNT - tid) << bit_shift;
    if(v_start_in > v_count) v_start_in = v_count;
    if(v_end_in > v_count) v_end_in = v_count;

    for(sktid_t id = 0; id < num_subgraphs; ++id){
        bind_thread_to_socket(tid, id);
        in_graph->archive_range_vbufs(v_start_in + id, v_end_in, num_subgraphs);
    }
}

void edge_shard_t::cleanup(){
    #pragma omp for schedule (static) //nowait
    for (rid_t rid = 0; rid < RANGE_COUNT; ++rid) {
        for(sktid_t sid = 0; sid < num_subgraphs; ++sid){
            if(!TEDGE_INPM){
                if(global_range_out[rid].edges[sid]){
                    free(global_range_out[rid].edges[sid]);
                }
                if (global_range_in[rid].edges[sid]) {
                    free(global_range_in[rid].edges[sid]);
                }
            } else {
                if(global_range_out[rid].edges[sid]){
                    pmem_unmap(global_range_out[rid].edges[sid], global_range_out[rid].count[sid] * sizeof(edge_t));
                }
                if (global_range_in[rid].edges) {
                    pmem_unmap(global_range_in[rid].edges[sid], global_range_in[rid].count[sid] * sizeof(edge_t));
                }
            }
            global_range_out[rid].count[sid] = 0;
            global_range_in[rid].count[sid] = 0;
        }
    }
}

void edge_shard_t::print_edgesum_per_thread(){
    std::cout << "edge_shard->print_edgesum_per_thread: " << std::endl;
    index_t out_sum = 0, in_sum = 0;
    index_t out_avg = 0, in_avg = 0;
    for (tid_t t = 0; t < THD_COUNT; ++t){
        out_sum += out_edgesum_per_thread[t];
        in_sum += in_edgesum_per_thread[t];
    }
    out_avg = out_sum/THD_COUNT;
    in_avg = in_sum/THD_COUNT;

    for (tid_t t = 0; t < THD_COUNT; ++t){
        std::cout << "\t out_edgesum[" << (int)t << "] = " << out_edgesum_per_thread[t] << ", x:avg = " << out_edgesum_per_thread[t]*1.0/out_avg
                  << "\t in_edgesum[" << (int)t << "] = " << in_edgesum_per_thread[t] << ", x:avg = " << in_edgesum_per_thread[t]*1.0/in_avg << std::endl;
    }
    std::cout << "Out-edge sum = " << out_sum << ", in-edge sum = " << in_sum << std::endl;
}