#pragma once
#include <libxpgraph.h>

// /* ---------------------------------------------------------------------- */
// // Simple case test
// void simple_test(metrics &m){ // edges = {1->2, 1->3, 2->1}
//     ELOG_CAP = 1L << 4;
//     const vid_t buf[8] = {0,1,1,2,1,3,2,1};
//     levelgraph_t *levelgraph = new levelgraph_t(4, m);
//     index_t edge_count = 4;
//     edge_t* edges = (edge_t*)&buf;
//     vid_t src, dst;
//     for (index_t i = 0; i < edge_count; ++i) {
//         levelgraph->batch_edge(edges[i]);
//         // edge_t edge = edges[i];
//         // vid_t src = (vid_t)GET_SRC(edge);
//         // vid_t dst = (vid_t)GET_DST(edge);
//         src = edges[i].src;
//         dst = edges[i].dst;
//         logstream(LOG_INFO) << i << ": " << src << " -> " << dst << std::endl;
//     }
//     levelgraph->inform_wait_buffer_all();

//     logstream(LOG_WARNING) << "Print out-neighbors :" << std::endl;
//     graph_t *out_graph = levelgraph->get_out_graph();
//     out_graph->query_nebrs(0); // expect 1
//     out_graph->query_nebrs(1); // expect 2 3
//     out_graph->query_nebrs(2); // expect 1
//     out_graph->query_nebrs(3); // expect NULL
//     logstream(LOG_WARNING) << "Print in-neighbors :" << std::endl;
//     graph_t *in_graph = levelgraph->get_in_graph();
//     in_graph->query_nebrs(1); // expect 2
//     in_graph->query_nebrs(2); // expect 1
//     in_graph->query_nebrs(3); // expect 1

//     delete levelgraph;
//     exit(0);
// }

/* ---------------------------------------------------------------------- */
// Out/In_neighbors test
void test_out_neighbors(XPGraph *xpgraph){
    std::cout << "Print out-neighbors :" << std::endl;
    if(xpgraph == 0) return;
    xpgraph->print_out_nebrs(1); // expect (outd:20) 2~21  
    xpgraph->print_out_nebrs(2); // expect (outd:0) 
    xpgraph->print_out_nebrs(22); // expect (outd:4) 2 23 24 25
    xpgraph->print_out_nebrs(26); // expect (outd:9) 2 27 28 29 30 31 32 33 34
    xpgraph->print_out_nebrs(70); // expect (outd:87) 71~157
}

void test_in_neighbors(XPGraph *xpgraph){
    std::cout << "Print in-neighbors :" << std::endl;
    if(xpgraph == 0) return;
    xpgraph->print_in_nebrs(1); // expect  
    xpgraph->print_in_nebrs(2); // expect (d:3) 1 22 26
    xpgraph->print_in_nebrs(22); // expect 
    xpgraph->print_in_nebrs(26); // expect 
    xpgraph->print_in_nebrs(710); // expect (d:33) 
}

// /* ---------------------------------------------------------------------- */
// // Sequantial/Random read test
// void sequantial_read(edge_t* edges, index_t edge_count, vid_t nverts){
//     edge_t edge;
//     vid_t src, dst;
//     for (index_t i = 0; i < edge_count; ++i){
//         edge = edges[i];
//         src = edge.src;
//         dst = edge.dst;
//         assert(src < nverts);
//         assert(dst < nverts);
//     }
// }

// void random_read(edge_t* edges, index_t edge_count, vid_t nverts){
//     edge_t edge;
//     vid_t src, dst;
//     index_t index = 0;
//     for (index_t i = 0; i < edge_count; ++i){
//         index = rand() % edge_count;
//         edge = edges[index];
//         src = edge.src;
//         dst = edge.dst;
//         assert(src < nverts);
//         assert(dst < nverts);
//     }
// }

// void part_seq_read(edge_t* edges, index_t edge_count, vid_t nverts){
//     edge_t edge;
//     vid_t src, dst;
//     index_t j, index = 0;
//     for (index_t i = 0; i < edge_count; ++i){
//         index = rand() % edge_count;
//         for(j = 0; j < 12; j++){
//             edge = edges[index+j];
//             src = edge.src;
//             dst = edge.dst;
//             assert(src < nverts);
//             assert(dst < nverts);
//             ++i;
//         }
//     }
// }

// /* ---------------------------------------------------------------------- */
// // Sequantial/Random write test
// void sequantial_write(const char* buf, size_t size, edge_t* edges){
//     index_t edge_count = size/sizeof(edge_t);
//     const char* bufptr = buf;
//     for (index_t i = 0; i < edge_count; ++i){
//         edges[i] = *((edge_t*)bufptr);
//         bufptr += sizeof(edge_t);
//     }
// }

// void random_write(const char* buf, size_t size, edge_t* edges){
//     index_t edge_count = size/sizeof(edge_t);
//     const char* bufptr = buf;
//     index_t index = 0;
//     for (index_t i = 0; i < edge_count; ++i){
//         index = rand() % edge_count;
//         edges[index] = *((edge_t*)bufptr);
//         bufptr += sizeof(edge_t);
//     }
// }

// void part_seq_write(const char* buf, size_t size, edge_t* edges){
//     index_t edge_count = size/sizeof(edge_t);
//     const char* bufptr = buf;
//     index_t j, index = 0;
//     for (index_t i = 0; i < edge_count; ++i){
//         index = rand() % edge_count;
//         for(j = 0; j < 12; j++){
//             edges[index+j] = *((edge_t*)bufptr);
//             bufptr += sizeof(edge_t);
//             i++;
//         }
//     }
// }

// /* ---------------------------------------------------------------------- */
// // Hybrid read/write test
// void test_write_read_rate(vid_t nverts){
//     metrics m("test_write_rate");
//     m.start_time("0-readbuf_to_dram");
//     char* buf = 0;
//     index_t size = alloc_mem_dir("/home/wr/data/Friendster/bin", "/pmem/wr/XPGraphDB/", &buf); // 输入文件读入 DRAM
//     if (size != read_bin_dir("/home/wr/data/Friendster/bin", buf)) {
//         logstream(LOG_FATAL) << "Read wrong size!" << std::endl;
//         assert(0);
//     }
//     logstream(LOG_INFO) << "Loacl buffer read " << (size >> 20) << "MB for local buffer on DRAM, tid = " << omp_get_thread_num() << std::endl;
//     index_t edge_count = size/sizeof(edge_t);
//     m.stop_time("00-readbuf_to_dram");

//     m.start_time("01-seq_write_to_dram0");
//     edge_t* dram_edges = (edge_t*)calloc(edge_count, sizeof(edge_t));
//     sequantial_write(buf, size, dram_edges);
//     m.stop_time("01-seq_write_to_dram0");
//     m.start_time("02-seq_write_to_pmem0");
//     edge_t* pmem_edges = (edge_t*)pmem_alloc("/mnt/pmem/wr/test_pmem_write.bin", edge_count * sizeof(edge_t));
//     sequantial_write(buf, size, pmem_edges);
//     m.stop_time("02-seq_write_to_pmem0");
//     m.start_time("03-seq_write_to_dram1");
//     edge_t* dram_edges1 = (edge_t*)calloc(edge_count, sizeof(edge_t));
//     sequantial_write(buf, size, dram_edges1);
//     m.stop_time("03-seq_write_to_dram1");
//     m.start_time("04-seq_write_to_pmem1");
//     edge_t* pmem_edges1 = (edge_t*)pmem_alloc("/mnt/pmem/wr/test_pmem_write1.bin", edge_count * sizeof(edge_t));
//     sequantial_write(buf, size, pmem_edges1);
//     m.stop_time("04-seq_write_to_pmem1");
//     m.start_time("05-seq_write_to_dram2");
//     edge_t* dram_edges2 = (edge_t*)calloc(edge_count, sizeof(edge_t));
//     sequantial_write(buf, size, dram_edges2);
//     m.stop_time("05-seq_write_to_dram2");
//     m.start_time("06-seq_write_to_pmem2");
//     edge_t* pmem_edges2 = (edge_t*)pmem_alloc("/mnt/pmem/wr/test_pmem_write2.bin", edge_count * sizeof(edge_t));
//     sequantial_write(buf, size, pmem_edges2);
//     m.stop_time("06-seq_write_to_pmem2");

//     m.start_time("07-rand_write_to_dram");
//     edge_t* dram_edges3 = (edge_t*)calloc(edge_count, sizeof(edge_t));
//     random_write(buf, size, dram_edges3);
//     m.stop_time("07-rand_write_to_dram");
//     m.start_time("08-rand_write_to_pmem");
//     edge_t* pmem_edges3 = (edge_t*)pmem_alloc("/mnt/pmem/wr/test_pmem_write.bin", edge_count * sizeof(edge_t));
//     random_write(buf, size, pmem_edges3);
//     m.stop_time("08-rand_write_to_pmem");
//     m.start_time("09-rand_write_to_dram1");
//     edge_t* dram_edges4 = (edge_t*)calloc(edge_count, sizeof(edge_t));
//     random_write(buf, size, dram_edges4);
//     m.stop_time("09-rand_write_to_dram1");
//     m.start_time("10-rand_write_to_pmem1");
//     edge_t* pmem_edges4 = (edge_t*)pmem_alloc("/mnt/pmem/wr/test_pmem_write1.bin", edge_count * sizeof(edge_t));
//     random_write(buf, size, pmem_edges4);
//     m.stop_time("10-rand_write_to_pmem1");
//     m.start_time("11-rand_write_to_dram2");
//     edge_t* dram_edges5 = (edge_t*)calloc(edge_count, sizeof(edge_t));
//     random_write(buf, size, dram_edges5);
//     m.stop_time("11-rand_write_to_dram2");
//     m.start_time("12-rand_write_to_pmem2");
//     edge_t* pmem_edges5 = (edge_t*)pmem_alloc("/mnt/pmem/wr/test_pmem_write2.bin", edge_count * sizeof(edge_t));
//     random_write(buf, size, pmem_edges5);
//     m.stop_time("12-rand_write_to_pmem2");

//     m.start_time("13-part_seq_write_to_dram");
//     edge_t* dram_edges6 = (edge_t*)calloc(edge_count, sizeof(edge_t));
//     random_write(buf, size, dram_edges6);
//     m.stop_time("13-part_seq_write_to_dram");
//     m.start_time("14-part_seq_write_to_pmem");
//     edge_t* pmem_edges6 = (edge_t*)pmem_alloc("/mnt/pmem/wr/test_pmem_write.bin", edge_count * sizeof(edge_t));
//     random_write(buf, size, pmem_edges6);
//     m.stop_time("14-part_seq_write_to_pmem");
//     m.start_time("15-part_seq_write_to_dram1");
//     edge_t* dram_edges7 = (edge_t*)calloc(edge_count, sizeof(edge_t));
//     random_write(buf, size, dram_edges7);
//     m.stop_time("15-part_seq_write_to_dram1");
//     m.start_time("16-part_seq_write_to_pmem1");
//     edge_t* pmem_edges7 = (edge_t*)pmem_alloc("/mnt/pmem/wr/test_pmem_write1.bin", edge_count * sizeof(edge_t));
//     random_write(buf, size, pmem_edges7);
//     m.stop_time("16-part_seq_write_to_pmem1");
//     m.start_time("17-part_seq_write_to_dram2");
//     edge_t* dram_edges8 = (edge_t*)calloc(edge_count, sizeof(edge_t));
//     random_write(buf, size, dram_edges8);
//     m.stop_time("17-part_seq_write_to_dram2");
//     m.start_time("18-part_seq_write_to_pmem2");
//     edge_t* pmem_edges8 = (edge_t*)pmem_alloc("/mnt/pmem/wr/test_pmem_write2.bin", edge_count * sizeof(edge_t));
//     random_write(buf, size, pmem_edges8);
//     m.stop_time("18-part_seq_write_to_pmem2");

//     /* ----- test read ---- */
//     m.start_time("19-seq_read_from_dram0");
//     sequantial_read(dram_edges, edge_count, nverts);
//     m.stop_time("19-seq_read_from_dram0");
//     m.start_time("20-seq_read_from_pmem0");
//     sequantial_read(pmem_edges, edge_count, nverts);
//     m.stop_time("20-seq_read_from_pmem0");
//     m.start_time("21-seq_read_from_dram1");
//     sequantial_read(dram_edges1, edge_count, nverts);
//     m.stop_time("21-seq_read_from_dram1");
//     m.start_time("22-seq_read_from_pmem1");
//     sequantial_read(pmem_edges1, edge_count, nverts);
//     m.stop_time("22-seq_read_from_pmem1");
//     m.start_time("23-seq_read_from_dram2");
//     sequantial_read(dram_edges2, edge_count, nverts);
//     m.stop_time("23-seq_read_from_dram2");
//     m.start_time("24-seq_read_from_pmem2");
//     sequantial_read(pmem_edges2, edge_count, nverts);
//     m.stop_time("24-seq_read_from_pmem2");

//     m.start_time("25-rand_read_from_dram0");
//     random_read(dram_edges, edge_count, nverts);
//     m.stop_time("25-rand_read_from_dram0");
//     m.start_time("26-rand_read_from_pmem0");
//     random_read(pmem_edges, edge_count, nverts);
//     m.stop_time("26-rand_read_from_pmem0");
//     m.start_time("27-rand_read_from_dram1");
//     random_read(dram_edges1, edge_count, nverts);
//     m.stop_time("27-rand_read_from_dram1");
//     m.start_time("28-rand_read_from_pmem1");
//     random_read(pmem_edges1, edge_count, nverts);
//     m.stop_time("28-rand_read_from_pmem1");
//     m.start_time("29-rand_read_from_dram2");
//     random_read(dram_edges2, edge_count, nverts);
//     m.stop_time("29-rand_read_from_dram2");
//     m.start_time("30-rand_read_from_pmem2");
//     random_read(pmem_edges2, edge_count, nverts);
//     m.stop_time("30-rand_read_from_pmem2");

//     m.start_time("31-part_seq_read_from_dram0");
//     part_seq_read(dram_edges, edge_count, nverts);
//     m.stop_time("31-part_seq_read_from_dram0");
//     m.start_time("32-part_seq_read_from_pmem0");
//     part_seq_read(pmem_edges, edge_count, nverts);
//     m.stop_time("32-part_seq_read_from_pmem0");
//     m.start_time("33-part_seq_read_from_dram1");
//     part_seq_read(dram_edges1, edge_count, nverts);
//     m.stop_time("33-part_seq_read_from_dram1");
//     m.start_time("34-part_seq_read_from_pmem1");
//     part_seq_read(pmem_edges1, edge_count, nverts);
//     m.stop_time("34-part_seq_read_from_pmem1");
//     m.start_time("35-part_seq_read_from_dram2");
//     part_seq_read(dram_edges2, edge_count, nverts);
//     m.stop_time("35-part_seq_read_from_dram2");
//     m.start_time("36-part_seq_read_from_pmem2");
//     part_seq_read(pmem_edges2, edge_count, nverts);
//     m.stop_time("36-part_seq_read_from_pmem2");

//     metrics_report(m);
//     exit(0);
// }

