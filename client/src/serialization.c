#include "serialization.h"
#include <stdlib.h>

// Message Header
// 0                8                16              24               31
// +---------------+----------------+----------------+----------------+
// |                          client_key                              |
// +---------------+--------+-------+----------------+----------------+
// |   session_id  |    stream_id   |           sequence_nr           |
// +---------------+--------+-------+----------------+----------------+
void serialize_message_header(SerializedBufferHandle* buffer, const MessageHeaderSpec* message_header)
{
    serialize_byte_4(buffer, message_header->client_key);
    serialize_byte(buffer, message_header->session_id);
    serialize_byte(buffer, message_header->stream_id);
    serialize_byte_2(buffer, message_header->sequence_nr);
}

void deserialize_message_header(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory,  MessageHeaderSpec* message_header)
{
    deserialize_byte_4(buffer, &message_header->client_key);
    deserialize_byte(buffer, &message_header->session_id);
    deserialize_byte(buffer, &message_header->stream_id);
    deserialize_byte_2(buffer, &message_header->sequence_nr);
}

int size_of_message_header(const MessageHeaderSpec* message_header)
{
    return sizeof(message_header->client_key)
         + sizeof(message_header->session_id)
         + sizeof(message_header->stream_id)
         + sizeof(message_header->sequence_nr);
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
    serialize_byte_2(buffer, submessage_header->length);
}

void deserialize_submessage_header(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SubmessageHeaderSpec* submessage_header)
{
    deserialize_byte(buffer, &submessage_header->id);
    deserialize_byte(buffer, &submessage_header->flags);
    deserialize_byte_2(buffer, &submessage_header->length);
}

int size_of_submessage_header(const SubmessageHeaderSpec* submessage_header)
{
    return sizeof(submessage_header->id)
         + sizeof(submessage_header->flags)
         + sizeof(submessage_header->length);
}

void serialize_create_payload(SerializedBufferHandle* buffer, const CreatePayloadSpec* payload)
{
    serialize_byte_4(buffer, payload->request_id);
    serialize_block(buffer, (uint8_t*)&payload->object_id, 3);

    serialize_object_kind(buffer, &payload->object);
}

void deserialize_create_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, CreatePayloadSpec* payload)
{
    payload->object_id = 0;
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_block(buffer, (uint8_t*)&payload->object_id, 3);
    deserialize_object_kind(buffer, dynamic_memory, &payload->object);
}

int size_of_create_payload(const CreatePayloadSpec* payload)
{
    return sizeof(payload->request_id)
         + 3
         + size_of_object_kind(&payload->object);
}

void serialize_delete_payload(SerializedBufferHandle* buffer, const DeletePayloadSpec* payload)
{
    serialize_byte_4(buffer, payload->request_id);
    serialize_block(buffer, (uint8_t*)&payload->object_id, 3);
}

void deserialize_delete_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DeletePayloadSpec* payload)
{
    payload->object_id = 0;
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_block(buffer, (uint8_t*)&payload->object_id, 3);
}

int size_of_delete_payload(const DeletePayloadSpec* payload)
{
    return sizeof(payload->request_id)
         + 3;
}

void serialize_write_data_payload(SerializedBufferHandle* buffer, const WriteDataPayloadSpec* payload)
{
    serialize_byte_4(buffer, payload->request_id);
    serialize_block(buffer, (uint8_t*)&payload->object_id, 3);

    serialize_data_mode(buffer, &payload->data_writer);
}

void deserialize_write_data_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, WriteDataPayloadSpec* payload)
{
    payload->object_id = 0;
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_block(buffer, (uint8_t*)&payload->object_id, 3);

    deserialize_data_mode(buffer, dynamic_memory, &payload->data_writer);
}

int size_of_write_data_payload(const WriteDataPayloadSpec* payload)
{
    return sizeof(&payload->request_id)
         + 3
         + size_of_data_mode(&payload->data_writer);
}

void serialize_read_data_payload(SerializedBufferHandle* buffer, const ReadDataPayloadSpec* payload)
{
    serialize_byte_4(buffer, payload->request_id);
    serialize_byte_2(buffer, payload->max_messages);
    serialize_byte_4(buffer, payload->max_elapsed_time);
    serialize_byte_4(buffer, payload->max_rate);

    serialize_byte_4(buffer, payload->expression_size);
    serialize_array(buffer, (uint8_t*)payload->content_filter_expression, payload->expression_size);

    serialize_byte_2(buffer, payload->max_samples);
    serialize_byte(buffer, payload->include_sample_info);
}

void deserialize_read_data_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, ReadDataPayloadSpec* payload)
{
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_byte_2(buffer, &payload->max_messages);
    deserialize_byte_4(buffer, &payload->max_elapsed_time);
    deserialize_byte_4(buffer, &payload->max_rate);

    deserialize_byte_4(buffer, &payload->expression_size);
    payload->content_filter_expression = request_memory_buffer(dynamic_memory, payload->expression_size);
    deserialize_array(buffer, (uint8_t*)payload->content_filter_expression, payload->expression_size);

    deserialize_byte_2(buffer, &payload->max_samples);
    deserialize_byte(buffer, &payload->include_sample_info);
}

int size_of_read_data_payload(const ReadDataPayloadSpec* payload)
{
    return sizeof(payload->request_id)
         + sizeof(payload->max_messages)
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
    serialize_block(buffer, (uint8_t*)&payload->object_id, 3);

    serialize_data_mode(buffer, &payload->data_reader);
}

void deserialize_data_payload(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataPayloadSpec* payload)
{
    payload->object_id = 0;
    deserialize_byte_4(buffer, &payload->request_id);
    deserialize_block(buffer, (uint8_t*)&payload->object_id, 3);

    deserialize_data_mode(buffer, dynamic_memory, &payload->data_reader);
}

int size_of_data_payload(const DataPayloadSpec* payload)
{
    return sizeof(payload->request_id)
         + 3
         + size_of_data_mode(&payload->data_reader);
}

void serialize_object_kind(SerializedBufferHandle* buffer, const ObjectKindSpec* object)
{
    serialize_byte(buffer, object->kind);
    serialize_byte_4(buffer, object->string_size);
    serialize_array(buffer, (uint8_t*)object->string, object->string_size);

    switch(object->kind)
    {
        case OBJECT_KIND_DATAWRITER:
            serialize_object_variant_data_writer(buffer, &object->variant.data_writer);
        break;

        case OBJECT_KIND_DATAREADER:
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

void deserialize_object_kind(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, ObjectKindSpec* object)
{
    deserialize_byte(buffer, &object->kind);

    deserialize_byte_4(buffer, &object->string_size);
    object->string = request_memory_buffer(dynamic_memory, object->string_size);
    deserialize_array(buffer, (uint8_t*)object->string, object->string_size);

    switch(object->kind)
    {
        case OBJECT_KIND_DATAWRITER:
            deserialize_object_variant_data_writer(buffer, dynamic_memory, &object->variant.data_writer);
        break;

        case OBJECT_KIND_DATAREADER:
            deserialize_object_variant_data_reader(buffer, dynamic_memory, &object->variant.data_reader);
        break;

        case OBJECT_KIND_PUBLISHER:
            deserialize_object_variant_publisher(buffer, dynamic_memory, &object->variant.publisher);
        break;

        case OBJECT_KIND_SUBSCRIBER:
            deserialize_object_variant_subscriber(buffer, dynamic_memory, &object->variant.subscriber);
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
        case OBJECT_KIND_DATAWRITER:
            size += size_of_object_variant_data_writer(&object->variant.data_writer);
        break;

        case OBJECT_KIND_DATAREADER:
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

void deserialize_data_mode(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataModeSpec* data_mode)
{
    deserialize_byte(buffer, &data_mode->read_mode);

    switch(data_mode->read_mode)
    {
        case READ_MODE_DATA:
            deserialize_sample_data(buffer,dynamic_memory,  &data_mode->sample_kind.data);
        break;

        case READ_MODE_SAMPLE:
            deserialize_sample(buffer, dynamic_memory, &data_mode->sample_kind.sample);
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

void serialize_object_variant_data_writer(SerializedBufferHandle* buffer, const DataWriterSpec* data_writer)
{
    serialize_block(buffer, (uint8_t*)&data_writer->participant_id, 3);
    serialize_block(buffer, (uint8_t*)&data_writer->publisher_id, 3);
}

void deserialize_object_variant_data_writer(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataWriterSpec* data_writer)
{
    data_writer->participant_id = 0;
    data_writer->publisher_id = 0;
    deserialize_block(buffer, (uint8_t*)&data_writer->participant_id, 3);
    deserialize_block(buffer, (uint8_t*)&data_writer->publisher_id, 3);
}

int size_of_object_variant_data_writer(const DataWriterSpec* data_writer)
{
    return 6;
}

void serialize_object_variant_data_reader(SerializedBufferHandle* buffer, const DataReaderSpec* data_reader)
{
    serialize_block(buffer, (uint8_t*)&data_reader->participant_id, 3);
    serialize_block(buffer, (uint8_t*)&data_reader->subscriber_id, 3);
}

void deserialize_object_variant_data_reader(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, DataReaderSpec* data_reader)
{
    data_reader->participant_id = 0;
    data_reader->subscriber_id = 0;
    deserialize_block(buffer, (uint8_t*)&data_reader->participant_id, 3);
    deserialize_block(buffer, (uint8_t*)&data_reader->subscriber_id, 3);
}

int size_of_object_variant_data_reader(const DataReaderSpec* data_reader)
{
    return 6;
}

void serialize_object_variant_publisher(SerializedBufferHandle* buffer, const PublisherSpec* publisher)
{
    serialize_block(buffer, (uint8_t*)&publisher->participant_id, 3);
}

void deserialize_object_variant_publisher(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, PublisherSpec* publisher)
{
    publisher->participant_id = 0;
    deserialize_block(buffer, (uint8_t*)&publisher->participant_id, 3);
}

int size_of_object_variant_publisher(const PublisherSpec* publisher)
{
    return 3;
}

void serialize_object_variant_subscriber(SerializedBufferHandle* buffer, const SubscriberSpec* subscriber)
{
    serialize_block(buffer, (uint8_t*)&subscriber->participant_id, 3);
}

void deserialize_object_variant_subscriber(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SubscriberSpec* subscriber)
{
    subscriber->participant_id = 0;
    deserialize_block(buffer, (uint8_t*)&subscriber->participant_id, 3);
}

int size_of_object_variant_subscriber(const SubscriberSpec* subscriber)
{
    return 3;
}

void serialize_sample(SerializedBufferHandle* buffer, const SampleSpec* sample)
{
    serialize_sample_info(buffer, &sample->info);
    serialize_sample_data(buffer, &sample->data);
}

void deserialize_sample(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SampleSpec* sample)
{
    deserialize_sample_info(buffer, dynamic_memory, &sample->info);
    deserialize_sample_data(buffer, dynamic_memory, &sample->data);
}

int size_of_sample(const SampleSpec* sample)
{
    //TODO
    return size_of_sample_info(&sample->info)
        +  size_of_sample_data(&sample->data);
}

void serialize_sample_data(SerializedBufferHandle* buffer, const SampleDataSpec* data)
{
    serialize_byte_4(buffer, data->serialized_data_size);
    serialize_array(buffer, data->serialized_data, data->serialized_data_size);
}

void deserialize_sample_data(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SampleDataSpec* data)
{
    deserialize_byte_4(buffer, &data->serialized_data_size);
    data->serialized_data = request_memory_buffer(dynamic_memory, data->serialized_data_size);
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
    serialize_byte_8(buffer, info->sequence_number);
    serialize_byte_4(buffer, info->session_time_offset);
}

void deserialize_sample_info(SerializedBufferHandle* buffer, DynamicBuffer* dynamic_memory, SampleInfoSpec* info)
{
    deserialize_byte(buffer, &info->state);
    deserialize_byte_8(buffer, &info->sequence_number);
    deserialize_byte_4(buffer, &info->session_time_offset);
}

int size_of_sample_info(const SampleInfoSpec* info)
{
    return sizeof(info->state)
          + sizeof(info->sequence_number)
          + sizeof(info->session_time_offset);
}

int align_and_check_size_of_submessage_header(SerializedBufferHandle* buffer)
{
    align_to(buffer, 4);
    return sizeof(SubmessageHeaderSpec);
}

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