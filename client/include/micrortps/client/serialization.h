#ifndef _DDS_XRCE_SERIALIZATION_H_
#define _DDS_XRCE_SERIALIZATION_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "micrortps/client/xrce_spec.h"
#include "micrortps/client/mini_cdr.h"
#include "micrortps/client/memory_cache.h"


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
void serialize_data_mode(SerializedBufferHandle* buffer, const DataModeSpec* data_mode);

//object variants
void serialize_object_variant_data_writer(SerializedBufferHandle* buffer, const DataWriterSpec* data_writer);
void serialize_object_variant_data_reader(SerializedBufferHandle* buffer, const DataReaderSpec* data_reader);
void serialize_object_variant_publisher(SerializedBufferHandle* buffer, const PublisherSpec* publisher);
void serialize_object_variant_subscriber(SerializedBufferHandle* buffer, const SubscriberSpec* subscriber);

//status
void serialize_result_status(SerializedBufferHandle* buffer, const ResultStatusSpec* result);

//samples
void serialize_sample(SerializedBufferHandle* buffer, const SampleSpec* sample);
void serialize_sample_data(SerializedBufferHandle* buffer, const SampleDataSpec* data);
void serialize_sample_info(SerializedBufferHandle* buffer, const SampleInfoSpec* info);

//util
void serialize_object_id(SerializedBufferHandle* buffer, uint_least24_t object_id);

//DESERIALIZATION
//headers
void deserialize_submessage_header(SerializedBufferHandle* buffer, MemoryCache* cache, SubmessageHeaderSpec* submessage_header);
void deserialize_message_header(SerializedBufferHandle* buffer, MemoryCache* cache, MessageHeaderSpec* message_header);

//payloads
void deserialize_create_payload(SerializedBufferHandle* buffer, MemoryCache* cache, CreatePayloadSpec* payload);
void deserialize_delete_payload(SerializedBufferHandle* buffer, MemoryCache* cache, DeletePayloadSpec* payload);
void deserialize_status_payload(SerializedBufferHandle* buffer, MemoryCache* cache, StatusPayloadSpec* payload);
void deserialize_write_data_payload(SerializedBufferHandle* buffer, MemoryCache* cache, WriteDataPayloadSpec* payload);
void deserialize_read_data_payload(SerializedBufferHandle* buffer, MemoryCache* cache, ReadDataPayloadSpec* payload);
void deserialize_data_payload(SerializedBufferHandle* buffer, MemoryCache* cache, DataPayloadSpec* payload);

//switches
void deserialize_object_kind(SerializedBufferHandle* buffer, MemoryCache* cache, ObjectKindSpec* object);
void deserialize_data_mode(SerializedBufferHandle* buffer, MemoryCache* cache, DataModeSpec* data_mode);

//object variants
void deserialize_object_variant_data_writer(SerializedBufferHandle* buffer, MemoryCache* cache, DataWriterSpec* data_writer);
void deserialize_object_variant_data_reader(SerializedBufferHandle* buffer, MemoryCache* cache, DataReaderSpec* data_reader);
void deserialize_object_variant_publisher(SerializedBufferHandle* buffer, MemoryCache* cache, PublisherSpec* publisher);
void deserialize_object_variant_subscriber(SerializedBufferHandle* buffer, MemoryCache* cache, SubscriberSpec* subscriber);

//status
void deserialize_result_status(SerializedBufferHandle* buffer, MemoryCache* cache, ResultStatusSpec* result);

//samples
void deserialize_sample(SerializedBufferHandle* buffer, MemoryCache* cache, SampleSpec* sample);
void deserialize_sample_data(SerializedBufferHandle* buffer, MemoryCache* cache, SampleDataSpec* data);
void deserialize_sample_info(SerializedBufferHandle* buffer, MemoryCache* cache, SampleInfoSpec* info);

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
int size_of_data_mode(const DataModeSpec* data_mode);

//object variants
int size_of_object_variant_data_writer(const DataWriterSpec* data_writer);
int size_of_object_variant_data_reader(const DataReaderSpec* data_reader);
int size_of_object_variant_publisher(const PublisherSpec* publisher);
int size_of_object_variant_subscriber(const SubscriberSpec* subscriber);

//status
int size_of_result_status(const ResultStatusSpec* result);

//samples
int size_of_sample(const SampleSpec* sample);
int size_of_sample_data(const SampleDataSpec* data);
int size_of_sample_info(const SampleInfoSpec* info);

//util
int size_of_object_id(uint_least24_t object_id);


#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_SERIALIZATION_H_