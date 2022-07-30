#pragma once
#include "graph/levelgraph.hpp"
#include "graph/recovergraph.hpp"

index_t buf_and_insert(const char* buf, size_t size, levelgraph_t *levelgraph, index_t count) {
    double start = mywtime(); 
    index_t edge_count = size/sizeof(edge_t);
    edge_t* edges = (edge_t*)buf;
    if(count > 0) edge_count = count;
    for (index_t i = 0; i < edge_count; ++i) {
        levelgraph->batch_edge(edges[i]);
    }
    double end = mywtime();
    cout << "  Batching time = " << end - start << " Edges = " << edge_count << endl;
    return edge_count;
}

index_t buf_and_log(const char* buf, size_t size, levelgraph_t *levelgraph, index_t count) {
    double start = mywtime(); 
    index_t edge_count = size/sizeof(edge_t);
    edge_t* edges = (edge_t*)buf;
    if(count > 0) edge_count = count;
    for (index_t i = 0; i < edge_count; ++i) {
        levelgraph->batch_edge_no_archive(edges[i]);
    }
    double end = mywtime();
    cout << "  Batching time = " << end - start << " Edges = " << edge_count << endl;
    return edge_count;
}

void prinf_timecost_breakdown(metrics &m, double time){
    std::string statistic_filename = "xpgraph_update.csv";
    std::ofstream ofs;
    ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );

    std::map<std::string, metrics_entry> entries = m.entries;
    std::string key2 = "2.2  -buf_and_insert";
    std::string key3 = "3-classify_and_buffer";
    std::string key_123 = "3.1-3 -classify";
    std::string key_4 = "3.4 -buffer";
    std::string key4 = "4-flush_all_bufs";

    std::string key3_123_prefix = "3.1-3.3  -classify_store_and_divide_t";
    std::string key3_4_prefix = "3.4  -buffer_ranges_t";
    std::string key3_1_prefix = "3.1  -count_range_edges_t";
    std::string key3_2_prefix = "3.2  -store_to_global_range_t";
    std::string key3_3_prefix = "3.3  -work_division_t";

    double time3_123 = 0, time3_4 = 0;
    double time3_1 = 0, time3_2 = 0, time3_3 = 0;

    std::string t_str;
    std::string key3_123, key3_4;
    std::string key3_1, key3_2, key3_3;

    for(tid_t tid = 0; tid < THD_COUNT; ++tid){
        if(tid < 10) t_str = "0" + std::to_string(tid);
        else t_str = std::to_string(tid);
        key3_123 = key3_123_prefix + t_str;
        key3_4 = key3_4_prefix + t_str;
        if(time3_123 < entries[key3_123].value) time3_123 = entries[key3_123].value;
        if(time3_4 < entries[key3_4].value) time3_4 = entries[key3_4].value;

        key3_1 = key3_1_prefix + t_str;
        key3_2 = key3_2_prefix + t_str;
        key3_3 = key3_3_prefix + t_str;
        if(time3_1 < entries[key3_1].value) time3_1 = entries[key3_1].value;
        if(time3_2 < entries[key3_2].value) time3_2 = entries[key3_2].value;
        if(time3_3 < entries[key3_3].value) time3_3 = entries[key3_3].value;
    } 

    ofs << "[UpdateTimings]:" << entries[key2].value << "," << entries[key3].count << ","
                        << entries[key3].value << "(" << entries[key_123].value << "+" << entries[key_4].value << ")," 
                        // // << "(" << time3_123 << "+" << time3_4 << ")" //"),"
                        // << "--classify:" << time3_123 << " (" << time3_1 << "+" << time3_2 << "+" << time3_3 << ")"
                        // << "--buffer:" << time3_4 << "," 
                        << entries[key4].value << ","
                        << time << "," ;
}

void print_mempool_usage(levelgraph_t *levelgraph){
    // count a process' virtual memory size and resident set size, and return the results in Byte.
    size_t vm, rss;
    pid_t proc_id = getpid();
    process_mem_usage(proc_id, vm, rss);
    // count global_range[].edges used space
    temp_ranged_edgelist_size *= 2; // out_edgelists + in_edgelists
    // count vbuf_pool used space
    size_t vbp_used = 0; //, vbp0_used = 0, vbp1_used = 0;
    if(MEMPOOL) {
        vbp_used = levelgraph->get_vbuf_pool_max_used();
        // vbp_used = vbp0_used + vbp1_used;
    }
    // count pblk_pools used space
    size_t pbp_used = 0, pbp_used_sub[NUM_SOCKETS];
    for(int id = 0; id < NUM_SOCKETS; ++id){
        pbp_used_sub[id] = levelgraph->get_pblk_pool(id)->print_usage();
        pbp_used += pbp_used_sub[id];
    }

    // size_t dm_used = vbp_used + vbulk_size + snap_bulk_size;
    // size_t pm_used = pbp_used + local_buf_size + elog_size;
    // if(TEDGE_INPM){
    //     pm_used += temp_ranged_edgelist_size;
    // } else {
    //     dm_used += temp_ranged_edgelist_size;
    // }
    // double pm_dm_used_ratio = pm_used * 1.0 / dm_used;
    // double pm_rss_ratio = pm_used * 1.0 / rss;

    std::string statistic_filename = "xpgraph_update.csv";
    std::ofstream ofs;
    ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    ofs << "[Memory]:" //<< time << "," << levelgraph->get_snapid() << ","
    // << B2GB(vm) << "," << B2GB(rss) << "," 
    // << B2GB(vbulk_size) << "," << B2GB(snap_bulk_size) << "," 
    // << B2GB(temp_ranged_edgelist_size) << "," 
    << B2GB(vbp_used) << "," // << "(" << B2GB(vbp_used_sub[0]) << "+" << B2GB(vbp_used_sub[1]) << ")," 
    << B2GB(pbp_used) << "(" << B2GB(pbp_used_sub[0]) << "+" << B2GB(pbp_used_sub[1]) << ")," ;
    // << B2GB(local_buf_size) << "," << B2GB(elog_size) << ","
    // << B2GB(dm_used) << "," << B2GB(pm_used) << "," 
    // << pm_dm_used_ratio << "," << pm_rss_ratio << ","; //<< std::endl;
    ofs << std::endl;
    ofs.close();
}

void graph_ingestion(levelgraph_t *levelgraph, std::string idirname, std::string odirname, index_t count, metrics &m){
    //allocate accuately and read files
    char* buf = 0;
    m.start_time("2.1  -alloc_read");  
    double start = mywtime ();
    size_t total_size = alloc_and_read_dir(idirname, odirname, &buf);
    double end = mywtime();
    std::cout << "  Read/alloc time = " << end - start << ", edge count = " << total_size/sizeof(edge_t) << endl;
    m.stop_time("2.1  -alloc_read");

    start = mywtime ();
    // batch edges one by one
    m.start_time("2.2  -buf_and_insert");
    index_t line = buf_and_insert(buf, total_size, levelgraph, count);
    m.set("nedges", (size_t)line);
    m.stop_time("2.2  -buf_and_insert");

    // wait for finishing archive
    m.start_time("2.3  -inform_wait_buffer_all"); 
    levelgraph->inform_wait_buffer_all();
    m.stop_time("2.3  -inform_wait_buffer_all"); 
    end = mywtime ();
    cout << "Ingest graph time = " << end - start << endl;
    levelgraph->print_edgeshard();

    if(LEBUF_INPM && buf){ pmem_unmap(buf, total_size); buf = 0;}
    else if(buf){ free(buf); buf = 0;}
    prinf_timecost_breakdown(m, end - start);
    print_mempool_usage(levelgraph);
}

void graph_logging(levelgraph_t *levelgraph, std::string idirname, std::string odirname, index_t count, metrics &m){
    //allocate accuately and read files
    char* buf = 0;
    m.start_time("2.1  -alloc_read");  
    double start = mywtime ();
    size_t total_size = alloc_and_read_dir(idirname, odirname, &buf);
    double end = mywtime();
    std::cout << "  Read/alloc time = " << end - start << ", edge count = " << total_size/sizeof(edge_t) << endl;
    m.stop_time("2.1  -alloc_read");
    
    // batch edges one by one
    m.start_time("2.2  -buf_and_insert");
    index_t line = buf_and_log(buf, total_size, levelgraph, count);
    m.set("nedges", (size_t)line);
    m.stop_time("2.2  -buf_and_insert");
    // levelgraph->free_elog(); 
}

void graph_archiving(levelgraph_t *levelgraph, std::string idirname, std::string odirname, index_t count, int residue, metrics &m){
    //allocate accuately and read files
    char* buf = 0;
    m.start_time("2.1  -alloc_read");  
    double start = mywtime ();
    size_t total_size = alloc_and_read_dir(idirname, odirname, &buf);
    double end = mywtime();
    std::cout << "  Read/alloc time = " << end - start << ", edge count = " << total_size/sizeof(edge_t) << endl;
    index_t line = total_size / sizeof(edge_t);
    if(count > 0) line = count;
    levelgraph->reset_elog(buf, line);
    m.stop_time("2.1  -alloc_read");
    m.set("nedges", (size_t)line);
    
    m.start_time("2.2  -buf_and_insert");
    // Make Graph by batched archive
    start = mywtime();
    index_t batch_size = (1L << residue);
    cout << "batch_size = " << batch_size << endl;
    ELOG_MASK = 0xFFFFFFFF;
    index_t marker = 0, head = line;
    while (marker < head) {
        marker = std::min(head, marker+batch_size);
        levelgraph->create_snapshot(marker);
    }
    m.stop_time("2.2  -buf_and_insert");
    
    // wait for finishing archive
    m.start_time("2.3  -inform_wait_buffer_all"); 
    levelgraph->inform_wait_buffer_all();
    m.stop_time("2.3  -inform_wait_buffer_all"); 
    end = mywtime ();
    cout << "Archive graph time = " << end - start << endl;
    levelgraph->print_edgeshard();
    
    prinf_timecost_breakdown(m, end - start);
    print_mempool_usage(levelgraph);
}

void graph_recover(std::string filename, levelgraph_t *levelgraph, metrics &m) {
    double start, end;

    m.start_time("2.1  -load_graph");
    // levelgraph->init_elog(); 
    // levelgraph->init_graph(); 

    start = mywtime ();
    // load_graph(levelgraph);
    load_graph_pmem(filename, levelgraph); 
    end = mywtime ();
    cout << "Recover graph time = " << end - start << endl;
    
    // std::string statistic_filename = "xpgraph_update.csv";
    // std::ofstream ofs;
    // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    // ofs << "load:" << end - start << ","; //<< std::endl;
    // ofs.close();

    m.stop_time("2.1  -load_graph");
}
