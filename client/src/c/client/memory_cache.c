#include "micrortps/client/memory_cache.h"

#include <stdlib.h>
#include <string.h>

#ifdef _DEBUG_
    #include <stdio.h>
#endif

/*void init_memory_buffer(DynamicBuffer* buffer, uint32_t size)
{
    buffer->memory = malloc(size);
    buffer->memory_alloc = size;
    buffer->size = 0;
}

void* request_memory_buffer(DynamicBuffer* buffer, uint32_t size)
{
    if(buffer->memory_alloc < buffer->size + size)
    {
        buffer->memory_alloc = (buffer->memory_alloc * 2 < buffer->size + size)
                ? buffer->size + size : buffer->memory_alloc * 2;

        void* new_memory = malloc(buffer->memory_alloc);
        memcpy(new_memory, buffer->memory, buffer->size);
        free(buffer->memory);

        buffer->memory = new_memory;
    }

    uint32_t init = buffer->size;
    buffer->size += size;
    return buffer->memory + init;
}

void reset_memory_buffer(DynamicBuffer* buffer)
{
    buffer->size = 0;
}

void free_memory_buffer(DynamicBuffer* buffer)
{
    free(buffer->memory);
}*/

void init_memory_cache(MemoryCache* cache, uint32_t memory_size)
{
    cache->memory = malloc(memory_size);
    cache->memory_alloc = memory_size;
    cache->size = 0;
}

void* request_memory_cache(MemoryCache* cache, uint32_t size)
{
    #ifdef _DEBUG_
    if(cache->size + size > cache->memory_alloc)
    {
        printf("ERROR: Memory cache oveflow.");
        return NULL;
    }
    #endif

    uint32_t init = cache->size;
    cache->size += size;
    return ((uint8_t*)cache->memory) + init;
}

void* reset_memory_cache(MemoryCache* cache, uint32_t memory_size)
{
    if(memory_size > cache->memory_alloc)
    {
        free(cache->memory);
        cache->memory = malloc(memory_size);
        cache->memory_alloc = memory_size;
    }

    cache->size = 0;
    return cache->memory;
}

void free_memory_cache(MemoryCache* cache)
{
    free(cache->memory);
}