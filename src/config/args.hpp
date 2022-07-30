#pragma once

/* ---------------------------------------------------------------------- */
// Basic arguments for graph info
std::string filepath; // path of graph dataset
uint32_t nverts;
uint64_t nedges;
/* ---------------------------------------------------------------------- */
// Basic arguments for system info
uint8_t THD_COUNT; // num of threads
uint8_t NUMA_OPT = 2; // implementation of numa optimization: 0 for closing NUMA optimization, 
                        // 1 for out/in-graph based implementation, 2 for sub-graph based implementation;
uint8_t NUM_SOCKETS = 2; // num of sockets
std::string NVMPATH0; // path of pmem0
std::string NVMPATH1; // path of pmem1
std::string RECYPATH; // path of recovery
/* ---------------------------------------------------------------------- */
// Basic arguments for job
uint8_t JOB; // job id: 0 for ingest, 1 for log, 2 for archive
uint8_t residue; // archive threshold
uint8_t QUERY; // times for query

/* ---------------------------------------------------------------------- */
// used for edge log management
uint8_t  ELOG_SHIFT;//bits of size of edge log, 30 for 8GB
uint8_t  BATCH_SHIFT;//bits of edge batching in edge log
uint64_t  ELOG_CAP;//capacity of edge log
uint64_t  ELOG_MASK;
uint64_t  BATCH_CAP;// edge batching in edge log
uint64_t  BATCH_MASK; // 0xFFFF;
uint64_t  BUFFER_CAP;// edge buffering size in edge log

/* ---------------------------------------------------------------------- */
// Leveled buffer config
bool LEVELED_BUF; // leveled_buf = 1 for adaptively hirerachical vertex buffering
size_t MIN_VBUF_SIZE; // Byte
size_t MAX_VBUF_SIZE;
uint64_t MAX_VBUF_CAP;
uint8_t MIN_VBUF_BIT;
uint8_t LEVEL_COUNT; // [16B,256B]->5

/* ---------------------------------------------------------------------- */
// Mempool management
bool MEMPOOL = 1;
size_t MEM_BULK_SIZE; // mem_bulk size
/* ---------------------------------------------------------------------- */
// Metadata management
uint64_t SNAP_BULK_SIZE;
uint64_t VERTEX_BULK_SIZE;
bool VERT_INPM;
bool SNAP_INPM;
bool TEDGE_INPM;
bool LEBUF_INPM;
/* ---------------------------------------------------------------------- */
// Mempool size
size_t VERT_POOL_SIZE; // vertex pool size 
size_t SNAP_POOL_SIZE; // snapshot pool size 
size_t TEDGE_POOL_SIZE; // Temp edge (global_range.edges) pool size
size_t VBUF_POOL_SIZE; // vbuf pool size 
size_t PBLK_POOL_SIZE; // Total pblk pool size for all sockets

/* ---------------------------------------------------------------------- */
// Basic arguments for XPGraph config
bool BATTERY;
bool DRAMONLY;
bool CLWB;
bool PERSIST;
