#include "micrortps/client/serialization.h"

#include <stdlib.h>


// --------------------------------------------------------------------------
//                                 HEADERS
// --------------------------------------------------------------------------

// Message Header
// 0                8                16              24               31
// +---------------+----------------+----------------+----------------+
// |                          client_key                              |
// +---------------+--------+-------+----------------+----------------+
// |   session_id  |    stream_id   |           sequence_number           |
// +---------------+--------+-------+----------------+----------------+
void serialize_message_header(SerializedBufferHandle* buffer, const MessageHeaderSpec* message_header)
{
    serialize_byte_4(buffer, message_header->client_key);
    serialize_byte(buffer, message_header->session_id);
    serialize_byte(buffer, message_header->stream_id);
    serialize_byte_2_endian(buffer, message_header->sequence_number, BIG_ENDIAN_MODE);
}

void deserialize_message_header(SerializedBufferHandle* buffer, MemoryCache* cache,  MessageHeaderSpec* message_header)
{
    deserialize_byte_4(buffer, &message_header->client_key);
    deserialize_byte(buffer, &message_header->session_id);
    deserialize_byte(buffer, &message_header->stream_id);
    deserialize_byte_2_endian(buffer, &message_header->sequence_number, BIG_ENDIAN_MODE);
}

int size_of_message_header(const MessageHeaderSpec* message_header)
{
    return sizeof(message_header->client_key)
         + sizeof(message_header->session_id)
         + sizeof(message_header->stream_id)
         + sizeof(message_header->sequence_number);
}


// Submessage header
// 0       4       8               16               24               31
// +-------+-------+----------------+----------------+----------------+
// |       id      |      flags     |              length             |
// +-------+-------+----------------+----------------+----------------+
void serialize_submessage_header(SerializedBufferHandle* buffer, const SubmessageHeaderSpec* submessage_header)
{
    serialize_byte(buffer, submessage_header->id);
    serialize_byte(buffer, submessage_header->flags);
    serialize_byte_2_endian(buffer, submessage_header->length, BIG_ENDIAN_MODE);
}

void deserialize_submessage_header(SerializedBufferHandle* buffer, MemoryCache* cache, SubmessageHeaderSpec* submessage_header)
{
    deserialize_byte(buffer, &submessage_header->id);
    deserialize_byte(buffer, &submessage_header->flags);
    deserialize_byte_2_endian(buffer, &submessage_header->length, BIG_ENDIAN_MODE);
}

int size_of_submessage_header(const SubmessageHeaderSpec* submessage_header)
{
    return sizeof(submessage_header->id)
         + sizeof(submessage_header->flags)
         + sizeof(submessage_header->length);
}

// --------------------------------------------------------------------------
//                                  PAYLOADS
// --------------------------------------------------------------------------

void serialize_create_payload(SerializedBufferHandle* buffer, const CreatePayloadSpec* payload)
{
    serialize_byte_4(buffer, payload->request_id);
    serialize_object_id(buffer, payload->object_id);
    serialize_object_kind(buffer, &payload->object);
}

void deserialize_create_payload(SerializedBufferHandle* buffer, MemoryCache* cache, CreatePayloadSpec* payload)
{
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_object_id(buffer, &payload->object_id);
    deserialize_object_kind(buffer, cache, &payload->object);
}

int size_of_create_payload(const CreatePayloadSpec* payload)
{
    return sizeof(payload->request_id)
         + size_of_object_id(payload->object_id)
         + size_of_object_kind(&payload->object);
}

void serialize_delete_payload(SerializedBufferHandle* buffer, const DeletePayloadSpec* payload)
{
    serialize_byte_4(buffer, payload->request_id);
    serialize_object_id(buffer, payload->object_id);;
}

void deserialize_delete_payload(SerializedBufferHandle* buffer, MemoryCache* cache, DeletePayloadSpec* payload)
{
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_object_id(buffer, &payload->object_id);
}

int size_of_delete_payload(const DeletePayloadSpec* payload)
{
    return sizeof(payload->request_id)
         + size_of_object_id(payload->object_id);
}

void serialize_status_payload(SerializedBufferHandle* buffer, const StatusPayloadSpec* payload)
{
    serialize_result_status(buffer, &payload->result);
    serialize_object_id(buffer, payload->object_id);
}

void deserialize_status_payload(SerializedBufferHandle* buffer, MemoryCache* cache, StatusPayloadSpec* payload)
{
    deserialize_result_status(buffer, cache, &payload->result);
    deserialize_object_id(buffer, &payload->object_id);
}

int size_of_status_payload(const StatusPayloadSpec* payload)
{
    return size_of_result_status(&payload->result)
         + size_of_object_id(payload->object_id);
}

void serialize_write_data_payload(SerializedBufferHandle* buffer, const WriteDataPayloadSpec* payload)
{
    serialize_byte_4(buffer, payload->request_id);
    serialize_object_id(buffer, payload->object_id);
    serialize_data_mode(buffer, &payload->data_writer);
}

void deserialize_write_data_payload(SerializedBufferHandle* buffer, MemoryCache* cache, WriteDataPayloadSpec* payload)
{
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_object_id(buffer, &payload->object_id);
    deserialize_data_mode(buffer, cache, &payload->data_writer);
}

int size_of_write_data_payload(const WriteDataPayloadSpec* payload)
{
    return sizeof(&payload->request_id)
         + size_of_object_id(payload->object_id)
         + size_of_data_mode(&payload->data_writer);
}

void serialize_read_data_payload(SerializedBufferHandle* buffer, const ReadDataPayloadSpec* payload)
{
    serialize_byte_4(buffer, payload->request_id);
    serialize_object_id(buffer, payload->object_id);

    serialize_byte_2(buffer, payload->max_messages);
    serialize_byte(buffer, payload->read_mode);
    serialize_byte_4(buffer, payload->max_elapsed_time);
    serialize_byte_4(buffer, payload->max_rate);

    serialize_byte_4(buffer, payload->expression_size);
    serialize_array(buffer, (uint8_t*)payload->content_filter_expression, payload->expression_size);

    serialize_byte_2(buffer, payload->max_samples);
    serialize_byte(buffer, payload->include_sample_info);
}

void deserialize_read_data_payload(SerializedBufferHandle* buffer, MemoryCache* cache, ReadDataPayloadSpec* payload)
{
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_object_id(buffer, &payload->object_id);

    deserialize_byte_2(buffer, &payload->max_messages);
    deserialize_byte(buffer, &payload->read_mode);
    deserialize_byte_4(buffer, &payload->max_elapsed_time);
    deserialize_byte_4(buffer, &payload->max_rate);

    deserialize_byte_4(buffer, &payload->expression_size);
    payload->content_filter_expression = request_memory_cache(cache, payload->expression_size);
    deserialize_array(buffer, (uint8_t*)payload->content_filter_expression, payload->expression_size);

    deserialize_byte_2(buffer, &payload->max_samples);
    deserialize_byte(buffer, &payload->include_sample_info);
}

int size_of_read_data_payload(const ReadDataPayloadSpec* payload)
{
    return sizeof(payload->request_id)
         + size_of_object_id(payload->object_id)
         + sizeof(payload->max_messages)
         + sizeof(payload->read_mode)
         + sizeof(payload->max_elapsed_time)
         + sizeof(payload->max_rate)
         + sizeof(payload->expression_size)
         + sizeof(uint8_t) * payload->expression_size
         + sizeof(payload->max_samples)
         + sizeof(payload->include_sample_info);
}

void serialize_data_payload(SerializedBufferHandle* buffer, const DataPayloadSpec* payload)
{
    serialize_byte_4(buffer, payload->request_id);
    serialize_object_id(buffer, payload->object_id);
    serialize_data_mode(buffer, &payload->data_reader);
}

void deserialize_data_payload(SerializedBufferHandle* buffer, MemoryCache* cache, DataPayloadSpec* payload)
{
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_object_id(buffer, &payload->object_id);
    deserialize_data_mode(buffer, cache, &payload->data_reader);
}

int size_of_data_payload(const DataPayloadSpec* payload)
{
    return sizeof(payload->request_id)
         + size_of_object_id(payload->object_id)
         + size_of_data_mode(&payload->data_reader);
}

// --------------------------------------------------------------------------
//                                  SWITCHES
// --------------------------------------------------------------------------

void serialize_object_kind(SerializedBufferHandle* buffer, const ObjectKindSpec* object)
{
    serialize_byte(buffer, object->kind);
    serialize_byte_4(buffer, object->string_size);
    serialize_array(buffer, (uint8_t*)object->string, object->string_size);

    switch(object->kind)
    {
        case OBJECT_KIND_DATA_WRITER:
            serialize_object_variant_data_writer(buffer, &object->variant.data_writer);
        break;

        case OBJECT_KIND_DATA_READER:
            serialize_object_variant_data_reader(buffer, &object->variant.data_reader);
        break;

        case OBJECT_KIND_PUBLISHER:
            serialize_object_variant_publisher(buffer, &object->variant.publisher);
        break;

        case OBJECT_KIND_SUBSCRIBER:
            serialize_object_variant_subscriber(buffer, &object->variant.subscriber);
        break;
    }
}

void deserialize_object_kind(SerializedBufferHandle* buffer, MemoryCache* cache, ObjectKindSpec* object)
{
    deserialize_byte(buffer, &object->kind);

    deserialize_byte_4(buffer, &object->string_size);
    object->string = request_memory_cache(cache, object->string_size);
    deserialize_array(buffer, (uint8_t*)object->string, object->string_size);

    switch(object->kind)
    {
        case OBJECT_KIND_DATA_WRITER:
            deserialize_object_variant_data_writer(buffer, cache, &object->variant.data_writer);
        break;

        case OBJECT_KIND_DATA_READER:
            deserialize_object_variant_data_reader(buffer, cache, &object->variant.data_reader);
        break;

        case OBJECT_KIND_PUBLISHER:
            deserialize_object_variant_publisher(buffer, cache, &object->variant.publisher);
        break;

        case OBJECT_KIND_SUBSCRIBER:
            deserialize_object_variant_subscriber(buffer, cache, &object->variant.subscriber);
        break;
    }
}

int size_of_object_kind(const ObjectKindSpec* object)
{
    uint32_t size = sizeof(object->kind)
                + sizeof(object->string_size)
                + sizeof(uint8_t) * object->string_size;

    switch(object->kind)
    {
        case OBJECT_KIND_DATA_WRITER:
            size += size_of_object_variant_data_writer(&object->variant.data_writer);
        break;

        case OBJECT_KIND_DATA_READER:
            size += size_of_object_variant_data_reader(&object->variant.data_reader);
        break;

        case OBJECT_KIND_PUBLISHER:
            size += size_of_object_variant_publisher(&object->variant.publisher);
        break;

        case OBJECT_KIND_SUBSCRIBER:
            size += size_of_object_variant_subscriber(&object->variant.subscriber);
        break;
    }

    return size;
}

void serialize_data_mode(SerializedBufferHandle* buffer, const DataModeSpec* data_mode)
{
    serialize_byte(buffer, data_mode->read_mode);
    switch(data_mode->read_mode)
    {
        case READ_MODE_DATA:
            serialize_sample_data(buffer, &data_mode->sample_kind.data);
        break;

        case READ_MODE_SAMPLE:
            serialize_sample(buffer, &data_mode->sample_kind.sample);
        break;
    }
}

void deserialize_data_mode(SerializedBufferHandle* buffer, MemoryCache* cache, DataModeSpec* data_mode)
{
    deserialize_byte(buffer, &data_mode->read_mode);

    switch(data_mode->read_mode)
    {
        case READ_MODE_DATA:
            deserialize_sample_data(buffer,cache,  &data_mode->sample_kind.data);
        break;

        case READ_MODE_SAMPLE:
            deserialize_sample(buffer, cache, &data_mode->sample_kind.sample);
        break;
    }
}

int size_of_data_mode(const DataModeSpec* data_mode)
{
    uint32_t size = sizeof(data_mode->read_mode);

    switch(data_mode->read_mode)
    {
        case READ_MODE_DATA:
            size += size_of_sample_data(&data_mode->sample_kind.data);
        break;

        case READ_MODE_SAMPLE:
            size += size_of_sample(&data_mode->sample_kind.sample);
        break;
    }

    return size;
}

// --------------------------------------------------------------------------
//                               OBJECT VARIANTS
// --------------------------------------------------------------------------

void serialize_object_variant_data_writer(SerializedBufferHandle* buffer, const DataWriterSpec* data_writer)
{
    serialize_object_id(buffer, data_writer->participant_id);
    serialize_object_id(buffer, data_writer->publisher_id);
}

void deserialize_object_variant_data_writer(SerializedBufferHandle* buffer, MemoryCache* cache, DataWriterSpec* data_writer)
{
    deserialize_object_id(buffer, &data_writer->participant_id);
    deserialize_object_id(buffer, &data_writer->publisher_id);
}

int size_of_object_variant_data_writer(const DataWriterSpec* data_writer)
{
    return size_of_object_id(data_writer->participant_id)
         + size_of_object_id(data_writer->publisher_id);
}

void serialize_object_variant_data_reader(SerializedBufferHandle* buffer, const DataReaderSpec* data_reader)
{
    serialize_object_id(buffer, data_reader->participant_id);
    serialize_object_id(buffer, data_reader->subscriber_id);
}

void deserialize_object_variant_data_reader(SerializedBufferHandle* buffer, MemoryCache* cache, DataReaderSpec* data_reader)
{
    deserialize_object_id(buffer, &data_reader->participant_id);
    deserialize_object_id(buffer, &data_reader->subscriber_id);
}

int size_of_object_variant_data_reader(const DataReaderSpec* data_reader)
{
    return size_of_object_id(data_reader->participant_id)
         + size_of_object_id(data_reader->subscriber_id);
}

void serialize_object_variant_publisher(SerializedBufferHandle* buffer, const PublisherSpec* publisher)
{
    serialize_object_id(buffer, publisher->participant_id);
}

void deserialize_object_variant_publisher(SerializedBufferHandle* buffer, MemoryCache* cache, PublisherSpec* publisher)
{
    deserialize_object_id(buffer, &publisher->participant_id);
}

int size_of_object_variant_publisher(const PublisherSpec* publisher)
{
    return size_of_object_id(publisher->participant_id);
}

void serialize_object_variant_subscriber(SerializedBufferHandle* buffer, const SubscriberSpec* subscriber)
{
    serialize_object_id(buffer, subscriber->participant_id);
}

void deserialize_object_variant_subscriber(SerializedBufferHandle* buffer, MemoryCache* cache, SubscriberSpec* subscriber)
{
    deserialize_object_id(buffer, &subscriber->participant_id);
}

int size_of_object_variant_subscriber(const SubscriberSpec* subscriber)
{
    return size_of_object_id(subscriber->participant_id);
}


// --------------------------------------------------------------------------
//                                  STATUS
// --------------------------------------------------------------------------

void serialize_result_status(SerializedBufferHandle* buffer, const ResultStatusSpec* result)
{
    serialize_byte_4(buffer, result->request_id);
    serialize_byte(buffer, result->status);
    serialize_byte(buffer, result->implementation_status);
}

void deserialize_result_status(SerializedBufferHandle* buffer, MemoryCache* cache, ResultStatusSpec* result)
{
    deserialize_byte_4(buffer, &result->request_id);
    deserialize_byte(buffer, &result->status);
    deserialize_byte(buffer, &result->implementation_status);
}

int size_of_result_status(const ResultStatusSpec* result)
{
    return sizeof(result->request_id)
         + sizeof(result->status)
         + sizeof(result->implementation_status);
}

// --------------------------------------------------------------------------
//                                  SAMPLES
// --------------------------------------------------------------------------

void serialize_sample(SerializedBufferHandle* buffer, const SampleSpec* sample)
{
    serialize_sample_info(buffer, &sample->info);
    serialize_sample_data(buffer, &sample->data);
}

void deserialize_sample(SerializedBufferHandle* buffer, MemoryCache* cache, SampleSpec* sample)
{
    deserialize_sample_info(buffer, cache, &sample->info);
    deserialize_sample_data(buffer, cache, &sample->data);
}

int size_of_sample(const SampleSpec* sample)
{
    return size_of_sample_info(&sample->info)
        +  size_of_sample_data(&sample->data);
}

void serialize_sample_data(SerializedBufferHandle* buffer, const SampleDataSpec* data)
{
    serialize_byte_4(buffer, data->serialized_data_size);
    serialize_array(buffer, data->serialized_data, data->serialized_data_size);
}

void deserialize_sample_data(SerializedBufferHandle* buffer, MemoryCache* cache, SampleDataSpec* data)
{
    deserialize_byte_4(buffer, &data->serialized_data_size);
    data->serialized_data = request_memory_cache(cache, data->serialized_data_size);
    deserialize_array(buffer, data->serialized_data, data->serialized_data_size);
}

int size_of_sample_data(const SampleDataSpec* data)
{
    return sizeof(data->serialized_data_size)
         + sizeof(uint8_t) * data->serialized_data_size;
}

void serialize_sample_info(SerializedBufferHandle* buffer, const SampleInfoSpec* info)
{
    serialize_byte(buffer, info->state);
    serialize_byte_4(buffer, info->sequence_number);
    serialize_byte_4(buffer, info->session_time_offset);
}

void deserialize_sample_info(SerializedBufferHandle* buffer, MemoryCache* cache, SampleInfoSpec* info)
{
    deserialize_byte(buffer, &info->state);
    deserialize_byte_4(buffer, &info->sequence_number);
    deserialize_byte_4(buffer, &info->session_time_offset);
}

int size_of_sample_info(const SampleInfoSpec* info)
{
    return sizeof(info->state)
          + sizeof(info->sequence_number)
          + sizeof(info->session_time_offset);
}

// --------------------------------------------------------------------------
//                                  UTIL
// --------------------------------------------------------------------------
void serialize_object_id(SerializedBufferHandle* buffer, uint_least24_t object_id)
{
    if(buffer->endian_machine == BIG_ENDIAN_MODE)
        object_id = object_id << 8;
    serialize_array(buffer, (uint8_t*)&object_id, 3);
}

void deserialize_object_id(SerializedBufferHandle* buffer, uint_least24_t* object_id)
{
    *object_id = 0;
    if(buffer->endian_machine == BIG_ENDIAN_MODE)
        object_id++;
    deserialize_array(buffer, (uint8_t*)object_id, 3);
}

int size_of_object_id(uint_least24_t object_id)
{
    return 3;
}
