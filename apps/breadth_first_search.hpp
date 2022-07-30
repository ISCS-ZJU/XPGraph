#pragma once
#include <omp.h>
#include <random>
#include <algorithm>
#include <libxpgraph.h>

inline void print_bfs_summary(uint8_t* status, uint8_t level, vid_t v_count, vid_t root){
    vid_t sum = 0;
    for (int l = 1; l < level; ++l) {
        vid_t vid_count = 0;
        #pragma omp parallel for reduction (+:vid_count) 
        for (vid_t v = 0; v < v_count; ++v) {
            if (status[v] == l) ++vid_count;
            //if (status[v] == l && l == 3) cout << v << endl;
        }
        sum += vid_count;
        std::cout << " Level = " << l << " count = " << vid_count << std::endl;
    }
    std::cout << " bfs_summary of root " << root << " = " << sum << std::endl;
}

index_t test_bfs(XPGraph* xpgraph, index_t root_count){
    std::cout << "test_bfs..." << std::endl;
    vid_t           v_count    = xpgraph->get_vcount();

    srand(0);
    index_t total_count = 0;
    while(root_count--){
        vid_t i1 = 0, root = 0;
        while (1) {
            root = rand() % v_count;
            if (xpgraph->get_out_degree(root) > 20) break;
            i1++;
            if (i1 >= v_count) {
                root = 0;
                break;
            }
        }

        int				level      = 1;
        int				top_down   = 1;
        vid_t			frontier   = 0;
        
        // double start1 = mywtime();
        uint8_t* status = new uint8_t[v_count];
        status[root] = level;
        
        total_count += 1; 
        do {
            frontier = 0;
            // double start = mywtime();
            #pragma omp parallel reduction(+:frontier)
            {
                vid_t vid;
                degree_t nebr_count = 0;
                degree_t local_degree = 0;
                vid_t* local_adjlist = 0;
                
                if (top_down) {
                    #pragma omp for nowait
                    for (vid_t v = 0; v < v_count; v++) {
                        if (status[v] != level) continue;
                        
                        nebr_count = xpgraph->get_out_degree(v);
                        if (0 == nebr_count) continue;
                        
                        local_adjlist = new vid_t[nebr_count];
                        local_degree  = xpgraph->get_out_nebrs(v, local_adjlist);
                        assert(local_degree  == nebr_count);
                        
                        //traverse the delta adj list
                        for (degree_t i = 0; i < local_degree; ++i) {
                            vid = local_adjlist[i];
                            if (status[vid] == 0) {
                                status[vid] = level + 1;
                                ++frontier;
                            }
                        }
                        delete [] local_adjlist;
                    }
                } else { // bottom up
                    #pragma omp for nowait
                    for (vid_t v = 0; v < v_count; v++) {
                        if (status[v] != 0) continue;

                        nebr_count = xpgraph->get_in_degree(v);
                        if (0 == nebr_count) continue;

                        local_adjlist = new vid_t[nebr_count];
                        local_degree  = xpgraph->get_in_nebrs(v, local_adjlist);
                        assert(local_degree  == nebr_count);

                        for (degree_t i = 0; i < local_degree; ++i) {
                            vid = local_adjlist[i];
                            if (status[vid] == level) {
                                status[v] = level + 1;
                                ++frontier;
                                break;
                            }
                        }
                        delete [] local_adjlist;
                    }
                }
            }
            
            // double end = mywtime();
            // std::cout << "Top down = " << top_down
            //      << " Level = " << level
            //      << " Frontier Count = " << frontier
            //      << " Time = " << end - start
            //      << std::endl;

            // Point is to simulate bottom up bfs, and measure the trade-off    
            if (frontier >= 0.002 * v_count) {
                top_down = false;
            } else {
                top_down = true;
            }
            ++level;
            total_count += frontier;
        } while (frontier);

        print_bfs_summary(status, level, v_count, root);
        delete [] status;
    }
    return total_count;
}

index_t test_bfs_numa(XPGraph* xpgraph, index_t root_count){
    std::cout << "test_bfs_numa..." << std::endl;
    vid_t           v_count    = xpgraph->get_vcount();

    srand(0);
    index_t total_count = 0;
    while(root_count--){
        vid_t i1 = 0, root = 0;
        while (1) {
            root = rand() % v_count;
            if (xpgraph->get_out_degree(root) > 20) break;
            i1++;
            if (i1 >= v_count) {
                root = 0;
                break;
            }
        }

        int				level      = 1;
        int				top_down   = 1;
        vid_t			frontier   = 0;

        // double start1 = mywtime();
        uint8_t* status = new uint8_t[v_count];
        status[root] = level;

        // uint8_t NUM_SOCKETS = 2;
        // tid_t ncores_per_socket = omp_get_max_threads() / NUM_SOCKETS / 2; //24
        
        index_t total_count = 1; 
        do {
            frontier = 0;
            // double start = mywtime();
            #pragma omp parallel reduction(+:frontier)
            {
                vid_t vid;
                degree_t nebr_count = 0;
                degree_t local_degree = 0;
                vid_t* local_adjlist = 0;

                tid_t tid = omp_get_thread_num();

                if (top_down) {
                    // First process vertices in socket 0
                    xpgraph->bind_cpu(tid, 0);
                    #pragma omp for nowait
                    for (vid_t v = 0; v < v_count; v+=2) {
                        if (status[v] != level) continue;
                        
                        nebr_count = xpgraph->get_out_degree(v);
                        if (0 == nebr_count) continue;
                        
                        local_adjlist = new vid_t[nebr_count];
                        local_degree  = xpgraph->get_out_nebrs(v, local_adjlist);
                        assert(local_degree  == nebr_count);
                        
                        //traverse the delta adj list
                        for (degree_t i = 0; i < local_degree; ++i) {
                            vid = local_adjlist[i];
                            if (status[vid] == 0) {
                                status[vid] = level + 1;
                                ++frontier;
                                //cout << " " << sid << endl;
                            }
                        }
                        delete [] local_adjlist;
                    }

                    // Then process vertices in socket 1
                    xpgraph->bind_cpu(tid, 1);
                    #pragma omp for nowait
                    for (vid_t v = 1; v < v_count; v+=2) {
                        if (status[v] != level) continue;
                        
                        nebr_count = xpgraph->get_out_degree(v);
                        if (0 == nebr_count) continue;
                        
                        local_adjlist = new vid_t[nebr_count];
                        local_degree  = xpgraph->get_out_nebrs(v, local_adjlist);
                        assert(local_degree  == nebr_count);
                        
                        //traverse the delta adj list
                        for (degree_t i = 0; i < local_degree; ++i) {
                            vid = local_adjlist[i];
                            if (status[vid] == 0) {
                                status[vid] = level + 1;
                                ++frontier;
                                //cout << " " << sid << endl;
                            }
                        }
                        delete [] local_adjlist;
                    }
                } else { // bottom up
                    // First process vertices in socket 0
                    xpgraph->bind_cpu(tid, 0);
                    #pragma omp for nowait
                    for (vid_t v = 0; v < v_count; v+=2) {
                        if (status[v] != 0) continue;

                        nebr_count = xpgraph->get_in_degree(v);
                        if (0 == nebr_count) continue;

                        local_adjlist = new vid_t[nebr_count];
                        local_degree  = xpgraph->get_in_nebrs(v, local_adjlist);
                        assert(local_degree  == nebr_count);

                        for (degree_t i = 0; i < local_degree; ++i) {
                            vid = local_adjlist[i];
                            if (status[vid] == level) {
                                status[v] = level + 1;
                                ++frontier;
                                break;
                            }
                        }
                        delete [] local_adjlist;
                    }

                    // Then process vertices in socket 1
                    xpgraph->bind_cpu(tid, 1);
                    #pragma omp for nowait
                    for (vid_t v = 1; v < v_count; v+=2) {
                        if (status[v] != 0) continue;

                        nebr_count = xpgraph->get_in_degree(v);
                        if (0 == nebr_count) continue;

                        local_adjlist = new vid_t[nebr_count];
                        local_degree  = xpgraph->get_in_nebrs(v, local_adjlist);
                        assert(local_degree  == nebr_count);

                        for (degree_t i = 0; i < local_degree; ++i) {
                            vid = local_adjlist[i];
                            if (status[vid] == level) {
                                status[v] = level + 1;
                                ++frontier;
                                break;
                            }
                        }
                        delete [] local_adjlist;
                    }
                }
                xpgraph->cancel_bind_cpu();
            }

            // double end = mywtime();
            // std::cout << "Top down = " << top_down
            //      << " Level = " << level
            //      << " Frontier Count = " << frontier
            //      << " Time = " << end - start
            //      << std::endl;

            // Point is to simulate bottom up bfs, and measure the trade-off    
            if (frontier >= 0.002 * v_count) {
                top_down = false;
            } else {
                top_down = true;
            }
            ++level;
            total_count += frontier;
        } while (frontier);

        print_bfs_summary(status, level, v_count, root);
        delete [] status;
    }
    return total_count;
}