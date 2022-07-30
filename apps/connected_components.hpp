#pragma once
#include <omp.h>
#include <random>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cinttypes>
#include <functional>
#include "graph/levelgraph.hpp"

bool compare_and_swap(vid_t &x, vid_t &old_val, vid_t &new_val) {
    return __sync_bool_compare_and_swap(&x, old_val, new_val);
}

// Returns k pairs with largest values from list of key-value pairs
template<typename KeyT, typename ValT>
std::vector<std::pair<ValT, KeyT>> TopK(
    const std::vector<std::pair<KeyT, ValT>> &to_sort, size_t k) {
    std::vector<std::pair<ValT, KeyT>> top_k;
    ValT min_so_far = 0;
    for (auto kvp : to_sort) {
        if ((top_k.size() < k) || (kvp.second > min_so_far)) {
            top_k.push_back(std::make_pair(kvp.second, kvp.first));
            std::sort(top_k.begin(), top_k.end(),
                        std::greater<std::pair<ValT, KeyT>>());
            if (top_k.size() > k)
                top_k.resize(k);
            min_so_far = top_k.back().first;
        }
    }
  return top_k;
}

// Place nodes u and v in same component of lower component ID
void Link(vid_t u, vid_t v, vid_t* comp) {
  vid_t p1 = comp[u];
  vid_t p2 = comp[v];
  while (p1 != p2) {
    vid_t high = p1 > p2 ? p1 : p2;
    vid_t low = p1 + (p2 - high);
    vid_t p_high = comp[high];
    // Was already 'low' or succeeded in writing 'low'
    if ((p_high == low) ||
        (p_high == high && compare_and_swap(comp[high], high, low)))
      break;
    p1 = comp[comp[high]];
    p2 = comp[low];
  }
}

// Reduce depth of tree for each component to 1 by crawling up parents
void Compress(vid_t v_count, vid_t* comp) {
    #pragma omp parallel for schedule(dynamic, 16384)
    for (vid_t n = 0; n < v_count; n++) {
        while (comp[n] != comp[comp[n]]) {
            comp[n] = comp[comp[n]];
        }
    }
}

vid_t SampleFrequentElement(vid_t* comp, vid_t v_count, index_t num_samples = 1024) {
  std::unordered_map<vid_t, int> sample_counts(32);
  using kvp_type = std::unordered_map<vid_t, int>::value_type;
  // Sample elements from 'comp'
  std::mt19937 gen;
  std::uniform_int_distribution<vid_t> distribution(0, v_count - 1);
  for (vid_t i = 0; i < num_samples; i++) {
    vid_t n = distribution(gen);
    sample_counts[comp[n]]++;
  }
  // Find most frequent element in samples (estimate of most frequent overall)
  auto most_frequent = std::max_element(
    sample_counts.begin(), sample_counts.end(),
    [](const kvp_type& a, const kvp_type& b) { return a.second < b.second; });
  float frac_of_graph = static_cast<float>(most_frequent->second) / num_samples;
  std::cout
    << "Skipping largest intermediate component (ID: " << most_frequent->first
    << ", approx. " << static_cast<int>(frac_of_graph * 100)
    << "% of the graph)" << std::endl;
  return most_frequent->first;
}

double test_connected_components(levelgraph_t* levelgraph, metrics &m, index_t neighbor_rounds = 2) {
    std::cout << "test_connected_components..." << std::endl;
    vid_t v_count = levelgraph->get_vcount();
    graph_t* out_graph = levelgraph->get_out_graph();
    graph_t* in_graph = levelgraph->get_in_graph();
    vid_t* comp = (vid_t*)calloc(sizeof(vid_t), v_count);
    
    double start = mywtime();

    // Initialize each node to a single-node self-pointing tree
    #pragma omp parallel for
    for (vid_t v = 0; v < v_count; v++) {
        comp[v] = v;
    }

    // Process a sparse sampled subgraph first for approximating components.
    // Sample by processing a fixed number of neighbors for each node (see paper)
    
    for (index_t r = 0; r < neighbor_rounds; ++r) {
        #pragma omp parallel for schedule(dynamic,16384)
        for (vid_t u = 0; u < v_count; u++) {
            degree_t nebr_count, local_degree;
            vid_t* local_adjlist;
            nebr_count = out_graph->get_degree(u);
            if (nebr_count == 0) continue;
            local_adjlist = new vid_t[nebr_count];
            local_degree  = out_graph->get_nebrs(u, local_adjlist);
            assert(nebr_count == local_degree);
            for (vid_t v = r; v < local_degree; v++) {
                Link(u, local_adjlist[v], comp);
                break;
            }
            delete [] local_adjlist;
        }
        Compress(v_count, comp);
    }

    vid_t c = SampleFrequentElement(comp, v_count);

    #pragma omp parallel for schedule(dynamic, 16384)
    for (vid_t u = 0; u < v_count; u++) {
        degree_t nebr_count, local_degree;
        vid_t* local_adjlist;
        if (comp[u] == c) continue;
        nebr_count = out_graph->get_degree(u);
        if (nebr_count != 0) {
            local_adjlist = new vid_t[nebr_count];
            local_degree  = out_graph->get_nebrs(u, local_adjlist);
            assert(nebr_count == local_degree);
            for (vid_t v = neighbor_rounds; v < local_degree; v++) {
                Link(u, local_adjlist[v], comp);
            }
            delete [] local_adjlist;
        }
        // To support directed graphs, process reverse graph completely
        nebr_count = in_graph->get_degree(u);
        if (nebr_count == 0) continue;
        local_adjlist = new vid_t[nebr_count];
        local_degree  = in_graph->get_nebrs(u, local_adjlist);
        assert(nebr_count == local_degree);
        for (vid_t v = 0; v < local_degree; v++) {
            Link(u, local_adjlist[v], comp);
        }
        delete [] local_adjlist;
    }                                                                                                              
    // Finally, 'compress' for final convergence
    Compress(v_count, comp);

    double end = mywtime();
    std::string statistic_filename = "xpgraph_query.csv";
    std::ofstream ofs;
    ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    ofs << "CC Time = " << end - start << std::endl;
    ofs << std::endl;
    ofs.close();

    cout << endl;
    std::unordered_map<vid_t, vid_t> count;
    for (vid_t comp_i = 0; comp_i < v_count; comp_i++)
        count[comp[comp_i]] += 1;
    int k = 5;
    std::vector<std::pair<vid_t, vid_t>> count_vector;
    count_vector.reserve(count.size());
    for (auto kvp : count) count_vector.push_back(kvp);
    std::vector<std::pair<vid_t, vid_t>> top_k = TopK(count_vector, k);
    k = std::min(k, static_cast<int>(top_k.size()));
    cout << k << " biggest clusters" << endl;
    for (auto kvp : top_k)
        cout << kvp.second << ":" << kvp.first << endl;
    cout << "There are " << count.size() << " components" << endl;

    free(comp);
    return end - start;
}

double test_connected_components_numa(levelgraph_t* levelgraph, metrics &m, index_t neighbor_rounds = 2) {
    std::cout << "test_connected_components_numa..." << std::endl;
    vid_t v_count = levelgraph->get_vcount();
    graph_t* out_graph = levelgraph->get_out_graph();
    graph_t* in_graph = levelgraph->get_in_graph();
    vid_t* comp = (vid_t*)calloc(sizeof(vid_t), v_count);
    
    double start = mywtime();

    // Initialize each node to a single-node self-pointing tree
    #pragma omp parallel for
    for (vid_t v = 0; v < v_count; v++) {
        comp[v] = v;
    }

    // Process a sparse sampled subgraph first for approximating components.
    // Sample by processing a fixed number of neighbors for each node (see paper)
    tid_t ncores_per_socket = omp_get_max_threads() / NUM_SOCKETS / 2; //24
    for (index_t r = 0; r < neighbor_rounds; ++r) {
        #pragma omp parallel 
        {
            tid_t tid = omp_get_thread_num();
            for(int id = 0; id < NUM_SOCKETS; ++id){ // query from numa id
                // if(tid >= 0 && tid < 24 || (tid >= 48 && tid < 72)){ // socket0
                // if(tid >= 24 && tid < 48 || (tid >= 72 && tid < 96)){ // socket1
                if((tid >= ncores_per_socket*id && tid < ncores_per_socket*(id+1)) 
                || (tid >= ncores_per_socket*NUM_SOCKETS + ncores_per_socket*id && tid < ncores_per_socket*NUM_SOCKETS + ncores_per_socket*(id+1))){
                    bind_thread_to_socket(tid, id);

                    #pragma omp for schedule(dynamic,16384) nowait
                    for (vid_t u = id; u < v_count; u+=NUM_SOCKETS) {
                        degree_t nebr_count, local_degree;
                        vid_t* local_adjlist;
                        nebr_count = out_graph->get_degree(u);
                        if (nebr_count == 0) continue;
                        local_adjlist = new vid_t[nebr_count];
                        local_degree  = out_graph->get_nebrs(u, local_adjlist);
                        assert(nebr_count == local_degree);
                        for (vid_t v = r; v < local_degree; v++) {
                            Link(u, local_adjlist[v], comp);
                            break;
                        }
                        delete [] local_adjlist;
                    }
                }
            }
        }
        Compress(v_count, comp);
    }

    vid_t c = SampleFrequentElement(comp, v_count);

    #pragma omp parallel 
    {
        tid_t tid = omp_get_thread_num();
        for(int id = 0; id < NUM_SOCKETS; ++id){ // query from numa id
            // if(tid >= 0 && tid < 24 || (tid >= 48 && tid < 72)){ // socket0
            // if(tid >= 24 && tid < 48 || (tid >= 72 && tid < 96)){ // socket1
            if((tid >= ncores_per_socket*id && tid < ncores_per_socket*(id+1)) 
            || (tid >= ncores_per_socket*NUM_SOCKETS + ncores_per_socket*id && tid < ncores_per_socket*NUM_SOCKETS + ncores_per_socket*(id+1))){
                bind_thread_to_socket(tid, id);

                #pragma omp for schedule(dynamic, 16384) nowait
                for (vid_t u = id; u < v_count; u+=NUM_SOCKETS) {
                    degree_t nebr_count, local_degree;
                    vid_t* local_adjlist;
                    if (comp[u] == c) continue;
                    nebr_count = out_graph->get_degree(u);
                    if (nebr_count != 0) {
                        local_adjlist = new vid_t[nebr_count];
                        local_degree  = out_graph->get_nebrs(u, local_adjlist);
                        assert(nebr_count == local_degree);
                        for (vid_t v = neighbor_rounds; v < local_degree; v++) {
                            Link(u, local_adjlist[v], comp);
                        }
                        delete [] local_adjlist;
                    }
                    // To support directed graphs, process reverse graph completely
                    nebr_count = in_graph->get_degree(u);
                    if (nebr_count == 0) continue;
                    local_adjlist = new vid_t[nebr_count];
                    local_degree  = in_graph->get_nebrs(u, local_adjlist);
                    assert(nebr_count == local_degree);
                    for (vid_t v = 0; v < local_degree; v++) {
                        Link(u, local_adjlist[v], comp);
                    }
                    delete [] local_adjlist;
                }
            }
        }
    }

    // Finally, 'compress' for final convergence
    Compress(v_count, comp);

    double end = mywtime();
    std::string statistic_filename = "xpgraph_query.csv";
    std::ofstream ofs;
    ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    ofs << "CC Time = " << end - start << std::endl;
    ofs << std::endl;
    ofs.close();

    cout << endl;
    std::unordered_map<vid_t, vid_t> count;
    for (vid_t comp_i = 0; comp_i < v_count; comp_i++)
        count[comp[comp_i]] += 1;
    int k = 5;
    std::vector<std::pair<vid_t, vid_t>> count_vector;
    count_vector.reserve(count.size());
    for (auto kvp : count) count_vector.push_back(kvp);
    std::vector<std::pair<vid_t, vid_t>> top_k = TopK(count_vector, k);
    k = std::min(k, static_cast<int>(top_k.size()));
    cout << k << " biggest clusters" << endl;
    for (auto kvp : top_k)
        cout << kvp.second << ":" << kvp.first << endl;
    cout << "There are " << count.size() << " components" << endl;

    free(comp);
    return end - start;
}

