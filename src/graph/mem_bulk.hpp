#pragma once

#include "graph/recycle_block.hpp"

struct mem_bulk_t{
    vertex_t* vertex_bulk;
    snap_t* snap_bulk;
    vid_t vert_count;
    sid_t snap_count;

    char* daddr; // dram bulk addr
    size_t dsize; // dram bulk size
    free_blocks_t* free_vbufs;

    char* paddr; // pmem bulk addr
    size_t psize; // pmem bulk size
    free_blocks_t* free_pblks;
};

class thd_mem_t {
    mempool_t* vbuf_pool;
    mempool_t** pblk_pools;
    mempool_t* vert_pool;
    mempool_t* snap_pool;
    mem_bulk_t* mem; 

    // used for free vert/snap bulks
    std::vector<vertex_t*> vert_bulk_start;
    std::vector<snap_t*> snap_bulk_start;
    std::mutex vb_mutex;
    std::mutex sb_mutex;
 
 public:	
    inline thd_mem_t(mempool_t* _vbuf_pool, mempool_t** _pblk_pool, mempool_t* _vert_pool, mempool_t* _snap_pool)
        : vbuf_pool(_vbuf_pool), pblk_pools(_pblk_pool), vert_pool(_vert_pool), snap_pool(_snap_pool){
        if(posix_memalign((void**)&mem, 64, THD_COUNT*sizeof(mem_bulk_t))) {
            std::cout << "posix_memalign failed()" << std::endl;
            mem = (mem_bulk_t*)calloc(sizeof(mem_bulk_t), THD_COUNT);
        } else {
            memset(mem, 0, THD_COUNT*sizeof(mem_bulk_t));
        }  
        if(MEMPOOL){
            for(tid_t tid = 0; tid < THD_COUNT; tid++){
                mem_bulk_t* mem1 = mem + tid;
                mem1->free_vbufs = new free_blocks_t(vbuf_pool->get_base());
            }
        }
        vert_bulk_start.clear();
        snap_bulk_start.clear();
    }

    ~thd_mem_t(){
        if(!VERT_INPM){
            for(auto it = vert_bulk_start.begin(); it != vert_bulk_start.end(); ++it){
                if(*it) free(*it);
            }
            vert_bulk_start.clear();
        }
        if(!SNAP_INPM){
            for(auto it = snap_bulk_start.begin(); it != snap_bulk_start.end(); ++it){
                if(*it) free(*it);
            }
            snap_bulk_start.clear();
        }
        if(MEMPOOL){
            for(tid_t tid = 0; tid < THD_COUNT; tid++){
                mem_bulk_t* mem1 = mem + tid;
                delete mem1->free_vbufs;
            }
        }
        if(mem) free(mem);
    } 

    inline mem_bulk_t* getMemT(tid_t tid){
        return mem + tid;  
    }

    void print_free_vbufs_size(){
        for(tid_t tid = 0; tid < THD_COUNT; tid++){
            mem_bulk_t* mem1 = mem + tid;
            size_t total_size = mem1->free_vbufs->get_total_size();
            printf ("print_free_vbufs_size of tid = %d, total_size = %ldB (%ldMB).\n", (uint32_t)tid, total_size, total_size>>20);
            mem1->free_vbufs->print_total_size();
        }
    }

    /* -------------------------------------------------------------- */
    // vertex and snap
    inline vertex_t* new_vertex() {
        mem_bulk_t* mem1 = mem + omp_get_thread_num();  
		if (mem1->vert_count == 0) {
            index_t count = VERTEX_BULK_SIZE;
            mem1->vert_count = count;
            if(VERT_INPM){
                mem1->vertex_bulk = (vertex_t*)vert_pool->alloc(sizeof(vertex_t) * count);
                logstream(LOG_WARNING) << "alloc_vertex_bulk: count = " << count << ", callocd " << ((count*sizeof(vertex_t)) >> 20) << "MB." << std::endl;
                vert_pool->print_usage();
            } else {
                mem1->vertex_bulk = (vertex_t*)calloc(sizeof(vertex_t), count);
                {
                    std::lock_guard<std::mutex> guard(vb_mutex);
                    vert_bulk_start.push_back(mem1->vertex_bulk); //多线程不安全
                }
                // logstream(LOG_WARNING) << "alloc_vertex_bulk: count = " << count << ", callocd " << ((count*sizeof(vertex_t)) >> 20) << "MB." << std::endl;
            }
            __sync_fetch_and_add(&vbulk_size, count * sizeof(vertex_t));
		}
		mem1->vert_count--;
		return mem1->vertex_bulk++;
	}

    inline snap_t* new_snap() {
        mem_bulk_t* mem1 = mem + omp_get_thread_num();  
		if (mem1->snap_count == 0) {
            index_t count = SNAP_BULK_SIZE;
            mem1->snap_count = count;
            if(SNAP_INPM){
                mem1->snap_bulk = (snap_t*)snap_pool->alloc(sizeof(snap_t) * count);
                logstream(LOG_WARNING) << "alloc_snap_bulk: count = " << count << ", callocd " << ((count*sizeof(snap_t)) >> 20) << "MB." << std::endl;
                snap_pool->print_usage();
            } else {
                mem1->snap_bulk = (snap_t*)calloc(sizeof(snap_t), count);
                {
                    std::lock_guard<std::mutex> guard(sb_mutex);
                    snap_bulk_start.push_back(mem1->snap_bulk);
                }
                // logstream(LOG_WARNING) << "alloc_snap_bulk: count = " << count << ", callocd " << ((count*sizeof(snap_t)) >> 20) << "MB." << std::endl;
            }
            __sync_fetch_and_add(&snap_bulk_size, count * sizeof(snap_t));
		}
		mem1->snap_count--;
		return mem1->snap_bulk++;
	}

    /* -------------------------------------------------------------- */
    // dram space management
    inline buffer_t* alloc_vbuf(index_t size) {
        buffer_t* vbuf;
        if(MEMPOOL){ 
            vbuf = alloc_vbuf_by_pool(size);
            if(!vbuf) return 0;
        } else { 
            vbuf = (buffer_t*)malloc(size);
        }
        // if(!vbuf) logstream(LOG_WARNING) << "alloc_vbuf of size " << size << "B failed, for no space left in vbuf_pool!" << endl;
        assert(vbuf);
        degree_t max_count = (size - sizeof(buffer_t))/sizeof(vid_t); //该节点当前新分配的vbuf可以容纳的邻居的个数
        vbuf->set_max_nebrcount(max_count); // 当前节点的当前邻接表的能够存储的最多邻居数
        vbuf->set_nebrcount(0); // 当前节点的当前邻接表的已经存储的邻居计数，初始化为0
        return vbuf; 
    }

    buffer_t* alloc_vbuf_by_pool(index_t size) {
        if(size < MIN_VBUF_SIZE || size > MAX_VBUF_SIZE) logstream(LOG_FATAL) << "Wrong size for alloc_vbuf_by_pool: " << size << endl;
        buffer_t* vbuf = 0;
        mem_bulk_t* mem1 = mem + omp_get_thread_num(); 
        if(size <= MAX_VBUF_SIZE){ // First find in the free block list
            index_t level = GET_SIZE_LEVEL(size);
            vbuf = (buffer_t*)mem1->free_vbufs->alloc_block(level);
        }
        if(vbuf == 0){ // Not found in free block list
            if (size > mem1->dsize){ 
                // logstream(LOG_DEBUG) << "alloc a new dram_mem_bulk of " << MEM_BULK_SIZE << ", mem1->dsize = " << mem1->dsize << ", need size = " << size << ", tid = " << omp_get_thread_num() << std::endl;
                assert(size <= MEM_BULK_SIZE);
                if(mem1->dsize > 0) mem1->free_vbufs->recycle_space(mem1->daddr, mem1->dsize);
                if(!alloc_vbuf_bulk()) return 0;
            }
            assert(size <= mem1->dsize);
            vbuf = (buffer_t*)mem1->daddr; // 将vbuf指向mem1区域中当前节点对应的数据区域，类似于CSR中beg_pos的思想
            mem1->daddr += size; //移动mem1中空闲区域的指针mem1->adjlog_beg，供下个节点使用
            mem1->dsize -= size; //更新剩余的空间量
        }
        return vbuf;
    }

    inline bool alloc_vbuf_bulk(){//当该mem1区域剩余的空间不足以容纳当前节点的邻居时，分配新空间，一次性最少分配 256MB
        mem_bulk_t* mem1 = mem + omp_get_thread_num();  
        size_t remain_size = vbuf_pool->get_remained();
        if(remain_size < MEM_BULK_SIZE){ 
            mem1->daddr = 0;
            mem1->dsize = 0;
            return false;
        }
        mem1->daddr = (char*)vbuf_pool->alloc(MEM_BULK_SIZE);
        mem1->dsize = MEM_BULK_SIZE;
        // vbuf_pool->print_usage();
        return true;
    }

    inline void free_vbuf(buffer_t* vbuf, index_t size){
        if(MEMPOOL && vbuf) free_vbuf_by_pool(vbuf, size);
        else if(vbuf){ free(vbuf);}
        vbuf = 0;
    }

    inline void free_vbuf_by_pool(buffer_t* vbuf, size_t size){
        if(size < MIN_VBUF_SIZE || size > MAX_VBUF_SIZE) logstream(LOG_FATAL) << "Wrong size for free_vbuf_by_pool: " << size << endl;
        mem_bulk_t* mem1 = mem + omp_get_thread_num(); 
        mem1->free_vbufs->recycle_block((char*)vbuf, GET_SIZE_LEVEL(size));
    }
    
    inline void clear_vbuf_on_pool(){
        mem_bulk_t* mem1 = mem + omp_get_thread_num(); 
        mem1->free_vbufs->clear();
        mem1->dsize = 0;
        mem1->daddr = 0;
    }

    /* -------------------------------------------------------------- */
    // pmem space management
    inline void alloc_pmem_bulk(size_t size, uint8_t socket_id){//当该mem1区域剩余的空间不足以容纳当前节点的邻居时，分配新空间，一次性最少分配 256MB
        mem_bulk_t* mem1 = mem + omp_get_thread_num();
        assert(size <= MEM_BULK_SIZE);
        mem1->paddr = (char*)pblk_pools[socket_id]->alloc(MEM_BULK_SIZE);
        if(mem1->paddr == 0){
            logstream(LOG_WARNING) << "Fail to alloc a new pmem mem_bulk of " << MEM_BULK_SIZE << ", mem1->size = " << mem1->psize << ", need size = " << size << ", tid = " << omp_get_thread_num() << std::endl;
            logstream(LOG_WARNING) << "No space left in pblk pool." << std::endl;
            exit(0);
        }
        mem1->psize = MEM_BULK_SIZE;
    }

    // sequential allocation
    inline pblock_t* alloc_pblk(index_t size, uint8_t socket_id) {
        mem_bulk_t* mem1 = mem + omp_get_thread_num(); 
        if (size > mem1->psize) alloc_pmem_bulk(size, socket_id);

        pblock_t* pblk = (pblock_t*)mem1->paddr; // 将vbuf指向mem1区域中当前节点对应的数据区域，类似于CSR中beg_pos的思想
        mem1->paddr += size; //移动mem1中空闲区域的指针mem1->adjlog_beg，供下个节点使用
        mem1->psize -= size; //更新剩余的空间量

        assert(pblk);
        degree_t max_count = (size - sizeof(pblock_t))/sizeof(vid_t); //该节点当前新分配的vbuf可以容纳的邻居的个数
        pblk->add_next((pblock_t*)NULL);
        pblk->set_nebrcount(0); // 当前节点的当前邻接表的已经存储的邻居计数，初始化为0
        pblk->set_max_nebrcount(max_count); // 当前节点的当前邻接表的能够存储的最多邻居数

        return pblk;
    }

    // XPLINE aliged allocation
    inline pblock_t* alloc_pblk_aligned(index_t size, uint8_t socket_id) {
        mem_bulk_t* mem1 = mem + omp_get_thread_num(); 
        if (size > mem1->psize) alloc_pmem_bulk(size, socket_id);
        
        pblock_t* pblk = 0;
        if(size <= MAX_VBUF_SIZE){
            index_t level = GET_SIZE_LEVEL(size);
            pblk = (pblock_t*)mem1->free_pblks->alloc_block(level);
        }
        if(pblk == 0){
            if (size > mem1->psize) { 
                alloc_pmem_bulk(size, socket_id);
            }
            pblk = (pblock_t*)mem1->paddr; // 将vbuf指向mem1区域中当前节点对应的数据区域，类似于CSR中beg_pos的思想
            if(MEMPOOL && size < XPLINE_SIZE){
                mem1->free_pblks->recycle_space(mem1->paddr + size, XPLINE_SIZE - size);
                mem1->paddr += XPLINE_SIZE; //移动mem1中空闲区域的指针mem1->adjlog_beg，供下个节点使用
                mem1->psize -= XPLINE_SIZE;
            } else {
                mem1->paddr += size; //移动mem1中空闲区域的指针mem1->adjlog_beg，供下个节点使用
                mem1->psize -= size; //更新剩余的空间量
            }
        }
        assert(pblk);
        degree_t max_count = (size - sizeof(pblock_t))/sizeof(vid_t); //该节点当前新分配的vbuf可以容纳的邻居的个数
        pblk->add_next((pblock_t*)NULL);
        pblk->set_nebrcount(0); // 当前节点的当前邻接表的已经存储的邻居计数，初始化为0
        pblk->set_max_nebrcount(max_count); // 当前节点的当前邻接表的能够存储的最多邻居数

        return pblk;
    }
};
