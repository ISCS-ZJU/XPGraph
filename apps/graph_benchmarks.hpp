#pragma once
#include <iostream>
#include <string>
#include <fstream>

#include <libxpgraph.h>
#include "graph_test.hpp"
#include "one_hop_nebrs.hpp"
#include "two_hop_nebrs.hpp"
#include "breadth_first_search.hpp"
#include "pagerank.hpp"
// #include "connected_components.hpp"

#include <sys/time.h>
#include <stdlib.h>
inline double mywtime()
{
	double time[2];	
	struct timeval time1;
	gettimeofday(&time1, NULL);

	time[0]=time1.tv_sec;
	time[1]=time1.tv_usec;

	return time[0] + time[1]*1.0e-6;
}

vector<vid_t> root_generator(XPGraph* xpgraph, index_t query_count){
    vid_t v_count = xpgraph->get_vcount();
    vector<vid_t> query_verts;
    index_t i1 = 0;
    srand(0);
    while (i1 < query_count) {
        vid_t vid = rand() % v_count;
        if (xpgraph->get_out_degree(vid) && xpgraph->get_in_degree(vid)){ 
            query_verts.push_back(vid);
            i1++;
        }
    }
    return query_verts;
}

vector< vector<vid_t> > root_generator_numa(XPGraph* xpgraph, index_t query_count){
    vid_t v_count = xpgraph->get_vcount();
    vector< vector<vid_t> > query_verts;
    query_verts.resize(xpgraph->get_socket_num());
    index_t i1 = 0;
    uint8_t socket_id = 0;
    srand(0);
    while (i1 < query_count) {
        vid_t vid = rand() % v_count;
        if (xpgraph->get_out_degree(vid) && xpgraph->get_in_degree(vid)){ 
            socket_id = xpgraph->get_socket_id(vid);
            query_verts[socket_id].push_back(vid);
            i1++;
        }
    }
    return query_verts;
}

void test_graph_benchmarks(XPGraph* xpgraph){
    std::string statistic_filename = "xpgraph_query.csv";
    std::ofstream ofs;
    ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    ofs << "[QueryTimings]:" ;

    uint8_t count = xpgraph->get_query_count();
    #pragma region test_graph_benchmarks
    while (count--) {
        double start, end; 
        { // test_1hop
            index_t query_count = 1<<24;
            vector<vid_t> query_verts = root_generator(xpgraph, query_count);
            start = mywtime();
            index_t res = test_1hop(xpgraph, query_verts);
            end = mywtime();
            ofs << (end - start) << ",";
            std::cout << "test_1hop for " << query_count << " vertices, sum of their 1-hop neighbors = " << res << ", 1 Hop Time = " << end - start << std::endl;
        }
        { // test_2hop_gone
            index_t query_count = 1<<14;
            vector<vid_t> query_verts = root_generator(xpgraph, query_count);
            start = mywtime();
            index_t res = test_2hop_gone(xpgraph, query_verts);
            end = mywtime();
            ofs << (end - start) << ",";
            std::cout << "test_2hop_gone for " << query_count << " vertices, sum of their 2-hop neighbors = " << res << ", 2 Hop Time = " << end - start << std::endl;
        }
        { // test_bfs
            vid_t root_count = 3;
            start = mywtime();
            index_t res = test_bfs(xpgraph, root_count);
            end = mywtime();
            ofs << (end - start) << ",";
            std::cout << "test_bfs for " << root_count << " root vertices, sum of frontier count = " << res << ", BFS Time = " << end - start << std::endl;
        }
        { // test_pagerank
            index_t num_iterations = 10;
            start = mywtime();
            test_pagerank_pull(xpgraph, num_iterations);
            end = mywtime();
            ofs << (end - start) << ",";
            std::cout << "test_pagerank_pull for " << num_iterations << " iterations, PageRank Time = " << end - start << std::endl;
        }
    }
    #pragma endregion test_graph_benchmarks
    ofs << std::endl;
    ofs.close();
}

void test_graph_benchmarks_numa(XPGraph* xpgraph){
    uint8_t count = xpgraph->get_query_count();
    if(count == 0) return;
    
    std::string statistic_filename = "xpgraph_query.csv";
    std::ofstream ofs;
    ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    ofs << "[QueryTimings]:" ;

    #pragma region test_graph_benchmarks
    while (count--) {
        double start, end; 
        // graph benchmark test with subgraph based NUMA optimization 
        { // test_1hop_numa
            index_t query_count = 1<<24;
            vector< vector<vid_t> > query_verts = root_generator_numa(xpgraph, query_count);
            start = mywtime();
            index_t res = test_1hop_numa(xpgraph, query_verts);
            end = mywtime();
            ofs << (end - start) << ",";
            std::cout << "test_1hop_numa for " << query_count << " vertices, sum of their 1-hop neighbors = " << res << ", 1 Hop Time = " << end - start << std::endl;
        }
        { // test_2hop_gone_numa
            index_t query_count = 1<<14;
            vector< vector<vid_t> > query_verts = root_generator_numa(xpgraph, query_count);
            start = mywtime();
            index_t res = test_2hop_gone_numa(xpgraph, query_verts);
            end = mywtime();
            ofs << (end - start) << ",";
            std::cout << "test_2hop_gone_numa for " << query_count << " vertices, sum of their 2-hop neighbors = " << res << ", 2 Hop Time = " << end - start << std::endl;
        }
        { // test_bfs_numa
            vid_t root_count = 3;
            start = mywtime();
            index_t res = test_bfs_numa(xpgraph, root_count);
            end = mywtime();
            ofs << (end - start) << ",";
            std::cout << "test_bfs_numa for " << root_count << " root vertices, sum of frontier count = " << res << ", BFS Time = " << end - start << std::endl;
        }
        { // test_pagerank_numa
            index_t num_iterations = 10;
            start = mywtime();
            test_pagerank_pull_numa(xpgraph, num_iterations);
            end = mywtime();
            ofs << (end - start) << ",";
            std::cout << "test_pagerank_pull for " << num_iterations << " iterations, PageRank Time = " << end - start << std::endl;
        }
    }
    #pragma endregion test_graph_benchmarks_numa
    ofs << std::endl;
    ofs.close();
}
