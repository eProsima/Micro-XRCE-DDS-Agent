#ifndef _DDS_XRCE_MESSAGE_H_
#define _DDS_XRCE_MESSAGE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "xrce_spec.h"
#include "mini_cdr.h"

typedef struct MessageCallback
{
    void (*message_header)(const MessageHeaderSpec* header);
    void (*submessage_header)(const SubmessageHeaderSpec* header);

    void (*create_resource)(const CreatePayloadSpec* payload);
    void (*delete_resource)(const DeletePayloadSpec* payload);
    void (*status)(const StatusPayloadSpec* payload);
    void (*write_data)(const WriteDataPayloadSpec* payload);
    void (*read_data)(const ReadDataPayloadSpec* payload);
    void (*data)(const DataPayloadSpec* payload);

} MessageCallback;



//For writing
void init_message(SerializedBufferHandle* writer, const MessageHeaderSpec* message_header);

int add_create_submessage(SerializedBufferHandle* writer, const CreatePayloadSpec* payload);
int add_delete_submessage(SerializedBufferHandle* writer, const DeletePayloadSpec* payload);
int add_status_submessage(SerializedBufferHandle* writer, const StatusPayloadSpec* payload);
int add_write_data_submessage(SerializedBufferHandle* writer, const WriteDataPayloadSpec* payload);
int add_read_data_submessage(SerializedBufferHandle* writer, const ReadDataPayloadSpec* payload);
int add_data_submessage(SerializedBufferHandle* writer, const DataPayloadSpec* payload);

//For reading
int parse_message(SerializedBufferHandle* reader, const MessageCallback* callback);


#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_MESSAGE_H_