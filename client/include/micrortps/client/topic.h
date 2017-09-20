#ifndef _DDS_XRCE_TOPIC_H_
#define _DDS_XRCE_TOPIC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "micrortps/client/mini_cdr.h"

typedef struct Topic
{
    char name[256];

    void (*serialize)(SerializedBufferHandle*  writer, const void* topic, void* callback_object);
    void* (*deserialize)(SerializedBufferHandle*  reader, void* callback_object);
    uint32_t (*size_of)(const void* topic);

} Topic;

#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_TOPIC_H_