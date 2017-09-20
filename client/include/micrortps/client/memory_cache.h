#ifndef _DDS_XRCE_MEMORY_CACHE_H_
#define _DDS_XRCE_MEMORY_CACHE_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>

typedef struct MemoryCache
{
    void* memory;
    uint32_t size;
    uint32_t memory_alloc;

} MemoryCache;

void init_memory_cache(MemoryCache* cache, uint32_t size);
void* request_memory_cache(MemoryCache* cache, uint32_t size);
void* reset_memory_cache(MemoryCache* cache, uint32_t size);
void free_memory_cache(MemoryCache* cache);

#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_MEMORY_CACHE_H_
