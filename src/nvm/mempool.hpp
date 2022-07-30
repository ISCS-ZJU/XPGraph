#ifndef NVM_MEM_POOL
#define NVM_MEM_POOL

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h> 
#include <string.h>
#include <malloc.h>
#include <libpmem.h>
#include <mutex>

#include "utils/basic_includes.hpp"

/* -------------------------------------------------------------- */

/**
 * mempool_t: allocate memory using malloc-like calls 
 *          then manage the memory by itself
 */
class mempool_t{
 private:
   long long   mempool_align;
   long long   mempool_size;
   char * mempool_start;
   char * mempool_cur;
   char * mempool_end;
   char * mempool_free_block;
   const char * mempool_name;

   std::mutex alloc_mutex;

 public:
   // initialize the memory pool.
   void init(char *start, long long size, long long align, const char * name){
      mempool_align = align;
      mempool_size = size;
      mempool_start = start;

      mempool_cur = mempool_start;
      mempool_end = mempool_start + size;
      mempool_free_block = NULL;
      mempool_name= name;
   }

   void clear(){
      mempool_cur = mempool_start;
      mempool_free_block = NULL;
   }

   // print all the parameters and addresses of the memory pool 
   void print_params(FILE *fp=stdout){
      fprintf (fp, "%s\n", mempool_name);
      fprintf (fp, "mempool_align=%lld\n", mempool_align);
      fprintf (fp, "mempool_size=%lld\n", mempool_size);
      fprintf (fp, "mempool_start=%p\n", mempool_start);
      fprintf (fp, "mempool_cur=%p\n", mempool_cur);
      fprintf (fp, "mempool_end=%p\n", mempool_end);
      fprintf (fp, "mempool_free_block=%p\n\n", mempool_free_block);
   }

   // obtain the starting address of the memory pool
   char * get_base(){return mempool_start;}
   size_t get_size(){return mempool_size;}

   size_t print_usage(){
       long long  used= (mempool_cur - mempool_start);
       long long  ff= 0;
       for(char *p = mempool_free_block; p; p= *((char **)p))
           ff ++;

       printf("%s: total %.1lfGB, use %.1lfGB, among which %lld free nodes\n",
               mempool_name, ((double)mempool_size)/GB, ((double)used)/GB, ff);
       return used;
   }

   inline size_t get_used(){
      return mempool_cur - mempool_start;
   }
   
   inline size_t get_remained(){
      long long  used= (mempool_cur - mempool_start);
      long long  remained = (mempool_size - used);
      return remained;
   }

 public:
   /**
   * allocate a piece of memory from the memory pool
   * Note: the memory pool will be allocated from the end.
   * Make sure the size is aligned.  The code will not check this.
   */
   void * alloc(unsigned long long size)
   {
      // alloc_mutex.lock();
      std::lock_guard<std::mutex> guard(alloc_mutex);
       if (mempool_cur + size <= mempool_end) {
         register char *p;
         p = mempool_cur;
         mempool_cur += size;
         // memset(p, 0, size);  //pre touch, 消除page fault影响
         return (void *) p;
       }
      // alloc_mutex.unlock();
       return 0;
      //  fprintf (stderr, "%s alloc - run out of memory!\n", mempool_name);
      //  exit (1);
   }

   // free memory that is previously allocated using alloc()
   void free(void *p) { }

   // allocate a block of align size
   inline void * alloc_block(){
      if (mempool_free_block) {
         register char *p;
         p = mempool_free_block;
         mempool_free_block = *((char **)p);
         return (void *) p;
      }
      else {
         return alloc(mempool_align);
      }
   }

   // free a block and append it into a linked list for future alloc_node
   void free_block(void *p){
      *((char **)p) = mempool_free_block;
      mempool_free_block = (char *)p;
   }

   // print the nodes on the free linked list
   void print_free_blocks(){
      char *p = mempool_free_block;

      printf ("%s free blocks:\n", mempool_name);
      while (p != NULL) {
         printf ("%p -> ", p);
         p = *((char **)p);
      }
      printf ("nil\n\n");
   }

}; // mempool

#endif /* NVM_MEM_POOL */