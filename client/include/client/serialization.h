#ifndef _DDS_XRCE_SERIALIZATION_H_
#define _DDS_XRCE_SERIALIZATION_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "xrce_spec.h"
#include "mini_cdr.h"


typedef struct DynamicBuffer
{
    uint8_t* memory;
    uint32_t size;
    uint32_t memory_alloc;

} DynamicBuffer;


//SERIALIZATION
//headers
void serialize_message_header(SerializedBufferHandle* buffer, const MessageHeaderSpec* message_header);
void serialize_submessage_header(SerializedBufferHandle* buffer, const SubmessageHeaderSpec* submessage_header);

//payloads
void serialize_create_payload(SerializedBufferHandle* buffer, const CreatePayloadSpec* payload);
void serialize_delete_payload(SerializedBufferHandle* buffer, const DeletePayloadSpec* payload);
void serialize_status_payload(SerializedBufferHandle* buffer, const StatusPayloadSpec* payload);
void serialize_write_data_payload(SerializedBufferHandle* buffer, const WriteDataPayloadSpec* payload);
void serialize_read_data_payload(SerializedBufferHandle* buffer, const ReadDataPayloadSpec* payload);
void serialize_data_payload(SerializedBufferHandle* buffer, const DataPayloadSpec* payload);

//switches
void serialize_object_kind(SerializedBufferHandle* buffer, const ObjectKindSpec* object);
void serialize_status_kind(SerializedBufferHandle* buffer, const StatusKindSpec* status);
void serialize_data_mode(SerializedBufferHandle* buffer, const DataModeSpec* data_mode);

//object variants
void serialize_object_variant_data_writer(SerializedBufferHandle* buffer, const DataWriterSpec* data_writer);
void serialize_object_variant_data_reader(SerializedBufferHandle* buffer, const DataReaderSpec* data_reader);
void serialize_object_variant_publisher(SerializedBufferHandle* buffer, const PublisherSpec* publisher);
void serialize_object_variant_subscriber(SerializedBufferHandle* buffer, const SubscriberSpec* subscriber);

//status
void serialize_result_status(SerializedBufferHandle* buffer, const ResultStatusSpec* result);
void serialize_status_variant_data_writer(SerializedBufferHandle* buffer, const DataWriterStatusSpec* writer_status);
void serialize_status_variant_data_reader(SerializedBufferHandle* buffer, const DataReaderStatusSpec* reader_status);

//samples
void serialize_sample(SerializedBufferHandle* buffer, const SampleSpec* sample);
void serialize_sample_data(SerializedBufferHandle* buffer, const SampleDataSpec* data);
void serialize_sample_info(SerializedBufferHandle* buffer, const SampleInfoSpec* info);

//util
void serialize_object_id(SerializedBufferHandle* buffer, uint_least24_t object_id);

//DESERIALIZATION
//headers
void deserialize_submessage_header(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SubmessageHeaderSpec* submessage_header);
void deserialize_message_header(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, MessageHeaderSpec* message_header);

//payloads
void deserialize_create_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, CreatePayloadSpec* payload);
void deserialize_delete_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DeletePayloadSpec* payload);
void deserialize_status_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, StatusPayloadSpec* payload);
void deserialize_write_data_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, WriteDataPayloadSpec* payload);
void deserialize_read_data_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, ReadDataPayloadSpec* payload);
void deserialize_data_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataPayloadSpec* payload);

//switches
void deserialize_object_kind(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, ObjectKindSpec* object);
void deserialize_status_kind(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, StatusKindSpec* status);
void deserialize_data_mode(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataModeSpec* data_mode);

//object variants
void deserialize_object_variant_data_writer(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataWriterSpec* data_writer);
void deserialize_object_variant_data_reader(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataReaderSpec* data_reader);
void deserialize_object_variant_publisher(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, PublisherSpec* publisher);
void deserialize_object_variant_subscriber(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SubscriberSpec* subscriber);

//status
void deserialize_result_status(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, ResultStatusSpec* result);
void deserialize_status_variant_data_writer(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataWriterStatusSpec* writer_status);
void deserialize_status_variant_data_reader(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataReaderStatusSpec* reader_status);

//samples
void deserialize_sample(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SampleSpec* sample);
void deserialize_sample_data(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SampleDataSpec* data);
void deserialize_sample_info(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SampleInfoSpec* info);

//util
void deserialize_object_id(SerializedBufferHandle* buffer, uint_least24_t* object_id);


//SIZE_OF
//headers
int size_of_message_header(const MessageHeaderSpec* message_header);
int size_of_submessage_header(const SubmessageHeaderSpec* submessage_header);

//payloads
int size_of_create_payload(const CreatePayloadSpec* payload);
int size_of_delete_payload(const DeletePayloadSpec* payload);
int size_of_status_payload(const StatusPayloadSpec* payload);
int size_of_write_data_payload(const WriteDataPayloadSpec* payload);
int size_of_read_data_payload(const ReadDataPayloadSpec* payload);
int size_of_data_payload(const DataPayloadSpec* payload);

//switches
int size_of_object_kind(const ObjectKindSpec* object);
int size_of_status_kind(const StatusKindSpec* status);
int size_of_data_mode(const DataModeSpec* data_mode);

//object variants
int size_of_object_variant_data_writer(const DataWriterSpec* data_writer);
int size_of_object_variant_data_reader(const DataReaderSpec* data_reader);
int size_of_object_variant_publisher(const PublisherSpec* publisher);
int size_of_object_variant_subscriber(const SubscriberSpec* subscriber);

//status
int size_of_result_status(const ResultStatusSpec* result);
int size_of_status_variant_data_writer(const DataWriterStatusSpec* writer_status);
int size_of_status_variant_data_reader(const DataReaderStatusSpec* reader_status);

//samples
int size_of_sample(const SampleSpec* sample);
int size_of_sample_data(const SampleDataSpec* data);
int size_of_sample_info(const SampleInfoSpec* info);

//util
int size_of_object_id(uint_least24_t object_id);



//UTIL
int align_and_check_size_of_submessage_header(SerializedBufferHandle* buffer);



//MEMORY MANEGEMENT
void init_memory_buffer(DynamicBuffer* buffer, uint32_t size);
void* request_memory_buffer(DynamicBuffer* buffer, uint32_t size);
void reset_memory_buffer(DynamicBuffer* buffer);
void free_memory_buffer(DynamicBuffer* buffer);


#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_SERIALIZATION_H_