#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h> 
#include <sys/stat.h>

#include "nvm/nvm-common.hpp"
 
#if !defined(MAP_HUGE_2MB)
#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#endif

size_t fsize(const std::string& fname){
    struct stat st;
    if (0 == stat(fname.c_str(), &st)) {
        return st.st_size;
    }
    perror("stat issue");
    return -1L;
}

size_t alloc_and_read(char** buf, const std::string& fname, bool is_pmem) {
    FILE* file = fopen(fname.c_str(), "rb");
    size_t size = fsize(fname);
    assert(file && size);

    void* bufptr = 0;
    if (is_pmem) {
        bufptr = pmem_alloc(fname.c_str(), size);
    } else {
        bufptr = (char*)mmap(0, size, PROT_READ|PROT_WRITE,
                MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0);
        
        if (MAP_FAILED == bufptr) {
            std::cout << "huge page alloc failed while load recovery dir, so use malloc" << std::endl;
            bufptr = (char*)malloc(size);
        }
    }

    if (size != fread(bufptr, sizeof(char), size, file)) {
        assert(0);
    }
    *buf = (char*)bufptr;
    fclose(file);
    
    return size;
}

inline size_t alloc_mem_dir(const std::string& idirname, std::string odirname, char** buf)
{
    struct dirent *ptr;
    DIR *dir;
    std::string filename;
    size_t size = 0;
    size_t total_size = 0;
    //allocate accuately
    dir = opendir(idirname.c_str());
    int file_count = 0;
    while (NULL != (ptr = readdir(dir))) {
        if (ptr->d_name[0] == '.') continue;
        filename = idirname + "/" + std::string(ptr->d_name);
        size = fsize(filename);
        total_size += size;
        file_count++;
    }
    closedir(dir);

    void* local_buf = 0;
    if(odirname != ""){
        std::string local_buf_file = odirname + "local_buf.txt";
        local_buf = pmem_alloc(local_buf_file.c_str(), total_size);
    } else {
        local_buf = mmap(0, total_size, PROT_READ|PROT_WRITE,
                MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB|MAP_HUGE_2MB, 0, 0);

        if (MAP_FAILED == local_buf) {
            std::cout << "huge page alloc failed while reading input dir, so use malloc" << std::endl;
            local_buf =  malloc(total_size);
        }
    }    

    *buf = (char*)local_buf;
    local_buf_size = total_size;
    return total_size;
}

inline size_t read_bin_dir(const std::string& idirname, char* buf){
    //Read input dir files
    struct dirent *ptr;
    FILE* file = 0;
    int file_count = 0;
    std::string filename;
    
    size_t size = 0;
    size_t total_size = 0;
    char* bufptr;

    std::cout << "Reading dir files... " << std::endl;
    DIR* dir = opendir(idirname.c_str());
    while (NULL != (ptr = readdir(dir))) {
        if (ptr->d_name[0] == '.') continue;
        filename = idirname + "/" + std::string(ptr->d_name);
        file_count++;
        
        std::cout << "  " << (idirname + "/" + std::string(ptr->d_name)) << std::endl;
        file = fopen((idirname + "/" + std::string(ptr->d_name)).c_str(), "rb");
        assert(file != 0);
        size = fsize(filename);
        bufptr = buf + total_size;
        if (size!= fread(bufptr, sizeof(char), size, file)) {
            assert(0);
        }
        total_size += size;
    }
    closedir(dir);
    std::cout << "Reading "  << file_count  << " files, total size = " << total_size << std::endl;
    return total_size;
}

size_t alloc_and_read_dir(std::string idirname, std::string odirname, char **buf){
    //allocate accuately and read files
    size_t total_size = alloc_mem_dir(idirname, odirname, buf); // 输入文件读入 buf
    if (total_size != read_bin_dir(idirname, *buf)) {
        std::cout << "Read wrong size!" << std::endl;
        assert(0);
    }
    return total_size;
}

int rm_dir(std::string dir_full_path){    
    DIR* dirp = opendir(dir_full_path.c_str());    
    if(!dirp){
        return -1;
    }
    struct dirent *dir;
    struct stat st;
    while((dir = readdir(dirp)) != NULL){
        if(strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..") == 0){
            continue;
        }    
        std::string sub_path = dir_full_path + '/' + dir->d_name;
        if(lstat(sub_path.c_str(),&st) == -1){
            // Log("rm_dir:lstat ",sub_path," error");
            std::cout << "rm_dir:lstat " << sub_path << " error" << std::endl;
            continue;
        }    
        if(S_ISDIR(st.st_mode)){
            if(rm_dir(sub_path) == -1){ // 如果是目录文件，递归删除
                closedir(dirp);
                return -1;
            }
            rmdir(sub_path.c_str());
        }
        else if(S_ISREG(st.st_mode)) {
            unlink(sub_path.c_str());     // 如果是普通文件，则unlink
        }
        else{
            // Log("rm_dir:st_mode ",sub_path," error");
            std::cout << "rm_dir:st_mode " << sub_path << " error" << std::endl;
            continue;
        }
    }
    if(rmdir(dir_full_path.c_str()) == -1){//delete dir itself.
        closedir(dirp);
        return -1;
    }
    closedir(dirp);
    return 0;
}

#endif