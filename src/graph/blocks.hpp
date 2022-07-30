#pragma once

#include "utils/basic_includes.hpp"

// blocks in DRAM, each vertex only have 1 block, 
// size: 16B ~ 256B
class buffer_t{ 
private:
    blk_deg_t max_count;
    blk_deg_t count;

public:
    /* -------------------------------------------------------------- */
    // get/set block info
    inline blk_deg_t get_max_nebrcount() { return max_count;}
	inline void set_max_nebrcount(blk_deg_t degree) {max_count = degree;}
    inline blk_deg_t get_nebrcount() { return count;}
	inline void set_nebrcount(blk_deg_t degree) {count = degree;}
    inline size_t get_size() { return max_count * sizeof(vid_t) + sizeof(buffer_t); };
    inline bool is_full() { 
        if (max_count >= 63) return count >= (max_count - 3);
        return count == max_count;
    }
    inline bool is_max_vbuf(){return max_count == MAX_VBUF_CAP;}

    /* -------------------------------------------------------------- */
    // get/update adjacency list info
    inline vid_t* get_adjlist() { 
        return (vid_t*)(&count + 1); 
    }

    inline void add_nebr(vid_t nebr) { 
        if(count >= max_count) logstream(LOG_FATAL) << "count >= max_count, " << count << " " << max_count << endl;
        assert(count < max_count);
        vid_t* adj_list1 = get_adjlist();
        adj_list1[count++] = nebr;
    }

    inline void add_nebrs(vid_t* nebrs, blk_deg_t cnt) { 
        if(!cnt) return;
        if(count + cnt > max_count) logstream(LOG_FATAL) << "count + cnt > max_count, " << count << " " << cnt << " " << max_count << endl;
        assert(count + cnt <= max_count);
        vid_t* adj_list1 = get_adjlist();
        memcpy(adj_list1+count, nebrs, cnt * sizeof(vid_t));
        count += cnt;
        // for(blk_deg_t i = 0; i < cnt; i++){
        //     adj_list1[count++] = nebrs[i];
        // }
    }

    inline void clear(){
        size_t size = sizeof(buffer_t) + count * sizeof(vid_t);
        memset(&max_count, 0, size);
    }

    inline void print_adjlist(){
        vid_t* adj_list1 = get_adjlist();
        cout << "print_adjlist of vbuf, count = " << count << ": ";
        for(blk_deg_t i = 0; i < count; i++){
            cout << adj_list1[i] << ", ";
        }
        cout << endl;
    }

    inline void assert_valid(){
        if(!max_count || count > max_count || max_count > MAX_VBUF_CAP) 
            logstream(LOG_FATAL) << "vbuf_count vbuf_maxcount = " << count << ", " << max_count << endl;
        assert(max_count && count <= max_count && max_count <= MAX_VBUF_CAP);
    }
};

class pblock_t{//: public buffer_t{ // base block 32B ~ 4KB
private:
    // pblock_t* next;
    offset_t offset;
    blk_deg_t max_count;
    blk_deg_t count;

public:
    /* -------------------------------------------------------------- */
    // used for recover next links between per-vertex pblocks
    // inline void add_next(pblock_t* ptr) {next = ptr; }
    // inline pblock_t* get_next() { return next; }
    inline void add_next(pblock_t* ptr) { 
        if (ptr == NULL) offset = FIRST_BLOCK_MARK;
        else offset = ptr - this; 
    }
    inline pblock_t* get_next() { 
        if (offset == FIRST_BLOCK_MARK) return NULL;
        return this + offset;
    }

    /* -------------------------------------------------------------- */
    // get/set pblock info
    inline blk_deg_t get_max_nebrcount() { return max_count;}
	inline void set_max_nebrcount(blk_deg_t degree) {max_count = degree;}
    inline blk_deg_t get_nebrcount() { return count;}
	inline void set_nebrcount(blk_deg_t degree) {count = degree;}
    inline bool is_full(){return count == max_count;}

    /* -------------------------------------------------------------- */
    // get/update adjlist info
    inline vid_t* get_adjlist() { 
        return (vid_t*)(&count + 1); 
    }

    inline void add_nebr(vid_t nebr) { 
        if(count >= max_count) logstream(LOG_FATAL) << "count >= max_count, " << count << " " << max_count << endl;
        assert(count < max_count);
        vid_t* adj_list1 = get_adjlist();
        adj_list1[count++] = nebr;
    }

    inline void add_nebrs(vid_t* nebrs, blk_deg_t cnt) { 
        if(!cnt) return;
        if(count + cnt > max_count) logstream(LOG_FATAL) << "count + cnt > max_count, " << count << " " << cnt << " " << max_count << endl;
        assert(count + cnt <= max_count);
        vid_t* adj_list1 = get_adjlist();
        memcpy(adj_list1+count, nebrs, cnt * sizeof(vid_t));
        // pmem_memcpy_nodrain(adj_list1+count, nebrs, cnt * sizeof(vid_t));
        count += cnt;
        // for(blk_deg_t i = 0; i < cnt; i++){
        //     adj_list1[count++] = nebrs[i];
        // }
    }

    void print_adjlist(){
        vid_t* adj_list1 = get_adjlist();
        cout << "print_adjlist of pblk, count = " << count << ": ";
        for(blk_deg_t i = 0; i < count; i++){
            cout << adj_list1[i] << ", ";
        }
        cout << endl;
    }

    inline void assert_valid(){
        if(!max_count || count > max_count) 
            logstream(LOG_FATAL) << "pblk_count pblk_maxcount = " << count << ", " << max_count << endl;
        assert(max_count && count <= max_count);
    }
};
