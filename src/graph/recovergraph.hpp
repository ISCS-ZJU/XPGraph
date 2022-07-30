#pragma once
#include "graph/levelgraph.hpp"

class rblock_t {
public:
    degree_t degree;
    pblock_t* fb_addr;
    pblock_t* lb_addr;

    rblock_t() { degree = 0; fb_addr = 0; lb_addr = 0; }
}; 

void save_graph_pmem(std::string odirname, levelgraph_t *levelgraph) {
    vid_t nverts = levelgraph->get_vcount();
    graph_t* out_graph = levelgraph->get_out_graph();
    graph_t* in_graph = levelgraph->get_in_graph();
    logstream(LOG_INFO) << "Start Saving Graph, nverts = " << nverts << std::endl;

    levelgraph->flush_all_bufs(0);

    rblock_t* rblks_out = (rblock_t*)pmem_alloc((odirname+"out.txt").c_str(), sizeof(rblock_t)*nverts);
    rblock_t* rblks_in = (rblock_t*)pmem_alloc((odirname+"in.txt").c_str(), sizeof(rblock_t)*nverts);

    // memset(rblks_out, 0, sizeof(rblock_t)*nverts);
    // memset(rblks_in, 0, sizeof(rblock_t)*nverts);

    #pragma omp for schedule(static) nowait 
    for (vid_t i = 0; i < nverts; ++i) {
        vertex_t* vert = out_graph->get_vertex(i);
        if (vert != 0) {
            rblks_out[i].fb_addr = vert->get_1st_block();
            rblks_out[i].lb_addr = vert->get_last_block();
            rblks_out[i].degree = out_graph->get_degree(i);    
        } else {
            rblks_out[i].fb_addr = NULL;
            rblks_out[i].lb_addr = NULL;
            rblks_out[i].degree = 0;    
        }
        
        vert = in_graph->get_vertex(i);
        if (vert != 0) {
            rblks_in[i].fb_addr = vert->get_1st_block();
            rblks_in[i].lb_addr = vert->get_last_block();
            rblks_in[i].degree = in_graph->get_degree(i);
        } else {
            rblks_in[i].fb_addr = NULL;
            rblks_in[i].lb_addr = NULL;
            rblks_in[i].degree = 0;    
        }
    }

    FILE* file = fopen((odirname + "info.txt").c_str(), "wb+");
    char* out_paddr_base = levelgraph->get_pblk_pool(0)->get_base();
    char* in_paddr_base = levelgraph->get_pblk_pool(1)->get_base();
    // printf("PMEM Begin Address = %p.\n", out_paddr_base);
    // printf("PMEM Begin Address = %p.\n", in_paddr_base);
    assert(file != 0);
    fwrite(&nverts, sizeof(vid_t), 1, file);
    fwrite(&out_paddr_base, sizeof(char*), 1, file);
    fwrite(&in_paddr_base, sizeof(char*), 1, file);
    fclose(file);
}

void load_graph_pmem(std::string idirname, levelgraph_t *levelgraph) {
    vid_t nverts = levelgraph->get_vcount();
    graph_t* out_graph = levelgraph->get_out_graph();
    graph_t* in_graph = levelgraph->get_in_graph();
    char* new_out_paddr_base = levelgraph->get_pblk_pool(0)->get_base();
    char* new_in_paddr_base = levelgraph->get_pblk_pool(1)->get_base();

    char* out_paddr_base = 0;
    char* in_paddr_base = 0;

    double start, end;
    double read_time, recover_time;

    start = mywtime();
    FILE* file = fopen((idirname + "info.txt").c_str(), "rb");
    assert(file != 0);
    if(!fread(&nverts, sizeof(vid_t), 1, file)) {
        assert(0);
    }
    fseek(file, sizeof(int), 0);
    if(!fread(&out_paddr_base, sizeof(char*), 1, file)) {
        assert(0);
    }
    fseek(file, sizeof(int)+sizeof(char*), 0);
    if(!fread(&in_paddr_base, sizeof(char*), 1, file)) {
        assert(0);
    }
    fclose(file);

    char* bufptr_out = 0;
    char* bufptr_in = 0;

    size_t out_size = alloc_and_read(&bufptr_out, idirname + "out.txt", 1);
    size_t in_size = alloc_and_read(&bufptr_in, idirname + "in.txt", 1);

    logstream(LOG_DEBUG) << "out graph size = " << out_size << " Bytes." << std::endl;
    logstream(LOG_DEBUG) << "in graph size = " << in_size  << " Bytes." << std::endl;

    rblock_t* rblks_out = (rblock_t*)bufptr_out;
    rblock_t* rblks_in = (rblock_t*)bufptr_in;

    end = mywtime();
    read_time = end - start;

    // printf("Old Out PMEM Begin Address = %p.\n", out_paddr_base);
    // printf("Old In PMEM Begin Address = %p.\n", in_paddr_base);
    // printf("New Out PMEM Begin Address = %p.\n", new_out_paddr_base);
    // printf("New In PMEM Begin Address = %p.\n", new_in_paddr_base);

    // new_addr = old_addr + new_base - paddr_base
    offset_t out_offset = (pblock_t*)new_out_paddr_base - (pblock_t*)out_paddr_base;
    offset_t in_offset = (pblock_t*)new_in_paddr_base - (pblock_t*)in_paddr_base;
    // printf("Out Offset = %ld.\n", out_offset);
    // printf("In Offset = %ld.\n", in_offset);

    start = mywtime();
    #pragma omp for schedule(static) nowait 
    for (vid_t i = 0; i < nverts; ++i) {
        // recover out graph
        if (rblks_out[i].degree != 0) {
            vertex_t *vert = out_graph->new_vertex();
            snap_t *vsnap = out_graph->new_snap();
            // recover block
            // pblock_t* cur_blk = rblks_out[i].fb_addr + out_offset;
            vert->set_1st_block(rblks_out[i].fb_addr + out_offset);
            vert->set_last_block(rblks_out[i].lb_addr + out_offset);
            // while (cur_blk && cur_blk->get_next()) {
            //     cur_blk = cur_blk->get_next();
            // }
            // vert->set_last_block(cur_blk);
            // recover degree
            vsnap->degree = rblks_out[i].degree;
            vert->set_snap(vsnap);
            // recover vertex
            out_graph->set_vertex(i, vert);
        }

        // recover in graph
        if (rblks_in[i].degree != 0) {
            vertex_t *vert = in_graph->new_vertex();
            snap_t *vsnap = in_graph->new_snap();
            // recover block
            // pblock_t* cur_blk = rblks_in[i].fb_addr + in_offset;
            vert->set_1st_block(rblks_in[i].fb_addr + in_offset);
            vert->set_last_block(rblks_in[i].lb_addr + in_offset);
            // while (cur_blk && cur_blk->get_next()) {
            //     cur_blk = cur_blk->get_next();
            // }
            // vert->set_last_block(cur_blk);
            // recover degree
            vsnap->degree = rblks_in[i].degree;
            vert->set_snap(vsnap);
            // recover vertex
            in_graph->set_vertex(i, vert);
        }
    }
    end = mywtime();
    recover_time = end - start;

    std::string statistic_filename = "xpgraph_recover.csv";
    std::ofstream ofs;
    ofs.open(statistic_filename.c_str(), std::ofstream::out | std::ofstream::app );
    ofs << "[RecoverTimings]:" << read_time+recover_time 
        << "(" << read_time << "+" << recover_time << ")," << std::endl;
    ofs.close();
}

// class rblock_t {
// public:
//     degree_t degree;
//     pblock_t* fb_addr;
//     pblock_t* lb_addr;

//     rblock_t() { degree = 0; fb_addr = 0; lb_addr = 0; }
// };

// void save_graph(levelgraph_t *levelgraph) {
//     vid_t nverts = levelgraph->get_vcount();
//     graph_t* out_graph = levelgraph->get_out_graph();
//     graph_t* in_graph = levelgraph->get_in_graph();
//     logstream(LOG_INFO) << "Start Saving Graph, nverts = " << nverts << std::endl;

//     levelgraph->flush_all_bufs(0);
//     // elog->efree = elog->head;
    
//     // std::string o_dir = NVMPATH0 + "recovery.txt";
//     // index_t size = sizeof(rblock_t) * nverts;
//     // char* start = (char*)pmem_alloc(o_dir.c_str(), size);
//     // mempool_t* rpool = new mempool_t();
//     // rpool->init(start, size, sizeof(rblock_t), "recovery_pool");

//     // logstream(LOG_DEBUG) << "rblock_t size = " << sizeof(rblock_t) << std::endl;

//     rblock_t* rblks_out = (rblock_t*)calloc(sizeof(rblock_t), nverts);
//     rblock_t* rblks_in = (rblock_t*)calloc(sizeof(rblock_t), nverts);
    
//     #pragma omp for schedule(static) nowait 
//     for (vid_t i = 0; i < nverts; ++i) {
//         vertex_t* vert = out_graph->get_vertex(i);
//         if (vert != 0) {
//             rblks_out[i].fb_addr = vert->get_1st_block();
//             rblks_out[i].lb_addr = vert->get_last_block();
//             rblks_out[i].degree = out_graph->get_degree(i);    
//         }
        
//         vert = in_graph->get_vertex(i);
//         if (vert != 0) {
//             rblks_in[i].fb_addr = vert->get_1st_block();
//             rblks_in[i].lb_addr = vert->get_last_block();
//             rblks_in[i].degree = in_graph->get_degree(i);
//         }
//     }

//     // std::string o_dir = "/home/wr/XPGraph/data/out";
//     std::string o_dir = "/mnt/pmem1/wr/Recovery";
//     FILE* file = fopen((o_dir + "/out_graph.txt").c_str(), "wb+");
//     assert(file != 0);
//     fwrite(rblks_out, sizeof(rblock_t), nverts, file);
//     fclose(file);

//     file = fopen((o_dir + "/in_graph.txt").c_str(), "wb+");
//     assert(file != 0);
//     fwrite(rblks_in, sizeof(rblock_t), nverts, file);
//     fclose(file);

//     char* out_paddr_base = levelgraph->get_pblk_pool0()->get_base();
//     char* in_paddr_base = levelgraph->get_pblk_pool1()->get_base();
//     printf("PMEM Begin Address = %p.\n", out_paddr_base);
//     printf("PMEM Begin Address = %p.\n", in_paddr_base);

//     file = fopen((o_dir + "/info.txt").c_str(), "wb+");
//     assert(file != 0);
//     fwrite(&nverts, sizeof(vid_t), 1, file);
//     fwrite(&out_paddr_base, sizeof(char*), 1, file);
//     fwrite(&in_paddr_base, sizeof(char*), 1, file);
//     fclose(file);
// }

// void load_graph(levelgraph_t *levelgraph) {
//     vid_t nverts = levelgraph->get_vcount();
//     graph_t* out_graph = levelgraph->get_out_graph();
//     graph_t* in_graph = levelgraph->get_in_graph();
//     char* new_out_paddr_base = levelgraph->get_pblk_pool0()->get_base();
//     char* new_in_paddr_base = levelgraph->get_pblk_pool1()->get_base();
    
//     // load mem_nvmpool from pmem
//     // std::string local_mem_file = NVMPATH0 + "nvm_mempool.txt";
//     // size = fsize(local_mem_file.c_str());
//     // local_buf = (char*)pmem_alloc((NVMPATH0 + "nvm_mempool.txt").c_str(), size);

//     // FILE* pfile = fopen(local_mem_file.c_str(), "rb");
//     // if (size != fread(local_buf, sizeof(char), size, pfile)) {
//     //     assert(0);
//     // }

//     char* out_paddr_base = 0;
//     char* in_paddr_base = 0;
//     // std::string recovery_dir = "/home/wr/XPGraph/data/out/";
//     std::string recovery_dir = "/mnt/pmem1/wr/Recovery";

//     FILE* file = fopen((recovery_dir + "/info.txt").c_str(), "rb");
//     assert(file != 0);
//     if(!fread(&nverts, sizeof(vid_t), 1, file)) {
//         assert(0);
//     }
//     fseek(file, sizeof(int), 0);
//     if(!fread(&out_paddr_base, sizeof(char*), 1, file)) {
//         assert(0);
//     }
//     fseek(file, sizeof(int)+sizeof(char*), 0);
//     if(!fread(&in_paddr_base, sizeof(char*), 1, file)) {
//         assert(0);
//     }
//     fclose(file);

//     char* bufptr_out = 0;
//     char* bufptr_in = 0;

//     size_t out_size = alloc_and_read(&bufptr_out, recovery_dir + "/out_graph.txt", 0);
//     size_t in_size = alloc_and_read(&bufptr_in, recovery_dir + "/in_graph.txt", 0);

//     logstream(LOG_DEBUG) << "out graph size = " << out_size << " Bytes." << std::endl;
//     logstream(LOG_DEBUG) << "in graph size = " << in_size  << " Bytes." << std::endl;

//     rblock_t* rblks_out = (rblock_t*)bufptr_out;
//     rblock_t* rblks_in = (rblock_t*)bufptr_in;

//     printf("Old Out PMEM Begin Address = %p.\n", out_paddr_base);
//     printf("Old In PMEM Begin Address = %p.\n", in_paddr_base);
//     printf("New Out PMEM Begin Address = %p.\n", new_out_paddr_base);
//     printf("New In PMEM Begin Address = %p.\n", new_in_paddr_base);

//     // new_addr = old_addr + new_base - paddr_base
//     offset_t out_offset = (pblock_t*)new_out_paddr_base - (pblock_t*)out_paddr_base;
//     offset_t in_offset = (pblock_t*)new_in_paddr_base - (pblock_t*)in_paddr_base;
//     printf("Out Offset = %ld.\n", out_offset);
//     printf("In Offset = %ld.\n", in_offset);

//     #pragma omp for schedule(static) nowait 
//     for (vid_t i = 0; i < nverts; ++i) {
//         // recover out graph
//         if (rblks_out[i].degree != 0) {
//             vertex_t *vert = out_graph->new_vertex();
//             snap_t *vsnap = out_graph->new_snap();
//             // recover block
//             pblock_t* fb = rblks_out[i].fb_addr + out_offset;
//             pblock_t* lb = rblks_out[i].lb_addr + out_offset;
//             pblock_t* cur_blk = fb;
//             while(cur_blk) {
//                 vert->update_last_block(cur_blk);
//                 if (cur_blk == lb) break;
//                 cur_blk = cur_blk->get_next() + out_offset;
//             }
//             // recover degree
//             vsnap->degree = rblks_out[i].degree;
//             vert->set_snap(vsnap);
//             // recover vertex
//             out_graph->set_vertex(i, vert);
//         }

//         // recover in graph
//         if (rblks_in[i].degree != 0) {
//             vertex_t *vert = in_graph->new_vertex();
//             snap_t *vsnap = in_graph->new_snap();
//             // recover block
//             pblock_t* fb = rblks_in[i].fb_addr + in_offset;
//             pblock_t* lb = rblks_in[i].lb_addr + in_offset;
//             pblock_t* cur_blk = fb;
//             while(cur_blk) {
//                 vert->update_last_block(cur_blk);
//                 if (cur_blk == lb) break;
//                 cur_blk = cur_blk->get_next() + in_offset;
//             }
//             // recover degree
//             vsnap->degree = rblks_in[i].degree;
//             vert->set_snap(vsnap);
//             // recover vertex
//             in_graph->set_vertex(i, vert);
//         }
//     }
// }