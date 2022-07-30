#pragma once

#include "graph/edgeshard.hpp"
#include "config/args_config.hpp"

class levelgraph_t
{
private:
    // edge log storing new coming edge lists
    edgelog_t* elog;
    edge_shard_t* edge_shard;
    bool is_finished;

    // metadata: vertex and snap info
    vid_t nverts;
    index_t snap_id;
    mempool_t* vert_pool;
    mempool_t* snap_pool;
    pthread_t       snap_thread;
    pthread_mutex_t snap_mutex;
    pthread_cond_t  snap_condition;

    // adjlists of graph
    graph_t* out_graph;
    graph_t* in_graph;
    mempool_t* vbuf_pool;
    mempool_t** pblk_pools;
    size_t vbuf_pool_max_used;

    // metric info
    metrics &m;

public:
    levelgraph_t(vid_t _nverts, metrics &_m):nverts(_nverts), m(_m){
        vbuf_pool_max_used = 0;
        m.set("nverts", (size_t)nverts);
    }
    ~levelgraph_t(){
    }

    /* -------------------------------------------------------------- */
    // Init levelgraph
    void init_elog();
    void free_elog();
    void reset_elog(char* buf, index_t count);
    void init_graph();
    void free_graph();

    /* -------------------------------------------------------------- */
    // alloc and free elog and mempools
    void alloc_elog_data();
    void free_elog_data();
    void alloc_pblk_pools();
    void free_pblk_pools();
    void alloc_vbuf_pool();
    void free_vbuf_pool();
    void alloc_vert_pool();
    void free_vert_pool();
    void alloc_snap_pool();
    void free_snap_pool();

    /* -------------------------------------------------------------- */
    // manage snapshots
    void create_threads(){
        logstream(LOG_INFO) << "create_snapthread... \t tid = " << omp_get_thread_num() << std::endl;
        pthread_mutex_init(&snap_mutex, 0);
        pthread_cond_init(&snap_condition, 0);
        // create new thread to conduct snap_func
        if (0 != pthread_create(&snap_thread, 0, levelgraph_t::snap_func, (void*)this)){
            assert(0);
        }
    }
    static void* snap_func(void* arg){
        levelgraph_t* ptr = (levelgraph_t*)(arg);
        do {
            pthread_mutex_lock(&ptr->snap_mutex);
            pthread_cond_wait(&ptr->snap_condition, &ptr->snap_mutex);
            // struct timespec ts;
            // clock_gettime(CLOCK_REALTIME, &ts);
            // ts.tv_nsec += 100 * 1000000;  //30 is my milliseconds
            // pthread_cond_timedwait(&ptr->snap_condition, &ptr->snap_mutex, &ts);
            pthread_mutex_unlock(&ptr->snap_mutex);
            // ptr->create_snapshot();
            while (ptr->create_snapshot());
        } while(1);
        return 0;
    }

    /* -------------------------------------------------------------- */
    // get levelgraph info
    inline vid_t get_vcount(){ return nverts; }
    inline vid_t get_snapid(){ return snap_id; }
    inline edgelog_t* get_elog(){ return elog; }  
    inline graph_t* get_out_graph(){ return out_graph; }
    inline graph_t* get_in_graph(){ return in_graph; }
    inline mempool_t* get_vbuf_pool(){ return vbuf_pool; }  
    inline mempool_t* get_pblk_pool(uint8_t id){ return pblk_pools[id]; }  
    inline size_t get_vbuf_pool_max_used(){ 
        if(vbuf_pool->get_used() > vbuf_pool_max_used) vbuf_pool_max_used = vbuf_pool->get_used();
        return vbuf_pool_max_used; 
    } 

    /* -------------------------------------------------------------- */
    // update and query adjacency info
    void batch_edge_no_archive(edge_t edge);
    void batch_edge(edge_t edge);
    bool create_snapshot(index_t marker1 = 0);
    void inform_buffer();
    void archive_edges_d(index_t count);
    void archive_edges(index_t count);
    void flush_all_bufs(index_t marker1);
    void inform_wait_buffer_all();
    degree_t get_degree(vid_t vid, bool is_in_graph); // 0 for out-graph, 1 for in_graph
    degree_t query_nebrs(vid_t* neighbors, vid_t vid, bool is_in_graph);
    degree_t query_nebrs_logged_mt(vid_t* neighbors, vid_t vid, bool is_in_graph);
    degree_t query_nebrs_logged_st(vid_t* neighbors, vid_t vid, bool is_in_graph);
    degree_t query_nebrs_buffered(vid_t* neighbors, vid_t vid, bool is_in_graph);
    degree_t query_nebrs_flushed(vid_t* neighbors, vid_t vid, bool is_in_graph);
    bool compact_adjlists(vid_t vid, bool is_in_graph);
    bool compact_all_adjlists();
    void print_edgeshard(){
        edge_shard->print_edgesum_per_thread();
    }
};

/* -------------------------------------------------------------- */
// Init elog and graph
inline void levelgraph_t::init_elog(){ 
    m.start_time("1.1  -init_alloc_elog");
    elog = new edgelog_t(); 
    alloc_elog_data();
    m.stop_time("1.1  -init_alloc_elog");
}
inline void levelgraph_t::free_elog(){ 
    if(elog->data) free_elog_data();
    delete elog; 
}
inline void levelgraph_t::reset_elog(char* buf, index_t count){ 
    if(elog) free_elog();
    elog = new edgelog_t((edge_t*)buf, count); 
}

inline void levelgraph_t::init_graph(){
    m.start_time("1.2  -init_alloc_graph");
    pblk_pools = 0;
    vbuf_pool = 0;
    vert_pool = 0;
    snap_pool = 0;
    alloc_pblk_pools();
    if(MEMPOOL) alloc_vbuf_pool();
    if(VERT_INPM && !DRAMONLY) alloc_vert_pool();
    if(SNAP_INPM && !DRAMONLY) alloc_snap_pool();

    edge_shard = new edge_shard_t(elog, m);
    out_graph = new graph_t(false, nverts, vbuf_pool, pblk_pools, vert_pool, snap_pool, m);
    in_graph = new graph_t(true, nverts, vbuf_pool, pblk_pools, vert_pool, snap_pool, m);
    create_threads(); // create thread to monitor for archive
    is_finished = false;
    snap_id = 0;
    m.stop_time("1.2  -init_alloc_graph");
}
inline void levelgraph_t::free_graph(){
    pthread_cancel(snap_thread);
    delete out_graph;
    delete in_graph;
    delete edge_shard;
    
    if(SNAP_INPM) free_snap_pool();
    if(VERT_INPM) free_vert_pool();
    if(MEMPOOL) free_vbuf_pool();
    free_pblk_pools();
}

/* -------------------------------------------------------------- */
// alloc and free elog and mempools
inline void levelgraph_t::alloc_elog_data(){
    elog->capacity = ELOG_CAP;
    size_t size =  elog->capacity * sizeof(edge_t);
    if(DRAMONLY == 0){
        std::string elog_file = NVMPATH0 + "edge_log.txt";
        elog->data = (edge_t*)pmem_alloc(elog_file.c_str(), size);
        // memset(elog->data, 0, size); // 没有 memset 后面加载反而更快
        logstream(LOG_DEBUG) << "Edgelog alloced " << elog->capacity << " * " << sizeof(edge_t) << " = " << (size >> 20) << "MB on PMEM, tid = " << omp_get_thread_num() << std::endl;
        elog_size = size; // used for count memory usage
    } else {
        elog->data = (edge_t*)malloc(size);
        logstream(LOG_DEBUG) << "Edgelog alloced " << elog->capacity << " * " << sizeof(edge_t) << " = " << (size >> 20) << "MB on DRAM, tid = " << omp_get_thread_num() << std::endl;
        elog_size = size; // used for count memory usage
    }
}
inline void levelgraph_t::free_elog_data(){
    if(DRAMONLY == 0){
        size_t size = ELOG_CAP * sizeof(edge_t);
        pmem_unmap(elog->data, size);
    } else {
        free(elog->data);
    }
    elog->data = 0;
    // logstream(LOG_DEBUG) << "Edgelog freed " << (size >> 20) << "MB, tid = " << omp_get_thread_num() << std::endl;
}

inline void levelgraph_t::alloc_pblk_pools(){
    pblk_pools = (mempool_t**)calloc(sizeof(mempool_t*), NUM_SOCKETS);
    
    size_t size = PBLK_POOL_SIZE / NUM_SOCKETS;
    char* start;
    // alloc for NVM mempool 0
    if(DRAMONLY == 0){
        std::string mempool_file0 = NVMPATH0 + "pblk_pools.txt";
        start = (char*)pmem_alloc(mempool_file0.c_str(), size);
        logstream(LOG_WARNING) << "pblk_mempool alloced " << size/GB << "GB on PMEM,  tid = " << omp_get_thread_num() << std::endl;
    } else {
        start = (char*)malloc(size);
        assert(start);
        logstream(LOG_WARNING) << "pblk_mempool alloced " << size/GB << "GB on DRAM,  tid = " << omp_get_thread_num() << std::endl;
    }
    pblk_pools[0] = new mempool_t();
    pblk_pools[0]->init(start, size, XPLINE_SIZE, "pblk_pool_0");
    
    if(NUM_SOCKETS > 1){
        // alloc for NVM mempool 1
        if(DRAMONLY == 0){
            std::string mempool_file1 = NVMPATH1 + "pblk_pools.txt";
            start = (char*)pmem_alloc(mempool_file1.c_str(), size);
            logstream(LOG_WARNING) << "pblk_mempool alloced " << size/GB << "GB on PMEM,  tid = " << omp_get_thread_num() << std::endl;
        } else {
            start = (char*)malloc(size);
            assert(start);
            logstream(LOG_WARNING) << "pblk_mempool alloced " << size/GB << "GB on DRAM,  tid = " << omp_get_thread_num() << std::endl;
        }
        pblk_pools[1] = new mempool_t();
        pblk_pools[1]->init(start, size, XPLINE_SIZE, "pblk_pool_1");
    }
    // FIRST_BLOCK_MARK = (index_t)pblk_pool0->get_base();
}
inline void levelgraph_t::free_pblk_pools(){
    size_t size = PBLK_POOL_SIZE / NUM_SOCKETS;
    if(NUM_SOCKETS > 1){
        pblk_pools[1]->clear();
        if(DRAMONLY == 0){
            pmem_unmap(pblk_pools[1]->get_base(), size);
        } else {
            free(pblk_pools[1]->get_base());
        }
        delete pblk_pools[1];
    }
    pblk_pools[0]->clear();
    if(DRAMONLY == 0){
        pmem_unmap(pblk_pools[0]->get_base(), size);
    } else {
        free(pblk_pools[0]->get_base());
    }
    delete pblk_pools[0];
    free(pblk_pools);
}

inline void levelgraph_t::alloc_vbuf_pool(){
    // alloc DRAM mempool for vertex buffers
    index_t vbuf_size = VBUF_POOL_SIZE;
    char* start = (char*)malloc(vbuf_size);
    vbuf_pool = new mempool_t();
    vbuf_pool->init(start, vbuf_size, CACHE_LINE_SIZE, "vbuf_pool");
    logstream(LOG_DEBUG) << "vbuf_pool alloced " << vbuf_size / GB << "GB on DRAM,  tid = " << omp_get_thread_num() << std::endl;
}
inline void levelgraph_t::free_vbuf_pool(){
    vbuf_pool->clear();
    free(vbuf_pool->get_base());
    delete vbuf_pool;
}

inline void levelgraph_t::alloc_vert_pool(){
    // alloc NVM mempool for vertex information
    std::string vertpool_file = NVMPATH0 + "vert_pool.txt";
    index_t vertpool_size = VERT_POOL_SIZE;
    char* vertpool_start = (char*)pmem_alloc(vertpool_file.c_str(), vertpool_size);
    vert_pool = new mempool_t();
    vert_pool->init(vertpool_start, vertpool_size, XPLINE_SIZE, "vert_pool");
    logstream(LOG_WARNING) << "vert_pool alloced " << (vertpool_size >> 30) << "GB on PMEM,  tid = " << omp_get_thread_num() << std::endl;
}
inline void levelgraph_t::free_vert_pool(){
    vert_pool->clear();
    pmem_unmap(vert_pool->get_base(), VERT_POOL_SIZE);
    delete vert_pool;
}

inline void levelgraph_t:: alloc_snap_pool(){
    // alloc NVM mempool for snapshot information
    std::string snappool_file = NVMPATH0 + "snap_pool.txt";
    index_t snappool_size = SNAP_POOL_SIZE;
    char* snappool_start = (char*)pmem_alloc(snappool_file.c_str(), snappool_size);
    snap_pool = new mempool_t();
    snap_pool->init(snappool_start, snappool_size, XPLINE_SIZE, "snap_pool");
    logstream(LOG_WARNING) << "snap_pool alloced " << (snappool_size >> 30) << "GB on PMEM,  tid = " << omp_get_thread_num() << std::endl;
}
inline void levelgraph_t::free_snap_pool(){
    snap_pool->clear();
    pmem_unmap(snap_pool->get_base(), SNAP_POOL_SIZE);
    delete snap_pool;
}

/* -------------------------------------------------------------- */
// write edge log
void levelgraph_t::batch_edge_no_archive(edge_t edge){
    index_t index = __sync_fetch_and_add(&elog->head, 1L);
    index_t index1 = (index & ELOG_MASK);
    elog->data[index1] = edge;
}

inline void levelgraph_t::batch_edge(edge_t edge){
    index_t index = __sync_fetch_and_add(&elog->head, 1L);
    //Check if we are overwritting the unarchived data, if so sleep
    // while (index + 1 - elog->tail > elog->capacity) {
    while (index + 2 - elog->efree > elog->capacity) {
        m.start_time("2.2.2    -sleep_to_wait_archive");
        // logstream(LOG_DEBUG) << "Sleeping for edge log, index + 1 = " << index + 1 << ", elog->efree = " << elog->efree << std::endl; sleep(1);
        usleep(10);
        m.stop_time("2.2.2    -sleep_to_wait_archive");
    }
    // write edge log to elog_head
    index_t index1 = (index & ELOG_MASK);
    elog->data[index1] = edge; // 这里也许可以加入预取优化？

    //inform archive thread if the number of edges reaching the threshold
    index += 1;
    index_t size = ((index - elog->marker) & BATCH_MASK);
    if ((0 == size) && (index != elog->marker)) {
        inform_buffer();
    }
}

/* -------------------------------------------------------------- */
// create_snapshot and archive
bool levelgraph_t::create_snapshot(index_t marker1){
    index_t count = elog->head - elog->marker;
    if(count == 0) return false;
    if(count < BATCH_CAP && !is_finished) return false;
    if(marker1 == 0) marker1 = elog->head;
    elog->marker = marker1;
    count = elog->marker - elog->tail;
    logstream(LOG_INFO) << "creating snapshot " << snap_id << " of range [" << elog->tail << ", " << elog->marker << "), archiving number of edges = " << count << ",  tid = " << omp_get_thread_num()  << std::endl;
    m.start_time("3-classify_and_buffer");
    // #pragma omp parallel num_threads (THD_COUNT) 
    // {
    //     edge_shard->classify_and_buffer_d(snap_id, out_graph, in_graph);
    // }

    m.start_time("3.1-3 -classify");
    // tid_t thd_count = THD_COUNT > 16? 16: THD_COUNT;
    // #pragma omp parallel num_threads (thd_count) 
    // {
    //     edge_shard->classify_store_and_divide(nverts, thd_count);
    // }
    #pragma omp parallel num_threads (THD_COUNT) 
    {
        edge_shard->classify_store_and_divide(nverts, THD_COUNT);
    }
    m.stop_time("3.1-3 -classify");
    m.start_time("3.4 -buffer");
    #pragma omp parallel num_threads (THD_COUNT) 
    {
        edge_shard->classify_and_buffer_d(snap_id, out_graph, in_graph);
    }
    m.stop_time("3.4 -buffer");

    m.stop_time("3-classify_and_buffer");

    // if overwrite comes soon, then flush all buffer to PMEM
    if(marker1 - elog->efree >= BUFFER_CAP){
        logstream(LOG_DEBUG) << "overwriting soon: " << marker1 << " " << elog->efree << " " << BUFFER_CAP << endl;
        if(BATTERY == 0 && DRAMONLY == 0){
            flush_all_bufs(marker1);
        } else {
            elog->efree = marker1;
        }
    }

    // if vbuf_pool fills up soon, then flush all buffer to PMEM
    if(MEMPOOL){
        if(vbuf_pool->get_remained() < 1024 * MB){
            logstream(LOG_DEBUG) << "vbuf_pool fills up soon..., remained: " << vbuf_pool->get_remained()/MB << "MB." << endl;
            flush_all_bufs(marker1);
        }
    } 
    
    elog->tail = marker1;
    snap_id++;
    return true;
}

void levelgraph_t::flush_all_bufs(index_t marker1){
    m.start_time("4-flush_all_bufs");
    logstream(LOG_WARNING) << "flush_all_vbufs start, elog->efree = " << elog->efree << ", marker1 = " <<marker1 << std::endl;
    if (MEMPOOL) {
        if(vbuf_pool->get_used() > vbuf_pool_max_used) vbuf_pool_max_used = vbuf_pool->get_used();
        vbuf_pool->print_usage();
    }
    
    #pragma omp parallel num_threads (THD_COUNT) 
    {
        edge_shard->archive_all_d(out_graph, in_graph);
    }
    if(MEMPOOL) vbuf_pool->clear();
    elog->efree = marker1;
    logstream(LOG_WARNING) << "flush_all_vbufs end, elog->efree = " << elog->efree << ", marker1 = " <<marker1 << std::endl;
    m.stop_time("4-flush_all_bufs");
}

inline void levelgraph_t::inform_buffer(){
    pthread_mutex_lock(&snap_mutex);
    // pthread_cond_signal函数的作用是发送一个信号给另外一个正在处于阻塞等待状态的线程, 使其脱离阻塞状态,继续执行.如果没有线程处在阻塞等待状态,pthread_cond_signal也会成功返回。
    pthread_cond_signal(&snap_condition);
    pthread_mutex_unlock(&snap_mutex);
}

// Wait for finishing buffering all edges (write edges in elog to vbufs). 
// Be careful on why you calling.
inline void levelgraph_t::inform_wait_buffer_all() {
    is_finished = true;
    while (elog->head != elog->tail) {
        inform_buffer();
        // logstream(LOG_DEBUG) << "Sleeping for buf end, head = " << elog->head << ", tail = " << elog->tail << std::endl; sleep(1);
        usleep(1);
    }
    // flush_all_bufs(0);
}

inline degree_t levelgraph_t::get_degree(vid_t vid, bool is_in_graph) {
    if(vid >= nverts){
        logstream(LOG_ERROR) << "Invalid vid, as vid: " << vid << " >= nverts:" << nverts << std::endl;
        return 0;
    }

    degree_t nebr_count = 0;
    if (elog->head > elog->marker){
        // query from edgelog from [marker, head)
        edge_t* edges = elog->data;
        #pragma omp parallel for reduction(+:nebr_count) schedule(static)
        for (index_t i = elog->marker; i < elog->head; ++i) {
            index_t index = i & ELOG_MASK;
            if (!is_in_graph) {
                if (vid == edges[index].src) {
                    nebr_count++;
                }
            } else {
                if (vid == edges[index].dst) {
                    nebr_count++;
                }
            }
        }
    }
    if (!is_in_graph) {
        nebr_count += out_graph->get_degree(vid);
    } else {
        nebr_count += in_graph->get_degree(vid);
    }
    return nebr_count;
}

inline degree_t levelgraph_t::query_nebrs(vid_t* neighbors, vid_t vid, bool is_in_graph) {
    if(vid >= nverts){
        logstream(LOG_ERROR) << "Invalid vid, as vid: " << vid << " >= nverts:" << nverts << std::endl;
        return 0;
    }

    // query from graph (from pblks + vbufs)
    degree_t degree, local_degree = 0;
    if (!is_in_graph) {
        degree = out_graph->get_degree(vid);
        if (degree != 0) {
            local_degree = out_graph->get_nebrs(vid, neighbors);
        }
    } else {
        degree = in_graph->get_degree(vid);
        if (degree != 0) {
            local_degree  = in_graph->get_nebrs(vid, neighbors);
        }
    }
    assert(degree == local_degree);

    // query from edgelog from [marker, head)
    degree += query_nebrs_logged_st(neighbors + local_degree, vid, is_in_graph);

    return degree;
}

// query neighbors from edgelog by multi-threads
inline degree_t levelgraph_t::query_nebrs_logged_mt(vid_t* neighbors, vid_t vid, bool is_in_graph) {
    if(vid >= nverts){
        logstream(LOG_ERROR) << "Invalid vid, as vid: " << vid << " >= nverts:" << nverts << std::endl;
        return 0;
    }

    // query from edgelog from [marker, head) by multi-threads
    degree_t degree = 0;
    if (elog->head > elog->marker){
        edge_t* edges = elog->data;
        #pragma omp parallel 
        {
            vid_t src, dst;
            degree_t d1 = 0;
            #pragma omp for schedule(static) nowait
            for (index_t i = elog->marker; i < elog->head; ++i) {
                index_t index = i & ELOG_MASK;
                src = edges[index].src;
                dst = edges[index].dst;
                if (!is_in_graph) {
                    if (vid == src) {
                        d1 = __sync_fetch_and_add(&degree, 1);
                        neighbors[d1] = dst;
                    }
                } else {
                    if (vid == dst) {
                        d1 = __sync_fetch_and_add(&degree, 1);
                        neighbors[d1] = src;
                    }
                }
            }
        }
    }
    return degree;
}

// query neighbors from edgelog by single-thread
inline degree_t levelgraph_t::query_nebrs_logged_st(vid_t* neighbors, vid_t vid, bool is_in_graph) {
    if(vid >= nverts){
        logstream(LOG_ERROR) << "Invalid vid, as vid: " << vid << " >= nverts:" << nverts << std::endl;
        return 0;
    }

    // query from edgelog from [marker, head) by single thread
    degree_t degree = 0;
    index_t i, index;
    vid_t src, dst;
    if (elog->head > elog->marker){
        edge_t* edges = elog->data;
        for (i = elog->marker; i < elog->head; ++i) {
            index = i & ELOG_MASK;
            src = edges[index].src;
            dst = edges[index].dst;
            if (!is_in_graph) {
                if (vid == src) {
                    neighbors[degree++] = dst;// 线程不安全
                }
            } else {
                if (vid == dst) {
                    neighbors[degree++] = src;
                }
            }
        }
    }
    return degree;
}

inline degree_t levelgraph_t::query_nebrs_buffered(vid_t* neighbors, vid_t vid, bool is_in_graph) {
    if(vid >= nverts){
        logstream(LOG_ERROR) << "Invalid vid, as vid: " << vid << " >= nverts:" << nverts << std::endl;
        return 0;
    }

    degree_t dcount = 0;
    if(!is_in_graph){
        dcount = out_graph->get_nebrs_from_vbuf(vid, neighbors);
    } else {
        dcount = in_graph->get_nebrs_from_vbuf(vid, neighbors);
    }
    return dcount;
}

inline degree_t levelgraph_t::query_nebrs_flushed(vid_t* neighbors, vid_t vid, bool is_in_graph) {
    if(vid >= nverts){
        logstream(LOG_ERROR) << "Invalid vid, as vid: " << vid << " >= nverts:" << nverts << std::endl;
        return 0;
    }

    degree_t pcount = 0;
    if(!is_in_graph){
        pcount = out_graph->get_nebrs_from_pblks(vid, neighbors);
    } else {
        pcount = in_graph->get_nebrs_from_pblks(vid, neighbors);
    }
    return pcount;
}

bool levelgraph_t::compact_adjlists(vid_t vid, bool is_in_graph){
    if(vid >= nverts){
        logstream(LOG_ERROR) << "Invalid vid, as vid: " << vid << " >= nverts:" << nverts << std::endl;
        return 0;
    }

    degree_t degree, local_degree = 0;
    if (!is_in_graph) {
        degree = out_graph->get_degree(vid);
        if (degree != 0) {
            local_degree = out_graph->compact_vbuf_pblks(vid);
        }
    } else {
        degree = in_graph->get_degree(vid);
        if (degree != 0) {
            local_degree  = in_graph->compact_vbuf_pblks(vid);
        }
    }
    assert(degree == local_degree);
    return true;
}


bool levelgraph_t::compact_all_adjlists(){
    out_graph->compact_all_vbuf_pblks();
    in_graph->compact_all_vbuf_pblks();
    return true;
}