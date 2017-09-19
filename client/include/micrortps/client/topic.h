#ifndef _DDS_XRCE_TOPIC_H_
#define _DDS_XRCE_TOPIC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "micrortps/client/mini_cdr.h"
#include "micrortps/client/dynamic_buffer.h"

typedef struct Topic
{
    char name[256];

    void (*serialize)(SerializedBufferHandle*  writer, const void* topic);
    void (*deserialize)(SerializedBufferHandle*  reader, DynamicBuffer* dynamic_buffer, void* topic);
    uint32_t (*size_of)(const void* topic);

} Topic;

#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_TOPIC_H_