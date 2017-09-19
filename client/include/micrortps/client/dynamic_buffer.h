#ifndef _DDS_XRCE_DYNAMIC_BUFFER_H_
#define _DDS_XRCE_DYNAMIC_BUFFER_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>

typedef struct DynamicBuffer
{
    uint8_t* memory;
    uint32_t size;
    uint32_t memory_alloc;

} DynamicBuffer;

void init_memory_buffer(DynamicBuffer* buffer, uint32_t size);
void* request_memory_buffer(DynamicBuffer* buffer, uint32_t size);
void reset_memory_buffer(DynamicBuffer* buffer);
void free_memory_buffer(DynamicBuffer* buffer);

#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_DYNAMIC_BUFFER_H_
