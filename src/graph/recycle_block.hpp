#pragma once

#include "nvm/mempool.hpp"
#include "graph/vertex.hpp"

class free_blocks_t{
private:
    char* base;
    size_t total_size;
    index_t* free_blk_count;
    char** blks; // blks[0]for 16B blocks, ..., blks[3] for 128B blocks 

    // insert to head
    inline void list_insert(char* p, index_t level){
        *((char **)p) = blks[level];
        blks[level] = (char *)p;
        free_blk_count[level]++;
        total_size += GET_LEVEL_SIZE(level);
    }

    // fetch from head
    inline char* list_fetch(index_t level){ 
        char *p = blks[level];
        assert(p);
        assert(blks[level]);
        blks[level] = *((char **)p);
        free_blk_count[level]--;
        total_size -= GET_LEVEL_SIZE(level);
        return p;
    }

    inline void list_del(char *p, index_t level){
        char *curr = blks[level];
        if(curr == p){ blks[level] = *((char **)p); return ; }
        while (curr != NULL) {
            char* next = *((char **)curr);
            if(next == p){ *(char**) curr = *((char **)p);  break; }
            curr = next;
        }
    }

    inline bool block_is_buddy(char *buddy, index_t level){
        char *p = blks[level];
        while (p != NULL) {
            if(p == buddy) return true;
            p = *((char **)p);
        }
        return false;
    }

    // find buddy in free_blocks, return buddy->prev
    inline bool find_and_del_buddy(char *buddy, index_t level){
        char *curr = blks[level];
        while (curr != NULL) {
            char* next = *((char **)curr);
            if(next == buddy) { 
                *(char**) curr = *((char **)buddy); // delete buddy in list
                return true; 
            }
            curr = next;
        }
        return false;
    }

    inline char* merge_blocks(char *p, index_t &level){
        size_t blk_idx = (p - base) >> MIN_VBUF_BIT;
        while (level < LEVEL_COUNT){
            size_t buddy_idx = blk_idx ^ (1 << level);
            char* buddy = base + (buddy_idx << MIN_VBUF_BIT);
            if(!find_and_del_buddy(buddy, level)) break;
            // if(!block_is_buddy(buddy, level)) break;
            // list_del(buddy, level);
            free_blk_count[level]--;
            total_size -= GET_LEVEL_SIZE(level);
            level++;
            blk_idx &= buddy_idx;
        }
        p = base + (blk_idx << MIN_VBUF_BIT);
        return p;
    }

public:
    free_blocks_t(char* _base){
        base = _base;
        total_size = 0;
        free_blk_count = (index_t*)calloc(sizeof(index_t), LEVEL_COUNT);
        blks = (char**)calloc(sizeof(char*), LEVEL_COUNT);
    }

    ~free_blocks_t(){
        if(blks) free(blks);
        if(free_blk_count) free(free_blk_count);
    }

    void clear(){
        memset(blks,0,sizeof(char*) * LEVEL_COUNT);
        memset(free_blk_count,0,sizeof(index_t) * LEVEL_COUNT);
        total_size = 0;
    }

    inline size_t get_total_size(){ 
        return total_size; 
    }

    // free a block and append it into a linked list for future alloc_node
    inline void recycle_block(char *p, index_t level){
        if(level >= LEVEL_COUNT) logstream(LOG_FATAL) << "Wrong level = " << (uint32_t)level << endl;
        assert(level < LEVEL_COUNT);
        // if(level < LEVEL_COUNT - 1) p = merge_blocks(p, level);
        list_insert(p, level);
    } 

    // free the space of a given size
    inline void recycle_space(char *p, size_t size){
        for(int level = LEVEL_COUNT-1; level >= 0; level--){
            size_t level_size = GET_LEVEL_SIZE(level);
            if(size >= level_size){ recycle_block(p, level); p = p + level_size; size -= level_size;}
        }
        if(size != 0) cout << size << endl;
        assert(size == 0);
    } 

    // allocate a block of align size
    void * alloc_block(index_t level){
        assert(level < LEVEL_COUNT);
        char* find_blk = 0;
        index_t find_level = LEVEL_COUNT;
        for(find_level = level; find_level < LEVEL_COUNT; ++find_level){
            if (blks[find_level]) {
                find_blk = list_fetch(find_level);
                break;
            }
        }
        if(find_blk && find_level > level){
            size_t level_size = GET_LEVEL_SIZE(level);
            char *p = find_blk + level_size;
            for(index_t cur_level = level; cur_level < find_level; ++cur_level){
                list_insert(p, cur_level);
                p = p + level_size;
                level_size = level_size << 1;
            }
        }
        return (void*)find_blk;
    } 

    // print the nodes on the free linked list
    void print_free_blocks(index_t level){
        assert(level < LEVEL_COUNT);
        size_t level_size = GET_LEVEL_SIZE(level);
        printf ("level_%ld free blocks of size_%ld, count = %ld\n", level, level_size, free_blk_count[level]);
        char *p = blks[level];
        while (p != NULL) {
            printf ("%p -> ", p);
            p = *((char **)p);
        }
        printf ("nil\n");
    }

    // print all free blocks
    void print_all_free_blocks(){
        for(index_t level = 0; level < LEVEL_COUNT; level++){
            print_free_blocks(level);
        }
        printf ("\n");
    }
    
    // print total size
    void print_total_size(){
        // printf ("Total size of all free blocks = %ldB (%ldMB).\n", total_size, total_size>>20);
        for(index_t level = 0; level < LEVEL_COUNT; level++){
            if(free_blk_count[level]){ 
                size_t level_size = GET_LEVEL_SIZE(level);
                printf ("\tFree block of size %ldB's count[%ld] = %ld.\n", level_size, level, free_blk_count[level]);
            }
        }
    }
};
