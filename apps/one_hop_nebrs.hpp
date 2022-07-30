#pragma once
#include <random>
#include <vector>
#include <assert.h>
#include <libxpgraph.h>

using std::vector;

degree_t query_vert_nebrs(XPGraph* xpgraph, vid_t vid){
    degree_t nebr_count, local_degree, deg_sum = 0;
    vid_t* local_adjlist;

    // query out-neighbors
    nebr_count = xpgraph->get_out_degree(vid);
    // std::cout << vid << ": outd = " << nebr_count << std::endl;
    if (0 == nebr_count) return deg_sum;
    local_adjlist = new vid_t[nebr_count];
    local_degree  = xpgraph->get_out_nebrs(vid, local_adjlist);
    assert(local_degree  == nebr_count);
    deg_sum += nebr_count;
    delete [] local_adjlist;

    // query in-neighbors
    nebr_count = xpgraph->get_in_degree(vid);
    // std::cout << vid << ": ind = " << nebr_count << std::endl;
    if (0 == nebr_count) return deg_sum;
    local_adjlist = new vid_t[nebr_count];
    local_degree  = xpgraph->get_in_nebrs(vid, local_adjlist);
    assert(local_degree  == nebr_count);
    deg_sum += nebr_count;
    delete [] local_adjlist;

    return deg_sum;
}

/* ---------------------------------------------------------------------- */
// query out-neighbors and in-neighbors for random selected 'query_count' non-zero degree vertices
index_t test_1hop(XPGraph* xpgraph, vector<vid_t> query_verts) {
    // vid_t v_count = xpgraph->get_vcount();
    // if (query_count == 0) query_count = v_count / 10;
    // vector<vid_t> query_verts = root_generator(xpgraph, query_count);

	// double start = mywtime();
    std::cout << "test_1hop..." << std::endl;
    index_t deg_sum = 0;

    #pragma omp parallel
    {
        #pragma omp for reduction(+:deg_sum) schedule (static) nowait
        for(auto it = query_verts.begin(); it != query_verts.end(); ++it){
            vid_t vid = *it;
            deg_sum += query_vert_nebrs(xpgraph, vid);
        }
    }

    return deg_sum;

    // double end = mywtime();

    // std::string statistic_filename = "xpgraph_query.csv";
    // std::ofstream ofs;
    // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    // ofs << "test_1hop for " << query_count << " vertices, sum of their 1-hop neighbors = " << deg_sum << ", 1 Hop Time = " << end - start << std::endl;
    // ofs.close();
    // // std::cout << "test_1hop for " << query_count << " vertices, sum of their 1-hop neighbors = " << deg_sum << ", 1 Hop Time = " << end - start << std::endl;
    // return end - start;
}

index_t test_1hop_numa(XPGraph* xpgraph, vector< vector<vid_t> > query_verts) {
    // vid_t v_count = xpgraph->get_vcount();
    // if (query_count == 0) query_count = v_count / 10;

    // vector< vector<vid_t> > query_verts = root_generator_numa(xpgraph, query_count);
	// double start = mywtime();
    std::cout << "test_1hop_numa..." << std::endl;
    index_t deg_sum = 0;

    #pragma omp parallel
    {
        tid_t tid = omp_get_thread_num();
        xpgraph->bind_cpu(tid, 0);
        #pragma omp for reduction(+:deg_sum) schedule (static) nowait
        for(auto it = query_verts[0].begin(); it != query_verts[0].end(); ++it){
            vid_t vid = *it;
            deg_sum += query_vert_nebrs(xpgraph, vid);
        }
        xpgraph->bind_cpu(tid, 1);
        #pragma omp for reduction(+:deg_sum) schedule (static) nowait
        for(auto it = query_verts[1].begin(); it != query_verts[1].end(); ++it){
            vid_t vid = *it;
            deg_sum += query_vert_nebrs(xpgraph, vid);
        }
        xpgraph->cancel_bind_cpu();
    }

    return deg_sum;

    // double end = mywtime();

    // std::string statistic_filename = "xpgraph_query.csv";
    // std::ofstream ofs;
    // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    // ofs << "test_1hop_numa for " << query_count << " vertices, sum of their 1-hop neighbors = " << deg_sum << ", 1 Hop Time = " << end - start << std::endl;
    // ofs.close();
    // // std::cout << "test_1hop for " << query_count << " vertices, sum of their 1-hop neighbors = " << deg_sum << ", 1 Hop Time = " << end - start << std::endl;
    // return end - start;
}

// /* ---------------------------------------------------------------------- */
// // query out-neighbors and in-neighbors for random selected 'query_count' non-zero degree vertices
// double test_1hop_numa0(levelgraph_t* levelgraph, metrics &m, index_t query_count = 0) {
//     std::cout << "test_1hop_numa..." << std::endl;
//     ///

//     index_t deg_sum = 0;

// 	double start = mywtime();
//     tid_t ncores_per_socket = omp_get_max_threads() / NUM_SOCKETS / 2; //24
//     std::cout << "In 1hop_numa: ncores_per_socket = " << (int)ncores_per_socket << std::endl;

//     #pragma omp parallel
//     {
//         for(int id = 0; id < NUM_SOCKETS; ++id){ // query from numa id
//             // if(tid >= 0 && tid < 24 || (tid >= 48 && tid < 72)){ // socket0
//             // if(tid >= 24 && tid < 48 || (tid >= 72 && tid < 96)){ // socket1
//             tid_t tid = omp_get_thread_num();
//             if((tid >= ncores_per_socket*id && tid < ncores_per_socket*(id+1)) 
//             || (tid >= ncores_per_socket*NUM_SOCKETS + ncores_per_socket*id && tid < ncores_per_socket*NUM_SOCKETS + ncores_per_socket*(id+1))){
//                 bind_thread_to_socket(tid, id);
//                 // std::cout << "bind_thread_to_socket: " << (int)tid << " " << (int)id << std::endl;
//                 // printf("bind_thread_to_socket: %d -> %d query_verts[id].size() = %d\n", tid, id, query_verts[id].size());
                
//                 // #pragma omp for reduction(+:deg_sum) schedule (static) nowait
//                 #pragma omp for reduction(+:deg_sum) schedule (dynamic) nowait
//                 for(auto it = query_verts[id].begin(); it != query_verts[id].end(); ++it){
//                     vid_t vid = *it;
//                     // ...
//                 }
//             }
//         }
//     }
// }

// /* ---------------------------------------------------------------------- */
// // query out-neighbors for random selected 'query_count' non-zero degree vertices
// double test_1hop_out(levelgraph_t* levelgraph, metrics &m, index_t query_count = 0) {
//     vid_t v_count = levelgraph->get_vcount();
//     if (query_count == 0) query_count = v_count / 10;
//     graph_t* out_graph = levelgraph->get_out_graph();
//     vid_t* query = (vid_t*)calloc(sizeof(vid_t), query_count);

//     index_t i1 = 0;
//     srand(0);
//     while (i1 < query_count) {
//         query[i1] = rand() % v_count;
//         if (out_graph->get_degree(query[i1]) != 0) { ++i1; };
//     }

//     index_t deg_sum = 0;
//     index_t sum1 = 0;

// 	double start = mywtime();
//     // #pragma omp parallel
//     {
//     vid_t vid;
//     degree_t nebr_count, local_degree;
//     vid_t* local_adjlist;

//     //#pragma omp for reduction(+:deg_sum) schedule (static) nowait
//     for (index_t i = 0; i < query_count; ++i) {
//         vid = query[i];
//         nebr_count = out_graph->get_degree(vid);
//         // if (0 == nebr_count) continue;
                    
//         local_adjlist = new vid_t[nebr_count];
//         local_degree  = out_graph->get_nebrs(vid, local_adjlist);
//         assert(local_degree  == nebr_count);
//         deg_sum += nebr_count;
//         delete [] local_adjlist;
        
//         // on-the-fly snapshots should process this
//         vid_t src, dst;
//         vid = query[i];
//         edgelog_t* elog = levelgraph->get_elog();
//         edge_t* edges = elog->data;
//         if (elog->marker == elog->tail) continue;
//         #pragma omp parallel for reduction(+:sum1) schedule(static)
//         for (index_t j = elog->tail; j < elog->marker; ++j) {
//             index_t index = j & ELOG_MASK;
//             src = edges[index].src;
//             dst = edges[index].dst;
//             if (src == vid) {
//                 nebr_count = out_graph->get_degree(dst);
//                 sum1 += nebr_count;
//             }

//             if (dst == vid) {
//                 nebr_count = out_graph->get_degree(src);
//                 sum1 += nebr_count;
//             }
//         }
//     }
//     }

//     deg_sum += sum1;
//     double end = mywtime();

//     std::string statistic_filename = "xpgraph_query.csv";
//     std::ofstream ofs;
//     ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
//     ofs << "test_1hop for " << query_count << " vertices, sum of their 1-hop neighbors = " << deg_sum << ", 1 Hop Time = " << end - start << std::endl;
//     ofs.close();
//     // std::cout << "test_1hop for " << query_count << " vertices, sum of their 1-hop neighbors = " << deg_sum << ", 1 Hop Time = " << end - start << std::endl;
//     return end - start;
// }



// double test_2hop(levelgraph_t* levelgraph, metrics &m, index_t query_count = 0){
//     vid_t v_count = levelgraph->get_vcount();
//     if (query_count == 0) query_count = v_count / 100;
//     graph_t* out_graph = levelgraph->get_out_graph();
//     vid_t* query = (vid_t*)calloc(sizeof(vid_t), query_count);

//     index_t i1 = 0;
//     srand(0);
//     while (i1 < query_count) {
//         query[i1] = rand() % v_count;
//         if (out_graph->get_degree(query[i1]) != 0) { ++i1; };
//     }

// 	double start = mywtime();
//     index_t deg_sum = 0;
//     vid_t vid, uid;
//     degree_t nebr_count1, local_degree1;
//     degree_t nebr_count2, local_degree2;
//     vid_t* local_adjlist1;
//     vid_t* local_adjlist2;
//     #pragma omp for schedule (static)
//     for (index_t i = 0; i < query_count; ++i) {
//         vid = query[i];
//         nebr_count1 = out_graph->get_degree(vid);
//         if (0 == nebr_count1) continue;
//         local_adjlist1 = new vid_t[nebr_count1];
//         local_degree1  = out_graph->get_nebrs(vid, local_adjlist1);
//         assert(local_degree1  == nebr_count1);

//         for (index_t j = 0; j < nebr_count1; ++j) {
//             uid = local_adjlist1[j];
//             nebr_count2 = out_graph->get_degree(uid);
//             if (0 == nebr_count2) continue;
//             local_adjlist2 = new vid_t[nebr_count2];
//             local_degree2  = out_graph->get_nebrs(uid, local_adjlist2);
//             assert(local_degree2  == nebr_count2);
//             deg_sum += nebr_count2;
//             delete [] local_adjlist2;
//         }
//         delete [] local_adjlist1;
//     }

//     double end = mywtime();
//     std::string statistic_filename = "xpgraph_query.csv";
//     std::ofstream ofs;
//     ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
//     ofs << "test_2hop for " << query_count << " vertices, sum of their 2-hop neighbors = " << deg_sum << ", 2 Hop Time = " << end - start << std::endl;
//     ofs.close();
//     // std::cout << "test_2hop for " << query_count << " vertices, sum of their 2-hop neighbors = " << deg_sum << ", 2 Hop Time = " << end - start << std::endl;
//     return end - start;
// }

// class hop2_t {
//  public:
//      vid_t vid;
//      degree_t d;
//      vid_t* vlist;
// };

// double test_2hop_gone(levelgraph_t* levelgraph, metrics &m, index_t query_count = 0) {
//     vid_t v_count = levelgraph->get_vcount();
//     if (query_count == 0) query_count = v_count / 100;
//     graph_t* out_graph = levelgraph->get_out_graph();
//     hop2_t* query = (hop2_t*)calloc(sizeof(hop2_t), query_count);

//     index_t i1 = 0;
//     srand(0);
//     while (i1 < query_count) {
//         query[i1].vid = rand() % v_count;
//         if (out_graph->get_degree(query[i1].vid) != 0) { ++i1; };
//     }

// 	double start = mywtime();
//     index_t     sum = 0;
//     index_t    sum1 = 0;
//     index_t    sum2 = 0;

//     degree_t nebr_count = 0, local_degree = 0;
//     vid_t v = 0;
//     vid_t* local_adjlist = 0;
//     degree_t d = 0;
//     vid_t* vlist = 0;
    
//     for (index_t q = 0; q < query_count; ++q) {
//         d = 0;
//         v = query[q].vid;
//         nebr_count = out_graph->get_degree(v);
//         if (nebr_count == 0) continue;

//         vlist = (vid_t*)calloc(sizeof(vid_t), nebr_count);
//         query[q].vlist = vlist;

//         local_adjlist = new vid_t[nebr_count];
//         local_degree  = out_graph->get_nebrs(v, local_adjlist);
//         for (degree_t i = 0; i < local_degree; ++i) {
//             vlist[d] = local_adjlist[i];
//             ++d;
//         }
//         query[q].d += d;
//         delete [] local_adjlist;

//         // on-the-fly snapshots should process this
//         edgelog_t* elog = levelgraph->get_elog();
//         edge_t* edges = elog->data;
//         #pragma omp parallel
//         {
//             vid_t src, dst;
//             degree_t d1 = 0;
//             vid_t v = query[q].vid;
//             vid_t* vlist = query[q].vlist;
//             #pragma omp for schedule(static) nowait 
//             for (index_t i = elog->tail; i < elog->marker; ++i) {
//                 index_t index = i & ELOG_MASK;
//                 src = edges[index].src;
//                 dst = edges[index].dst;
//                 if (src == v) {
//                     d1 = __sync_fetch_and_add(&query[q].d, 1);
//                     vlist[d1] = dst;
//                 }

//                 if (dst == v) {
//                     d1 = __sync_fetch_and_add(&query[q].d, 1);
//                     vlist[d1] = src;
//                 }
//             }
//         }

//         //Second hop------------------
//         sum = 0;
//         sum1 = 0;
//         #pragma omp parallel
//         {
//         degree_t        nebr_count = 0;
//         degree_t      local_degree = 0;
//         vid_t v = 0;
//         vid_t* local_adjlist = 0;
//         vid_t* vlist = 0;
//         degree_t d = 0;
        
//          vlist = query[q].vlist;
//          d = query[q].d;

//         #pragma omp for schedule (static) reduction(+:sum) nowait
//         for (degree_t j = 0; j < d; ++j) {
//             v = vlist[j];
//             nebr_count = out_graph->get_degree(v);
//             if (nebr_count == 0) continue;
//             local_adjlist = new vid_t[nebr_count];
//             local_degree  = out_graph->get_nebrs(v, local_adjlist);
//             for (degree_t i = 0; i < local_degree; ++i) {
//                 sum += local_adjlist[i];
//             }
//             delete [] local_adjlist;
//         }

//         // on-the-fly snapshots should process this
//         vid_t src, dst;
//         edgelog_t* elog = levelgraph->get_elog();
//         edge_t* edges = elog->data;
//         #pragma omp for reduction(+:sum1) schedule(static) nowait
//         for (index_t i = elog->tail; i < elog->marker; ++i) {
//             index_t index = i & ELOG_MASK;
//             src = edges[index].src;
//             dst = edges[index].dst;
//             for (degree_t j = 0; j < d; ++j) {
//                 v = vlist[j];
//                 if (src == v) {
//                     sum1 += dst;
//                 }
//                 if (dst == v) {
//                     sum1 += src;
//                 }
//             }
//         }
//         }
//         sum2 += sum1;
//         sum2 += sum; 
//     }

//     double end = mywtime();
//     free(query);
//     std::string statistic_filename = "xpgraph_query.csv";
//     std::ofstream ofs;
//     ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
//     ofs << "test_2hop for " << query_count << " vertices, sum of their 2-hop neighbors = " << sum2 << ", 2 Hop Time = " << end - start << std::endl;
//     ofs.close();
//     // std::cout << "test_2hop for " << query_count << " vertices, sum of their 2-hop neighbors = " << deg_sum << ", 2 Hop Time = " << end - start << std::endl;
//     return end - start;
// }
