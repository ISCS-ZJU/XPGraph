#pragma once
#include <omp.h>
#include <random>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cinttypes>
#include <functional>

#include <sys/mman.h>
#if !defined(MAP_HUGE_2MB)
#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#endif

#include <libxpgraph.h>

// Credits to :
// http://www.memoryhole.net/kyle/2012/06/a_use_for_volatile_in_multithr.html
typedef float rank_t; 
float qthread_dincr(float *operand, float incr)
{
    //*operand = *operand + incr;
    //return incr;
    
    union {
       rank_t   d;
       uint32_t i;
    } oldval, newval, retval;
    do {
         oldval.d = *(volatile rank_t *)operand;
         newval.d = oldval.d + incr;
         //__asm__ __volatile__ ("lock; cmpxchgq %1, (%2)"
         __asm__ __volatile__ ("lock; cmpxchg %1, (%2)"
                                : "=a" (retval.i)
                                : "r" (newval.i), "r" (operand),
                                 "0" (oldval.i)
                                : "memory");
    } while (retval.i != oldval.i);
    return oldval.d;
}

void test_pagerank_pull(XPGraph* xpgraph, int iteration_count) {
    std::cout << "test_pagerank_pull..." << std::endl;
    float* rank_array = 0 ;
	float* prior_rank_array = 0;
    float* dset = 0;
    vid_t v_count = xpgraph->get_vcount();

    // double start = mywtime();

    // m.start_time("5.4.1  -alloc_array");

    rank_array  = (float*)mmap(NULL, sizeof(float)*v_count, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0 );
    if (MAP_FAILED == rank_array) {
        std::cout << "Huge page alloc failed for rank array" << std::endl;
        rank_array = (float*)calloc(v_count, sizeof(float));
    }
    
    prior_rank_array  = (float*)mmap(NULL, sizeof(float)*v_count, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0 );
    if (MAP_FAILED == prior_rank_array) {
        std::cout << "Huge page alloc failed for prior rank array" << std::endl;
        prior_rank_array = (float*)calloc(v_count, sizeof(float));
    }
    
    dset  = (float*)mmap(NULL, sizeof(float)*v_count, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0 );
    if (MAP_FAILED == dset) {
        std::cout << "Huge page alloc failed for dset" << std::endl;
        dset = (float*)calloc(v_count, sizeof(float));
    }

    // m.stop_time("5.4.1  -alloc_array");

    //initialize the rank, and get the degree information
    
    // m.start_time("5.4.2  -get_degree_information");

    #pragma omp parallel
    { 
    degree_t degree = 0;
    float inv_v_count = 0.15; // 1.0f / vert_count;
    #pragma omp for
        for (vid_t v = 0; v < v_count; ++v) {
            degree = xpgraph->get_in_degree(v);
            if (degree != 0) {
                dset[v] = 1.0f / degree;
                prior_rank_array[v] = inv_v_count; // XXX
            } else {
                dset[v] = 0;
                prior_rank_array[v] = 0;
            }
        }
    }

    // m.stop_time("5.4.2  -get_degree_information");

    // let's run the pagerank
    for (int iter_count = 0; iter_count < iteration_count; ++iter_count) {
        // double start1 = mywtime();
        #pragma omp parallel 
        {
            // m.start_time("5.4.3  -do_pagerank");
            #pragma omp for schedule (dynamic, 4096) nowait 
            for (vid_t v = 0; v < v_count; ++v) {
                sid_t uid;
                degree_t nebr_count = 0;
                degree_t local_degree = 0;
                vid_t* local_adjlist;

                float rank = 0.0f; 

                nebr_count = xpgraph->get_in_degree(v);
                if (0 == nebr_count) continue;
                
                local_adjlist = new vid_t[nebr_count];
                local_degree = xpgraph->get_in_nebrs(v, local_adjlist);
                assert(local_degree == nebr_count); // here's the problem
                
                // traverse the delta adj list
                for (index_t j = 0; j < local_degree; ++j){
                    uid = local_adjlist[j];
                    rank += prior_rank_array[uid];
                }
                qthread_dincr(rank_array + v, rank);
                delete [] local_adjlist;
            }
            // m.stop_time("5.4.3  -do_pagerank");

            // m.start_time("5.4.5  -renew_rank_array");
            if (iter_count != iteration_count - 1) {
                #pragma omp for
                for (vid_t v = 0; v < v_count; v++) {
                    rank_array[v] = (0.15 + 0.85 * rank_array[v]) * dset[v];
                    prior_rank_array[v] = 0;
                } 
            } else { 
                #pragma omp for
                for (vid_t v = 0; v < v_count; v++) {
                    rank_array[v] = (0.15 + 0.85 * rank_array[v]);
                    prior_rank_array[v] = 0;
                }
            }
            // m.stop_time("5.4.5  -renew_rank_array");
        }

        std::swap(prior_rank_array, rank_array);
        // double end1 = mywtime();
        // std::string statistic_filename = "xpgraph_query.csv";
        // std::ofstream ofs;
        // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
        // ofs << "Iteration Time = " << end1 - start1 << std::endl;
        // ofs.close();
    }
    // double end = mywtime();

    // std::string statistic_filename = "xpgraph_query.csv";
    // std::ofstream ofs;
    // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    // ofs << "PR Time = " << end - start << std::endl;
    // ofs << std::endl;
    // ofs.close();
    // return end - start;
}

void test_pagerank_push(XPGraph* xpgraph, int iteration_count){
    float* rank_array = 0 ;
	float* prior_rank_array = 0;
    float* dset = 0;
	vid_t v_count = xpgraph->get_vcount();
    
    // double start = mywtime();
    
    rank_array  = (float*)mmap(NULL, sizeof(float)*v_count, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0 );
    if (MAP_FAILED == rank_array) {
        std::cout << "Huge page alloc failed for rank array" << std::endl;
        rank_array = (float*)calloc(v_count, sizeof(float));
    }
    
    prior_rank_array  = (float*)mmap(NULL, sizeof(float)*v_count, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0 );
    if (MAP_FAILED == prior_rank_array) {
        std::cout << "Huge page alloc failed for prior rank array" << std::endl;
        prior_rank_array = (float*)calloc(v_count, sizeof(float));
    }
    
    dset  = (float*)mmap(NULL, sizeof(float)*v_count, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0 );
    
    if (MAP_FAILED == dset) {
        std::cout << "Huge page alloc failed for dset" << std::endl;
        dset = (float*)calloc(v_count, sizeof(float));
    }
	
	//initialize the rank, and get the degree information
    
    #pragma omp parallel
    { 
    degree_t degree = 0;
    float	inv_v_count = 0.15;//1.0f/vert_count;
    #pragma omp for
    for (vid_t v = 0; v < v_count; ++v) {
        degree = xpgraph->get_out_degree(v);
        if (degree != 0) {
            dset[v] = 1.0f/degree;
            prior_rank_array[v] = inv_v_count;//XXX
        } else {
            dset[v] = 0;
            prior_rank_array[v] = 0;
        }
    }
    }

	//let's run the pagerank
	for (int iter_count = 0; iter_count < iteration_count; ++iter_count) {
        // double start1 = mywtime();
        #pragma omp parallel 
        {
            sid_t uid;
            degree_t nebr_count, local_degree;
            vid_t* local_adjlist;
            float rank = 0.0f; 
         
            #pragma omp for schedule (dynamic, 4096) nowait 
            for (vid_t v = 0; v < v_count; v++) {
                nebr_count = xpgraph->get_out_degree(v);
                if (0 == nebr_count) continue;
                local_adjlist = new vid_t[nebr_count];
                local_degree  = xpgraph->get_out_nebrs(v, local_adjlist);
                // assert(local_degree  == nebr_count);
                
                rank = prior_rank_array[v];
                
                //traverse the delta adj list
                for(index_t j = 0; j < local_degree; ++j) {
                    uid = local_adjlist[j];
                    qthread_dincr(rank_array + uid, rank);
                }
                delete [] local_adjlist;
            }

            if (iter_count != iteration_count - 1) {
                #pragma omp for
                for (vid_t v = 0; v < v_count; v++) {
                    rank_array[v] = (0.15 + 0.85 * rank_array[v]) * dset[v];
                    prior_rank_array[v] = 0;
                } 
            } else { 
                #pragma omp for
                for (vid_t v = 0; v < v_count; v++) {
                    rank_array[v] = (0.15 + 0.85 * rank_array[v]);
                    prior_rank_array[v] = 0;
                }
            }
        }
        std::swap(prior_rank_array, rank_array);
        // double end1 = mywtime();
        // std::string statistic_filename = "xpgraph_query.csv";
        // std::ofstream ofs;
        // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
        // ofs << "Iteration Time = " << end1 - start1 << std::endl;
        // ofs.close();
        // // std::cout << "Iteration Time = " << end1 - start1 << std::endl;
    }	
    // double end = mywtime();

    // std::string statistic_filename = "xpgraph_query.csv";
    // std::ofstream ofs;
    // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    // ofs << "PR Time = " << end - start << std::endl;
    // ofs << std::endl;
    // ofs.close();
	// // std::cout << "PR Time = " << end - start << std::endl;
	// // std::cout << std::endl;
    // return end - start;
}

void test_pagerank_pull_numa(XPGraph* xpgraph, int iteration_count){
    std::cout << "test_pagerank_pull_numa..." << std::endl;
    float* rank_array = 0 ;
	float* prior_rank_array = 0;
    float* dset = 0;
    vid_t v_count = xpgraph->get_vcount();

    // double start = mywtime();

    // m.start_time("5.4.1  -alloc_array");

    rank_array  = (float*)mmap(NULL, sizeof(float)*v_count, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0 );
    if (MAP_FAILED == rank_array) {
        std::cout << "Huge page alloc failed for rank array" << std::endl;
        rank_array = (float*)calloc(v_count, sizeof(float));
    }
    
    prior_rank_array  = (float*)mmap(NULL, sizeof(float)*v_count, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0 );
    if (MAP_FAILED == prior_rank_array) {
        std::cout << "Huge page alloc failed for prior rank array" << std::endl;
        prior_rank_array = (float*)calloc(v_count, sizeof(float));
    }
    
    dset  = (float*)mmap(NULL, sizeof(float)*v_count, PROT_READ|PROT_WRITE,
                            MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0 );
    if (MAP_FAILED == dset) {
        std::cout << "Huge page alloc failed for dset" << std::endl;
        dset = (float*)calloc(v_count, sizeof(float));
    }

    // m.stop_time("5.4.1  -alloc_array");

    //initialize the rank, and get the degree information
    
    // m.start_time("5.4.2  -get_degree_information");

    #pragma omp parallel
    { 
        degree_t degree = 0;
        float inv_v_count = 0.15; // 1.0f / vert_count;
        #pragma omp for
        for (vid_t v = 0; v < v_count; ++v) {
            degree = xpgraph->get_in_degree(v);
            if (degree != 0) {
                dset[v] = 1.0f / degree;
                prior_rank_array[v] = inv_v_count; // XXX
            } else {
                dset[v] = 0;
                prior_rank_array[v] = 0;
            }
        }
    }

    // m.stop_time("5.4.2  -get_degree_information");

    uint8_t NUM_SOCKETS = 2;
    tid_t ncores_per_socket = omp_get_max_threads() / NUM_SOCKETS / 2; //24

    // let's run the pagerank
    for (int iter_count = 0; iter_count < iteration_count; ++iter_count) {
        // double start1 = mywtime();
        #pragma omp parallel 
        {
            tid_t tid = omp_get_thread_num();
            for(int id = 0; id < NUM_SOCKETS; ++id){ // query from numa id
                // if(tid >= 0 && tid < 24 || (tid >= 48 && tid < 72)){ // socket0
                // if(tid >= 24 && tid < 48 || (tid >= 72 && tid < 96)){ // socket1
                if((tid >= ncores_per_socket*id && tid < ncores_per_socket*(id+1)) 
                || (tid >= ncores_per_socket*NUM_SOCKETS + ncores_per_socket*id && tid < ncores_per_socket*NUM_SOCKETS + ncores_per_socket*(id+1))){
                    // bind_thread_to_socket(tid, id);
                    xpgraph->bind_cpu(tid, id);
                    
                    // #pragma omp for schedule (static) nowait 
                    #pragma omp for schedule (dynamic, 4096) nowait 
                    for (vid_t v = id; v < v_count; v+=NUM_SOCKETS) {
                        sid_t uid;
                        degree_t nebr_count = 0;
                        degree_t local_degree = 0;
                        vid_t* local_adjlist;

                        float rank = 0.0f; 

                        nebr_count = xpgraph->get_in_degree(v);
                        if (0 == nebr_count) continue;

                        local_adjlist = new vid_t[nebr_count];
                        local_degree = xpgraph->get_in_nebrs(v, local_adjlist);
                        assert(local_degree == nebr_count);

                        // traverse the delta adj list
                        for (index_t j = 0; j < local_degree; ++j){
                            uid = local_adjlist[j];
                            rank += prior_rank_array[uid];
                        }
                        qthread_dincr(rank_array + v, rank);
                        delete [] local_adjlist;
                    }
                } 
                xpgraph->cancel_bind_cpu();
            }

            if (iter_count != iteration_count - 1) {
                #pragma omp for
                for (vid_t v = 0; v < v_count; v++) {
                    rank_array[v] = (0.15 + 0.85 * rank_array[v]) * dset[v];
                    prior_rank_array[v] = 0;
                } 
            } else { 
                #pragma omp for
                for (vid_t v = 0; v < v_count; v++) {
                    rank_array[v] = (0.15 + 0.85 * rank_array[v]);
                    prior_rank_array[v] = 0;
                }
            }
            
        }

        std::swap(prior_rank_array, rank_array);
        // double end1 = mywtime();

        // std::string statistic_filename = "xpgraph_query.csv";
        // std::ofstream ofs;
        // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
        // ofs << "Iteration Time = " << end1 - start1 << std::endl;
        // ofs.close();
    }
    // double end = mywtime();

    // std::string statistic_filename = "xpgraph_query.csv";
    // std::ofstream ofs;
    // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    // ofs << "PR Time = " << end - start << std::endl;
    // ofs << std::endl;
    // ofs.close();
    // return end - start;
}