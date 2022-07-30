#pragma once
#include <random>
#include <vector>
#include <assert.h>
#include <libxpgraph.h>

using std::vector;

index_t test_2hop(XPGraph* xpgraph, vector<vid_t> query_verts){
    // vid_t v_count = xpgraph->get_vcount();
    // if (query_count == 0) query_count = v_count / 100;
    // vid_t* query = (vid_t*)calloc(sizeof(vid_t), query_count);

    // vector<vid_t> query_verts;
    // index_t i1 = 0;
    // srand(0);
    // while (i1 < query_count) {
    //     vid_t vid = rand() % v_count;
    //     if (xpgraph->get_out_degree(vid) && xpgraph->get_in_degree(vid)){ 
    //         query_verts.push_back(vid);
    //         i1++;
    //     }
    // }

	// double start = mywtime();
    std::cout << "test_2hop..." << std::endl;
    index_t deg_sum = 0;
    vid_t vid, uid;
    degree_t nebr_count1, local_degree1;
    degree_t nebr_count2, local_degree2;
    vid_t* local_adjlist1;
    vid_t* local_adjlist2;
    #pragma omp for schedule (static)
    for(auto it = query_verts.begin(); it != query_verts.end(); ++it){
        vid = *it;
        nebr_count1 = xpgraph->get_out_degree(vid);
        if (0 == nebr_count1) continue;
        local_adjlist1 = new vid_t[nebr_count1];
        local_degree1  = xpgraph->get_out_nebrs(vid, local_adjlist1);
        assert(local_degree1  == nebr_count1);

        for (index_t j = 0; j < nebr_count1; ++j) {
            uid = local_adjlist1[j];
            nebr_count2 = xpgraph->get_out_degree(uid);
            if (0 == nebr_count2) continue;
            local_adjlist2 = new vid_t[nebr_count2];
            local_degree2  = xpgraph->get_out_nebrs(uid, local_adjlist2);
            assert(local_degree2  == nebr_count2);
            deg_sum += nebr_count2;
            delete [] local_adjlist2;
        }
        delete [] local_adjlist1;
    }
    return deg_sum;

    // double end = mywtime();
    // std::string statistic_filename = "xpgraph_query.csv";
    // std::ofstream ofs;
    // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    // ofs << "test_2hop for " << query_count << " vertices, sum of their 2-hop neighbors = " << deg_sum << ", 2 Hop Time = " << end - start << std::endl;
    // ofs.close();
    // // std::cout << "test_2hop for " << query_count << " vertices, sum of their 2-hop neighbors = " << deg_sum << ", 2 Hop Time = " << end - start << std::endl;
    // return end - start;
}

class hop2_t {
 public:
     vid_t vid;
     degree_t d;
     vid_t* vlist;
};

double test_2hop_gone(XPGraph* xpgraph, vector<vid_t> query_verts) {
    // vid_t v_count = xpgraph->get_vcount();
    // if (query_count == 0) query_count = v_count / 100;
    // hop2_t* query = (hop2_t*)calloc(sizeof(hop2_t), query_count);

    // vector<hop2_t> queries;
    // index_t i1 = 0;
    // srand(0);
    // while (i1 < query_count) {
    //     vid_t vid = rand() % v_count;
    //     if (xpgraph->get_out_degree(vid) && xpgraph->get_in_degree(vid)){ 
    //         hop2_t vert;
    //         vert.vid = vid;
    //         query_verts.push_back(vert);
    //         i1++;
    //     }
    // }
	// double start = mywtime();

    std::cout << "test_2hop..." << std::endl;
    index_t query_count = query_verts.size();
    hop2_t* query = (hop2_t*)calloc(sizeof(hop2_t), query_count);
    for (index_t q = 0; q < query_count; ++q) {
        query[q].vid = query_verts[q];
    }

    index_t     sum = 0;
    index_t    sum1 = 0;
    index_t    sum2 = 0;

    degree_t nebr_count = 0, local_degree = 0;
    vid_t v = 0;
    vid_t* local_adjlist = 0;
    degree_t d = 0;
    vid_t* vlist = 0;
    
    for (index_t q = 0; q < query_count; ++q) {
        d = 0;
        v = query[q].vid;
        nebr_count = xpgraph->get_out_degree(v);
        if (nebr_count == 0) continue;

        vlist = (vid_t*)calloc(sizeof(vid_t), nebr_count);
        query[q].vlist = vlist;

        local_adjlist = new vid_t[nebr_count];
        local_degree  = xpgraph->get_out_nebrs(v, local_adjlist);
        for (degree_t i = 0; i < local_degree; ++i) {
            vlist[d] = local_adjlist[i];
            ++d;
        }
        query[q].d += d;
        delete [] local_adjlist;

        //Second hop------------------
        sum = 0;
        sum1 = 0;
        #pragma omp parallel
        {
            degree_t        nebr_count = 0;
            degree_t      local_degree = 0;
            vid_t v = 0;
            vid_t* local_adjlist = 0;
            vid_t* vlist = 0;
            degree_t d = 0;
            
            vlist = query[q].vlist;
            d = query[q].d;

            #pragma omp for schedule (static) reduction(+:sum) nowait
            for (degree_t j = 0; j < d; ++j) {
                v = vlist[j];
                nebr_count = xpgraph->get_out_degree(v);
                if (nebr_count == 0) continue;
                local_adjlist = new vid_t[nebr_count];
                local_degree  = xpgraph->get_out_nebrs(v, local_adjlist);
                for (degree_t i = 0; i < local_degree; ++i) {
                    sum += local_adjlist[i];
                }
                delete [] local_adjlist;
            }
        }
        sum2 += sum1;
        sum2 += sum; 
    }
    free(query);
    return sum2;

    // double end = mywtime();
    // std::string statistic_filename = "xpgraph_query.csv";
    // std::ofstream ofs;
    // ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    // ofs << "test_2hop for " << query_count << " vertices, sum of their 2-hop neighbors = " << sum2 << ", 2 Hop Time = " << end - start << std::endl;
    // ofs.close();
    // // std::cout << "test_2hop for " << query_count << " vertices, sum of their 2-hop neighbors = " << deg_sum << ", 2 Hop Time = " << end - start << std::endl;
    // return end - start;
}

double test_2hop_gone_numa(XPGraph* xpgraph, vector<vector<vid_t> > query_verts) {
    std::cout << "test_2hop_numa..." << std::endl;
    uint8_t NUM_SOCKETS = query_verts.size();
    hop2_t** query = (hop2_t**)calloc(sizeof(hop2_t*), NUM_SOCKETS);
    for(uint8_t sid = 0; sid < NUM_SOCKETS; sid++){
        index_t query_count = query_verts[sid].size();
        query[sid] = (hop2_t*)calloc(sizeof(hop2_t), query_count);
        for (index_t q = 0; q < query_count; ++q) {
            query[sid][q].vid = query_verts[sid][q];
        }
    }

    index_t     sum = 0;
    index_t    sum1 = 0;
    index_t    sum2 = 0;

    degree_t nebr_count = 0, local_degree = 0;
    vid_t v = 0;
    vid_t* local_adjlist = 0;
    degree_t d = 0;
    vid_t* vlist = 0;

    for(uint8_t sid = 0; sid < NUM_SOCKETS; sid++){
        index_t query_count = query_verts[sid].size();
    
        for (index_t q = 0; q < query_count; ++q) {
            d = 0;
            v = query[sid][q].vid;
            nebr_count = xpgraph->get_out_degree(v);
            if (nebr_count == 0) continue;

            vlist = (vid_t*)calloc(sizeof(vid_t), nebr_count);
            query[sid][q].vlist = vlist;

            local_adjlist = new vid_t[nebr_count];
            local_degree  = xpgraph->get_out_nebrs(v, local_adjlist);
            for (degree_t i = 0; i < local_degree; ++i) {
                vlist[d] = local_adjlist[i];
                ++d;
            }
            query[sid][q].d += d;
            delete [] local_adjlist;

            //Second hop------------------
            sum = 0;
            sum1 = 0;
            #pragma omp parallel
            {
                // tid_t tid = omp_get_thread_num();
                // xpgraph->bind_cpu(tid, sid);

                degree_t        nebr_count = 0;
                degree_t      local_degree = 0;
                vid_t v = 0;
                vid_t* local_adjlist = 0;
                vid_t* vlist = 0;
                degree_t d = 0;
                
                vlist = query[sid][q].vlist;
                d = query[sid][q].d;

                #pragma omp for schedule (static) reduction(+:sum) nowait
                for (degree_t j = 0; j < d; ++j) {
                    v = vlist[j];
                    nebr_count = xpgraph->get_out_degree(v);
                    if (nebr_count == 0) continue;
                    local_adjlist = new vid_t[nebr_count];
                    local_degree  = xpgraph->get_out_nebrs(v, local_adjlist);
                    for (degree_t i = 0; i < local_degree; ++i) {
                        sum += local_adjlist[i];
                    }
                    delete [] local_adjlist;
                }

                // xpgraph->cancel_bind_cpu();
            }
            sum2 += sum1;
            sum2 += sum; 
        }
        free(query[sid]);
    }
    free(query);
    return sum2;
}
