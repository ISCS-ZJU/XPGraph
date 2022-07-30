#ifndef NVM_COMMON
#define NVM_COMMON

#include <libpmem.h>
#include <iostream>
#include <cstring>

/* ---------------------------------------------------------------------- */
// used for memory usage
size_t local_buf_size = 0;
size_t elog_size = 0;

#define CACHE_LINE_SIZE    64
#define XPLINE_SIZE    256

// obtain the starting address of a cache line
#define getline(addr) \
(((unsigned long long)(addr)) & (~(unsigned long long)(CACHE_LINE_SIZE-1)))

// check if address is aligned at line boundary
#define isaligned_atline(addr) \
(!(((unsigned long long)(addr)) & (unsigned long long)(CACHE_LINE_SIZE-1)))

/* -------------------------------------------------------------- */
// ALLOC -- use pmem_map_file
static inline void* pmem_alloc(const char* filepath, size_t size){
    size_t mapped_len;
    int is_pmem;
    void* addr = pmem_map_file(filepath, size, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem);
    if (!addr)  {
        std::cout << "Could not pmem_map_file for :" << filepath << " error: " << strerror(errno) << std::endl;
        assert(0);
    }
    if (!is_pmem){
        std::cout << "pmem_map_file for :" << filepath << " error: not in pmem" << std::endl;
        assert(0);
    }
    if (mapped_len != size){
        std::cout << "pmem_map_file for :" << filepath << " error: mapped_len != size" << std::endl;
        assert(0);
    }
    // memset(addr, 0, size);  //pre touch, 消除page fault影响
    return addr;
}

/* -------------------------------------------------------------- */
// FLUSH -- use clwb and sfence
static inline void clwb(void * addr){ 
  asm volatile("clwb %0": :"m"(*((char *)addr))); 
} 

static inline void clwb2(void *start, void *end){
  clwb(start);
  if (getline(start) != getline(end)) {
    clwb(end);
  }
}

static inline void clwbmore(void *start, void *end){ 
  unsigned long long start_line= getline(start);
  unsigned long long end_line= getline(end);
  do {
    clwb((char *)start_line);
    start_line += CACHE_LINE_SIZE;
  } while (start_line <= end_line);
}

static inline void sfence(void){ 
  asm volatile("sfence"); 
}

/* -------------------------------------------------------------- */
/**
 * copy src line to dest line using movntdq
 *
 * Both src and dest are 64 byte lines and aligned
 *
 * @param dest  destination line
 * @param src   src line
 */
static inline
void writeLineMOVNT(char *dest, char *src)
{   
  asm volatile(
    "movdqa   %4, %%xmm0    \n\t"
    "movdqa   %5, %%xmm1    \n\t"
    "movdqa   %6, %%xmm2    \n\t"
    "movdqa   %7, %%xmm3    \n\t"
    "movntdq  %%xmm0, %0    \n\t"
    "movntdq  %%xmm1, %1    \n\t"
    "movntdq  %%xmm2, %2    \n\t"
    "movntdq  %%xmm3, %3    \n\t" 
    : "=m"(dest[ 0]), "=m"(dest[16]), "=m"(dest[32]), "=m"(dest[48])
    :  "m"(src[ 0]),   "m"(src[16]),   "m"(src[32]),   "m"(src[48])
    : "xmm0", "xmm1", "xmm2", "xmm3",
      "memory");
}

/* -------------------------------------------------------------- */
// PREFETCH -- use prefetch instructions by default.
#define prefetcht0(mem_var)     \
        __asm__ __volatile__ ("prefetcht0 %0": :"m"(mem_var))
#define prefetchnta(mem_var)    \
        __asm__ __volatile__ ("prefetchnta %0": :"m"(mem_var))

#define pref(mem_var)      prefetcht0(mem_var)
#define prefnta(mem_var)   prefetchnta(mem_var)

#define ptouch(mem_var) \
        __asm__ __volatile__ ("movl %0, %%eax": :"m"(mem_var):"%eax")



/* -------------------------------------------------------------- */
/*
ntstore [fast20-guide]
*/

/* ymm0: 256-bit register (requires AVX support)
 * vbroadcastsd: VEX.256-bit version (r[0] = r[1] = r[2] = r[3] = v)
 */
// #ifdef __KERNEL__
#if 0
  #include <asm/fpu/api.h>
  #define KERNEL_BEGIN \
	 kernel_fpu_begin();
  #define KERNEL_END \
	 kernel_fpu_end();
#else
  #define KERNEL_BEGIN do { } while (0);
  #define KERNEL_END do { } while (0);
#endif

/* Assembly instructions utilize the following registers:
 * rsi: memory address
 * rax, rdx, rcx, r8d and r9d: timing
 * rdx: populating cache-lines
 * ymm0: streaming instructions
 */
#define REGISTERS "rsi", "rax", "rdx", "rcx", "r8", "r9", "ymm0"
#define REGISTERS_NONSSE "rsi", "rax", "rdx", "rcx", "r8", "r9"

#define LOAD_VALUE              "vbroadcastsd %[value], %%ymm0 \n"
#define LOAD_ADDR               "mov %[memarea], %%rsi \n" \
                                "mfence \n"

#define FLUSH_CACHE_LINE        "clflush 0*32(%%rsi) \n" \
                                "clflush 2*32(%%rsi) \n" \
                                "clflush 4*32(%%rsi) \n" \
                                "clflush 6*32(%%rsi) \n" \
                                "clflush 8*32(%%rsi) \n" \
                                "clflush 10*32(%%rsi) \n" \
                                "clflush 12*32(%%rsi) \n" \
                                "clflush 14*32(%%rsi) \n" \
                                "mfence \n"

#define CLEAR_PIPELINE          "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n" \
                                "nop \nnop \nnop \nnop \nnop \nnop \n"

/* rdtscp: reads current timestamp to EDX:EAX and also sets ECX
 * higher 32-bits of RAX, RDX and RCX are cleared
 */
#define TIMING_BEG              "rdtscp \n" \
                                "lfence \n" \
                                "mov %%edx, %%r9d \n" \
                                "mov %%eax, %%r8d \n"

/* r9d = old EDX
 * r8d = old EAX
 * Here is what we do to compute t1 and t2:
 * - RDX holds t2
 * - RAX holds t1
 */
#define TIMING_END              "mfence \n" \
                                "rdtscp \n" \
                                "lfence \n" \
                                "shl $32, %%rdx \n" \
                                "mov %%eax, %%edx \n" \
                                "mov %%r9d, %%eax \n" \
                                "shl $32, %%rax \n" \
                                "mov %%r8d, %%eax \n" \
                                "mov %%rax, %[t1] \n" \
                                "mov %%rdx, %[t2] \n"

#define TIMING_END_NOFENCE      "rdtscp \n" \
                                "shl $32, %%rdx \n" \
                                "or  %%rax, %%rdx \n" \
                                "mov %%r9d, %%eax \n" \
                                "shl $32, %%rax \n" \
                                "or  %%r8, %%rax \n" \
                                "mov %%rax, %[t1] \n" \
                                "mov %%rdx, %[t2] \n"

int64_t nstore_64byte_fence(char *addr) {
    int64_t t1 = 0, t2 = 0;
    int64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    /*
     * vmovntpd: 32-byte non-temporal store (check below)
     * https://software.intel.com/en-us/node/524246
     */
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        //FLUSH_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovntpd %%ymm0, 0*32(%%rsi) \n"
        "vmovntpd %%ymm0, 1*32(%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

int64_t store_64byte_clwb(char *addr) {
    int64_t t1 = 0, t2 = 0;
    int64_t value = 0xC0FFEEEEBABE0000;
    KERNEL_BEGIN
    asm volatile(LOAD_ADDR
        LOAD_VALUE
        // LOAD_CACHE_LINE
        CLEAR_PIPELINE
        TIMING_BEG
        "vmovdqa %%ymm0, 0*32(%%rsi) \n"
        "vmovdqa %%ymm0, 1*32(%%rsi) \n"
        "clwb (%%rsi) \n"
        TIMING_END
        : [t1] "=r" (t1), [t2] "=r" (t2)
        : [memarea] "r" (addr), [value] "m" (value)
        : REGISTERS);
    KERNEL_END
    return t2 - t1;
}

#endif /* NVM_COMMON */
