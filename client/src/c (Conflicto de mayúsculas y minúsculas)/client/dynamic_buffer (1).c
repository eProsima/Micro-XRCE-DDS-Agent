#include "micrortps/client/dynamic_buffer.h"

#include <stdlib.h>

void init_memory_buffer(DynamicBuffer* buffer, uint32_t size)
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
}