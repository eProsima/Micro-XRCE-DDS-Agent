#ifndef _DDS_XRCE_MESSAGE_H_
#define _DDS_XRCE_MESSAGE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "micrortps/client/mini_cdr.h"
#include "micrortps/client/memory_cache.h"
#include "micrortps/client/xrce_spec.h"

typedef struct MessageCallback
{
    //serialize callbacks
    int (*on_initialize_message)(MessageHeaderSpec* header, void* data);

    //deserialize callbacks
    int (*on_message_header)(const MessageHeaderSpec* header, void* data);
    void (*on_submessage_header)(const SubmessageHeaderSpec* header, void* data);

    void (*on_create_resource)(const CreatePayloadSpec* payload, void* data);
    void (*on_delete_resource)(const DeletePayloadSpec* payload, void* data);
    void (*on_status)(const StatusPayloadSpec* payload, void* data);
    void (*on_write_data)(const WriteDataPayloadSpec* payload, void* data);
    void (*on_read_data)(const ReadDataPayloadSpec* payload, void* data);
    void (*on_data)(const DataPayloadSpec* payload, void* data);

    void* data;

} MessageCallback;

typedef struct MessageManager
{
    uint8_t* out_buffer; //64 bytes minimun
    uint8_t* in_buffer; //64 bytes minimun

    SerializedBufferHandle writer;
    SerializedBufferHandle reader;

    MemoryCache cache;

    MessageCallback callback;

} MessageManager;

void init_message_manager(MessageManager* message_manager, uint8_t* out_buffer, uint32_t out_buffer_size,
        uint8_t* in_buffer, uint32_t in_buffer_size, MessageCallback callback);
void destroy_message_manager(MessageManager* message_manager);
void remove_cache(MessageManager* message_manager);

//For writing
uint32_t add_create_submessage(MessageManager* message_manager, const CreatePayloadSpec* payload);
uint32_t add_delete_submessage(MessageManager* message_manager, const DeletePayloadSpec* payload);
uint32_t add_status_submessage(MessageManager* message_manager, const StatusPayloadSpec* payload);
uint32_t add_write_data_submessage(MessageManager* message_manager, const WriteDataPayloadSpec* payload);
uint32_t add_read_data_submessage(MessageManager* message_manager, const ReadDataPayloadSpec* payload);
uint32_t add_data_submessage(MessageManager* message_manager, const DataPayloadSpec* payload);

//For reading
int parse_message(MessageManager* message_manager, uint32_t message_length);


#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_MESSAGE_H_