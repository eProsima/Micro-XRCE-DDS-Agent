#include "micrortps/client/message.h"
#include "micrortps/client/serialization.h"

#ifdef _DEBUG_
#include <string.h>
#endif

// PRIVATE DEFINITIONS
int check_and_set_headers(MessageManager* message_manager, const SubmessageHeaderSpec* header);


void init_message_manager(MessageManager* message_manager, uint8_t* out_buffer, uint32_t out_buffer_size,
        uint8_t* in_buffer, uint32_t in_buffer_size, MessageCallback callback)
{
    message_manager->out_buffer = out_buffer;
    message_manager->in_buffer = in_buffer;

    init_serialized_buffer(&message_manager->writer, message_manager->out_buffer, out_buffer_size);
    init_serialized_buffer(&message_manager->reader, message_manager->in_buffer, in_buffer_size);

    message_manager->callback = callback;

    // Initialize with the headers cache requirements. Currently 0.
    init_memory_cache(&message_manager->cache, 0);
}

void destroy_message_manager(MessageManager* message_manager)
{
    free_memory_cache(&message_manager->cache);
}

void remove_cache(MessageManager* message_manager)
{
    free_memory_cache(&message_manager->cache);

    // Initialize with the headers cache requirements. Currently 0.
    init_memory_cache(&message_manager->cache, 0);
}

int check_and_set_headers(MessageManager* message_manager, const SubmessageHeaderSpec* header)
{
    SerializedBufferHandle* writer = &message_manager->writer;

    if(writer->iterator == writer->data)
    {
        MessageHeaderSpec message_header;
        if(!message_manager->callback.on_initialize_message(&message_header, message_manager->callback.data))
            return 0;

        serialize_message_header(writer, &message_header);
    }
    else //because the message header is already alignment.
    {
        align_to(writer, 4);
    }

    if(writer->iterator + sizeof(SubmessageHeaderSpec) + header->length > writer->final)
        return 0;

    serialize_submessage_header(writer, header);
    return 1;
}

uint32_t add_create_submessage(MessageManager* message_manager, const CreatePayloadSpec* payload)
{
    SerializedBufferHandle* writer = &message_manager->writer;

    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_CREATE;
    header.flags = 0x07;
    header.length = size_of_create_payload(payload);

    if(check_and_set_headers(message_manager, &header))
    {
        serialize_create_payload(writer, payload);
        return writer->iterator - writer->data;
    }

    return 0;
}

uint32_t add_delete_submessage(MessageManager* message_manager, const DeletePayloadSpec* payload)
{
    SerializedBufferHandle* writer = &message_manager->writer;

    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_DELETE;
    header.flags = 0x07;
    header.length = size_of_delete_payload(payload);

    if(check_and_set_headers(message_manager, &header))
    {
        serialize_delete_payload(writer, payload);
        return writer->iterator - writer->data;
    }

    return 0;
}

uint32_t add_status_submessage(MessageManager* message_manager, const StatusPayloadSpec* payload)
{
    SerializedBufferHandle* writer = &message_manager->writer;

    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_STATUS;
    header.flags = 0x07;
    header.length = size_of_status_payload(payload);

    if(check_and_set_headers(message_manager, &header))
    {
        serialize_status_payload(writer, payload);
        return writer->iterator - writer->data;
    }

    return 0;
}

uint32_t add_write_data_submessage(MessageManager* message_manager, const WriteDataPayloadSpec* payload)
{
    SerializedBufferHandle* writer = &message_manager->writer;

    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_WRITE_DATA;
    header.flags = 0x07;
    header.length = size_of_write_data_payload(payload);

    if(check_and_set_headers(message_manager, &header))
    {
        serialize_write_data_payload(writer, payload);
        return writer->iterator - writer->data;
    }

    return 0;
}

uint32_t add_read_data_submessage(MessageManager* message_manager, const ReadDataPayloadSpec* payload)
{
    SerializedBufferHandle* writer = &message_manager->writer;

    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_READ_DATA;
    header.flags = 0x07;
    header.length = size_of_read_data_payload(payload);

    if(check_and_set_headers(message_manager, &header))
    {
        serialize_read_data_payload(writer, payload);
        return writer->iterator - writer->data;
    }

    return 0;
}

uint32_t add_data_submessage(MessageManager* message_manager, const DataPayloadSpec* payload)
{
    SerializedBufferHandle* writer = &message_manager->writer;

    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_DATA;
    header.flags = 0x07;
    header.length = size_of_data_payload(payload);

    if(check_and_set_headers(message_manager, &header))
    {
        serialize_data_payload(writer, payload);
        return writer->iterator - writer->data;
    }

    return 0;
}

int parse_message(MessageManager* message_manager, uint32_t message_length)
{
    MessageCallback* callback = &message_manager->callback;
    MemoryCache* cache = &message_manager->cache;
    SerializedBufferHandle* reader = &message_manager->reader;
    reset_buffer_iterator(reader);

    if(reader->data + message_length > reader->final || message_length < sizeof(MessageHeaderSpec))
        return 0;

    MessageHeaderSpec message_header;
    deserialize_message_header(reader, cache, &message_header);
    if(callback->on_message_header)
        if(!callback->on_message_header(&message_header, callback->data))
            return 0;

    do
    {
        SubmessageHeaderSpec submessage_header;
        deserialize_submessage_header(reader, cache, &submessage_header);
        if(callback->on_submessage_header)
            callback->on_submessage_header(&submessage_header, callback->data);

        if(reader->iterator + submessage_header.length > reader->data + message_length)
            return 0;

        //buffer->endian_mode = //set with flags

        reset_memory_cache(cache, message_length);

        PayloadSpec payload;
        #ifdef _DEBUG_
        memset(&payload, 0xFF, sizeof(PayloadSpec));
        #endif
        switch(submessage_header.id)
        {
            case SUBMESSAGE_CREATE:
                deserialize_create_payload(reader, cache, &payload.create_resource);
                if(callback->on_create_resource)
                    callback->on_create_resource(&payload.create_resource, callback->data);
            break;

            case SUBMESSAGE_DELETE:
                deserialize_delete_payload(reader, cache, &payload.delete_resource);
                if(callback->on_delete_resource)
                    callback->on_delete_resource(&payload.delete_resource, callback->data);
            break;

            case SUBMESSAGE_STATUS:
                deserialize_status_payload(reader, cache, &payload.status);
                if(callback->on_status)
                    callback->on_status(&payload.status, callback->data);
            break;

            case SUBMESSAGE_WRITE_DATA:
                deserialize_write_data_payload(reader, cache, &payload.write_data);
                if(callback->on_write_data)
                    callback->on_write_data(&payload.write_data, callback->data);
            break;

            case SUBMESSAGE_READ_DATA:
                deserialize_read_data_payload(reader, cache, &payload.read_data);
                if(callback->on_read_data)
                    callback->on_read_data(&payload.read_data, callback->data);
            break;

            case SUBMESSAGE_DATA:
                deserialize_data_payload(reader, cache, &payload.data);
                if(callback->on_data)
                    callback->on_data(&payload.data, callback->data);
            break;

            default:
                return 0;
        }

        align_to(reader, 4);
    }
    while(reader->iterator + sizeof(SubmessageHeaderSpec) <= reader->data + message_length);

    return 1;
}