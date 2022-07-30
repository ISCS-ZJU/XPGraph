#pragma once
#include "utils/basic_includes.hpp"

using std::cout;
using std::endl;

bool HELP = 0;

void print_usage() {
    string help = "./exe options.\n";
    help += " -h: This message.\n";

    help += " -f: Input dataset file.\n";
    help += " -v: Vertex count.\n";
    help += " -e: Edge count need for import. Default: 0 for importing all edges in the dataset file.\n";

    help += " -j: Job id (0 for ingest, 1 for log, 2 for archive...). Default: 0.\n";
    help += " -r: Archive threshold for job2.\n";
    help += " -q: The number of executions for each graph query algorithm.\n";

    help += " -t: Thread count for buffering and flushing. Default: 16.\n"; //Cores in your system - 1.\n";
    help += " -s: Sockets count. Default: 2.\n";
    help += " -p0: Path of pmem0 of NUMA node0.\n";
    help += " -p1: Path of pmem1 of NUMA node1.\n";
    help += " -numa: Implementation of numa optimization: 0 for closing NUMA optimization, 1 for out/in-graph based implementation, 2 for sub-graph based implementation. Default: 2.\n";

    help += " --elogsize: Bits of maximum number of edges in edge log. Default: 30 for 1 billion edges, i.e., edge size equals 8GB.\n";
    help += " --leveled_buf: 0 for fixed vertex buffer size setting, 1 for hierarchical vertex buffer size setting. Default: 1.\n";
    help += " --minvbuf: Minimum per-vertex buffer size in bytes. Default: 16.\n";
    help += " --maxvbuf: Maximum per-vertex buffer size in bytes, or fixed per-vertex buffer size when leveled_buf = 0. Default: 256.\n";

    help += " --mempool: 0 for malloc/free-based vertex buffer managing, 1 for memory pool based vertex buffer managing. Default: 1.\n";
    help += " --membulk: Memory bulk size in MegaBytes. Default: 16.\n";
    help += " --vbuf_pool: Total vertex buffer pool size threshold in GigaByte. Default: 16.\n";
    help += " --pblk_pools: Total persistent PMEM block pool size threshold in GigaByte. Default: 64.\n";

    help += " --battery: 1 for XPGraph-B by allowing overwritting buffered edges. Default: 0.\n";
    help += " --dram_only: 1 for XPGraph-D by storing all data structure in DRAM and set the per-vertex buffer size as fixed 64B. Default: 0.\n";
    help += "--persist: 1 for saving XPGraph information, used for recovery. Default: 1.\n";
    help += "--recovery: Path of recovery data.\n";
 
    // help += " --direction -d: Direction, 0 for undirected, 1 for directed, 2 for unidirected. Default: 0(undirected)\n";
    // help += " --source  -s: Data source. 0 for text files, 1 for binary files. Default: text files\n";

    cout << help << endl;
}

void args_config(int argc, const char ** argv) {
    set_argc(argc, argv);
    /* ---------------------------------------------------------------------- */
    // print_usage
    HELP = get_option_int("-h", 0); // help for print_usage

    /* ---------------------------------------------------------------------- */
    // Basic arguments for graph info
    filepath = get_option_string("-f", "/mnt/nvme1/wr/dataset/Friendster/all/bin");  // Input dataset file.
    // filepath = get_option_string("-f", "/data/wr/Friendster/all/bin");  // Input dataset file
    nverts = get_option_int("-v", 68349467); // Vertex count. //TT:61588415, FS:68349467, UK:105153953, K28:268435456, YW:1413511394, K29:536870912, K30:1073741824
    nedges = get_option_int("-e", 0); // Edge count need for import. Default: 0 for importing all edges in the dataset file.

    /* ---------------------------------------------------------------------- */
    // Basic arguments for job
    JOB = get_option_int("-j", 0); // Job id (0 for ingest, 1 for log, 2 for archive...). Default: 0.
    residue = get_option_int("-r", 16); // Archive threshold for job2.
    QUERY = get_option_int("-q", 0); // The number of executions for each graph query algorithm.

    /* ---------------------------------------------------------------------- */
    // Basic arguments for system info
    THD_COUNT = get_option_int("-t", 16); // Thread count for buffering and flushing. Default: 16.
    NUM_SOCKETS = get_option_int("-s", 2); // Sockets count. Default: 2.
    NVMPATH0 = get_option_string("-p0", "/pmem/wr/XPGraphDB/");  // Path of pmem0 of NUMA node0.
    NVMPATH1 = get_option_string("-p1", "/mnt/pmem1/wr/XPGraphDB/"); // Path of pmem1 of NUMA node1.
    NUMA_OPT = get_option_int("-numa", 2); // Implementation of numa optimization: 0 for closing NUMA optimization, 
                                        // 1 for out/in-graph based implementation, 2 for sub-graph based implementation;

    /* ---------------------------------------------------------------------- */
    // used for edge log management
    ELOG_SHIFT = get_option_int("--elogsize", 30); // Bits of maximum number of edges in edge log. Default: 30 for 1 billion edges, i.e., edge size equals 8GB. (27 for 1GB)
    ELOG_CAP = 1L << ELOG_SHIFT;//capacity of edge log
    ELOG_MASK =  ELOG_CAP - 1;
    BATCH_SHIFT = 16;//bits of edge batching in edge log
    BATCH_CAP = 1L << BATCH_SHIFT;// edge batching in edge log
    BATCH_MASK = BATCH_CAP - 1; // 0xFFFF;
    BUFFER_CAP = ELOG_CAP - (BATCH_CAP * 64);// edge buffering size in edge log

    /* ---------------------------------------------------------------------- */
    // Leveled vertex buffer config
    LEVELED_BUF = get_option_int("--leveled_buf", 1); // 0 for fixed vertex buffer size setting, 1 for hierarchical vertex buffer size setting. Default: 1.
    MIN_VBUF_SIZE = get_option_int("--minvbuf", 16); // Minimum per-vertex buffer size in bytes. Default: 16.
    MAX_VBUF_SIZE = get_option_int("--maxvbuf", 256); // Maximum per-vertex buffer size in bytes, or fixed per-vertex buffer size when leveled_buf = 0. Default: 256.
    MIN_VBUF_BIT  = log2(MIN_VBUF_SIZE); // 4
    MAX_VBUF_CAP = (MAX_VBUF_SIZE - sizeof(buffer_t))/sizeof(vid_t); // 63
    LEVEL_COUNT = log2(MAX_VBUF_SIZE/MIN_VBUF_SIZE) + 1; // 5

    /* ---------------------------------------------------------------------- */
    // Mempool management
    MEMPOOL = get_option_int("--mempool", 1); // 0 for malloc/free-based vertex buffer managing, 1 for memory pool based vertex buffer managing. Default: 1.
    MEM_BULK_SIZE = get_option_int("--membulk", 16) * MB; // Memory bulk size in MegaBytes. Default: 16.

    /* ---------------------------------------------------------------------- */
    // Metadata management
    VERTEX_BULK_SIZE = 1 << 20;
    SNAP_BULK_SIZE = 1 << 20;
    VERT_INPM = get_option_int("--vert_inpm", 0); // vert_inpm = 1 for store vertex info in pmem
    SNAP_INPM = get_option_int("--snap_inpm", 0); // snap_inpm = 1 for store snap info in pmem
    TEDGE_INPM = get_option_int("--tedge_inpm", 0); // lbuf_inpm = 1 for store local buf in pmem
    LEBUF_INPM = get_option_int("--lbuf_inpm", 1); // lbuf_inpm = 1 for store local buf in pmem

    /* ---------------------------------------------------------------------- */
    // mempool size
    VERT_POOL_SIZE = get_option_long("--vert_pool", 8) * GB; // Total vert pool size threshold, GB
    SNAP_POOL_SIZE = get_option_long("--snap_pool", 8) * GB; // Total snap pool size threshold, GB
    TEDGE_POOL_SIZE = get_option_long("--tedge_pool", 8) * GB; // Total tedge pool size threshold, GB
    VBUF_POOL_SIZE = get_option_long("--vbuf_pool", 16) * GB; // Total vertex buffer pool size threshold in GigaByte. Default: 16.
    PBLK_POOL_SIZE = get_option_long("--pblk_pools", 64) * GB; // Total persistent PMEM block pool size threshold in GigaByte. Default: 64. 
    /* ---------------------------------------------------------------------- */
    // PMEM mempool size

    /* ---------------------------------------------------------------------- */
    // Other arguments config
    BATTERY = get_option_int("--battery", 0); // 1 for XPGraph-B by allowing overwritting buffered edges. Default: 0.
    DRAMONLY = get_option_int("--dram_only", 0); // 1 for XPGraph-D by storing all data structure in DRAM and set the per-vertex buffer size as fixed 64B. Default: 0.
    if(DRAMONLY == 1){
        LEVELED_BUF = 0;
        MIN_VBUF_SIZE = MAX_VBUF_SIZE = 64;
        MIN_VBUF_BIT  = log2(MIN_VBUF_SIZE); // 4
        MAX_VBUF_CAP = (MAX_VBUF_SIZE - sizeof(buffer_t))/sizeof(vid_t); // 63
        LEVEL_COUNT = log2(MAX_VBUF_SIZE/MIN_VBUF_SIZE) + 1; // 5
    }

    CLWB = get_option_int("--clwb", 0); // clwb = 1 for clwb proactively
    PERSIST = get_option_int("--persist", 1); // persist = 1 for saving XPGraph information, used for recovery
    RECYPATH = get_option_string("--recovery", "/mnt/pmem1/wr/Recovery/");  // Path of recovery data.
}

void prinf_config(){
    /* ---------------------------------------------------------------------- */
    // print_usage
    if(HELP == 1){
        print_usage();
        exit(0);
    }

    /* ---------------------------------------------------------------------- */
    // Basic arguments for graph info
    cout<< "Print config information:" << endl;
    cout<< "  Graph info: " << endl;
    cout<< "\t - filepath = " << filepath << endl;
    cout<< "\t - nverts = " << nverts << endl;
    cout<< "\t - nedges = " << nedges << " (0 for processing all edges in the filepath) " << endl;

    /* ---------------------------------------------------------------------- */
    // Basic arguments for job
    cout<< "  JOB: " << endl;
    cout<<  "\t - Update type = " << (uint32_t)JOB << " (0 for ingest, 1 for log, 2 for archive) " << endl;
    if(JOB == 2) cout<< "\t - Archive threshold = " << (uint32_t)residue << ", i.e., " << pow(2.0, residue) << " edges." << endl;
    cout<< "\t - QUERY = " << (uint32_t)QUERY << " time(s) for each algorithm" << endl;

    /* ---------------------------------------------------------------------- */
    // Basic arguments for system info
    cout<< "  System info: " << endl;
    cout<< "\t - THD_COUNT = " << (uint32_t)THD_COUNT << endl;
    cout<< "\t - NUM_SOCKETS = " << (uint32_t)NUM_SOCKETS << endl;
    cout<< "\t - NVMPATH0 = " << NVMPATH0 << endl;
    cout<< "\t - NVMPATH1 = " << NVMPATH1 << endl;
    cout<< "\t - RECOVERY = " << RECYPATH << endl;
    cout<< "\t - NUMA_OPT = " << (uint32_t)NUMA_OPT << " (0 for closing NUMA optimization, 1 for out/in-graph based implementation, 2 for sub-graph based implementation) " << endl;

    /* ---------------------------------------------------------------------- */
    // used for edge log management
    cout<< "  Edge log: " << endl;
    cout<< "\t - ELOG_CAP = " << ELOG_CAP << ", size = " << ((ELOG_CAP+1) * 8 /GB) << "GB." << endl;
    cout<< "\t - BATCH_CAP = " << BATCH_CAP << ", size = " << ((BATCH_CAP) * 8 / KB) << "KB." << endl;
    cout<< "\t - BUFFER_CAP = " << BUFFER_CAP << ", size = " << ((BUFFER_CAP) * 8 / MB) << "MB." << endl;
    /* ---------------------------------------------------------------------- */
    // Leveled vertex buffer config
    cout<< "  Vertex buffer: LEVELED_BUF = " << (uint32_t)LEVELED_BUF << endl;
    cout<< "\t - Per-vertex MIN_VBUF_SIZE = " << (uint32_t)MIN_VBUF_SIZE << ",  MAX_VBUF_SIZE = " << (uint32_t)MAX_VBUF_SIZE << endl;
    cout<< "\t - LEVEL_COUNT = " << (uint32_t)LEVEL_COUNT << ", MAX_VBUF_CAP = " << MAX_VBUF_CAP << endl;
    /* ---------------------------------------------------------------------- */
    // Mempool management
    cout<< "  MEMPOOL = " << (uint32_t)MEMPOOL << endl;
    if(MEMPOOL){
        cout<< "\t - MEM_BULK_SIZE = " << MEM_BULK_SIZE / MB << "MB." << endl;
    }
    /* ---------------------------------------------------------------------- */
    // Metadata management
    cout<< "  Meta management: " << endl;
    cout<< "\t - VERTEX_BULK_SIZE = " << VERTEX_BULK_SIZE << endl;
    cout<< "\t - SNAP_BULK_SIZE = " << SNAP_BULK_SIZE << endl;
    cout<< "\t - VERT_INPM = " << (uint32_t)VERT_INPM << endl;
    cout<< "\t - SNAP_INPM = " << (uint32_t)SNAP_INPM << endl;
    cout<< "\t - TEDGE_INPM = " << (uint32_t)TEDGE_INPM << endl;
    cout<< "\t - LEBUF_INPM = " << (uint32_t)LEBUF_INPM << endl;
    /* ---------------------------------------------------------------------- */
    // mempool size
    if(MEMPOOL){
        cout<< "  Memory size: " << endl;
        if(VERT_INPM) cout<< "\t - VERT_POOL_SIZE = " << VERT_POOL_SIZE / GB << "GB." << endl;
        if(SNAP_INPM) cout<< "\t - SNAP_POOL_SIZE = " << SNAP_POOL_SIZE / GB << "GB." << endl;
        if(TEDGE_INPM) cout<< "\t - TEDGE_POOL_SIZE = " << TEDGE_POOL_SIZE / GB << "GB." << endl;
        cout<< "\t - VBUF_POOL_SIZE = " << VBUF_POOL_SIZE / GB << "GB." << endl;
        cout<< "\t - PBLK_POOL_SIZE = " << PBLK_POOL_SIZE / GB << "GB." << endl;
    }
    /* ---------------------------------------------------------------------- */
    // Other arguments config
    cout<< "  Other arguments config: " << endl;
    cout<< "\t - BATTERY = " << (uint32_t)BATTERY << endl;
    cout<< "\t - DRAMONLY = " << (uint32_t)DRAMONLY << endl;
    cout<< "\t - CLWB = " << (uint32_t)CLWB << endl;
    cout<< "\t - PERSIST = " << (uint32_t)PERSIST << endl;
}
