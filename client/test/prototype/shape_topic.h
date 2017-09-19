#ifndef _DDS_XRCE_SHAPE_TOPIC_H_
#define _DDS_XRCE_SHAPE_TOPIC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "micrortps/client/topic.h"

typedef struct ShapeTopic
{
    uint32_t color_length;
    char* color;
    uint32_t x;
    uint32_t y;
    uint32_t size;

} ShapeTopic;

//callbacks for serialization
void serialize_shape_topic(SerializedBufferHandle*  writer, const void* topic);
void deserialize_shape_topic(SerializedBufferHandle*  reader, DynamicBuffer* dynamic_buffer, void* topic);
uint32_t size_of_shape_topic(const void* topic);

void print_shape_topic(ShapeTopic* topic);

#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_SHAPE_TOPIC_H_