#pragma once

#include "utils/basic_includes.hpp"

class  edge_t {
 public:
    vid_t src;
    vid_t dst;
};

class edgelog_t
{
public:
    edge_t* data; // circular buffer for edge logs
    index_t capacity;
    index_t head; // position of write log
    index_t marker; // unbuffered range = [elog_marker, elog_head)
    index_t tail; // buffering range = [elog_tail, elog_marker)
    index_t efree; // buffered range = [elog_efree, elog_tail), archived = [xx, elog_efree) -> can be overwritten

    edgelog_t():data(0), capacity(0){
        // capacity = ELOG_CAP;
        // size_t size =  capacity * sizeof(edge_t);
        // std::string elog_file = NVMPATH0 + "edge_log.txt";
        // data = (edge_t*)pmem_alloc(elog_file.c_str(), size);
        // memset(data, 0, size); 
        // logstream(LOG_DEBUG) << "Edgelog alloced " << capacity << " * " << sizeof(edge_t) << " = " << (size >> 20) << "MB on PMEM, tid = " << omp_get_thread_num() << std::endl;
        head = marker = tail = efree = 0;
    }

    edgelog_t(edge_t* buf, index_t count):data(buf), capacity(count){
        marker = tail = efree = 0;
        head = capacity;
    }

    ~edgelog_t(){
        // size_t size =  capacity * sizeof(edge_t);
        // pmem_unmap(data, size);
        // data = NULL;
        head = marker = tail = efree = capacity = 0;
    }
};
