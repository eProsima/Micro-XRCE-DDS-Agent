#include "message.h"
#include "serialization.h"

void init_message(SerializedBufferHandle* writer, const MessageHeaderSpec* message_header)
{
    serialize_message_header(writer, message_header);
}

int add_create_submessage(SerializedBufferHandle* writer, const CreatePayloadSpec* payload)
{
    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_CREATE;
    header.flags = 0x07;
    header.length = size_of_create_payload(payload);

    if(align_and_check_size_of_submessage_header(writer) + header.length > get_free_space(writer))
        return 0;

    serialize_submessage_header(writer, &header);
    serialize_create_payload(writer, payload);

    return 1;
}

int add_delete_submessage(SerializedBufferHandle* writer, const DeletePayloadSpec* payload)
{
    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_DELETE;
    header.flags = 0x07;
    header.length = size_of_delete_payload(payload);

    if(align_and_check_size_of_submessage_header(writer) + header.length > get_free_space(writer))
        return 0;

    serialize_submessage_header(writer, &header);
    serialize_delete_payload(writer, payload);

    return 1;
}

int add_write_data_submessage(SerializedBufferHandle* writer, const WriteDataPayloadSpec* payload)
{
    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_WRITE_DATA;
    header.flags = 0x07;
    header.length = size_of_write_data_payload(payload);

    if(align_and_check_size_of_submessage_header(writer) + header.length > get_free_space(writer))
        return 0;

    serialize_submessage_header(writer, &header);
    serialize_write_data_payload(writer, payload);

    return 1;
}

int add_read_data_submessage(SerializedBufferHandle* writer, const ReadDataPayloadSpec* payload)
{
    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_READ_DATA;
    header.flags = 0x07;
    header.length = size_of_read_data_payload(payload);

    if(align_and_check_size_of_submessage_header(writer) + header.length > get_free_space(writer))
        return 0;

    serialize_submessage_header(writer, &header);
    serialize_read_data_payload(writer, payload);

    return 1;
}

int add_data_submessage(SerializedBufferHandle* writer, const DataPayloadSpec* payload)
{
    SubmessageHeaderSpec header;
    header.id = SUBMESSAGE_DATA;
    header.flags = 0x07;
    header.length = size_of_data_payload(payload);

    if(align_and_check_size_of_submessage_header(writer)+ header.length > get_free_space(writer))
        return 0;

    serialize_submessage_header(writer, &header);
    serialize_data_payload(writer, payload);

    return 1;
}

int parse_message(SerializedBufferHandle* reader, const MessageCallback* callback)
{
    if(get_free_space(reader) < sizeof(MessageHeaderSpec))
            return 0;

    DynamicBuffer dynamic_memory;
    init_memory_buffer(&dynamic_memory, 0);

    MessageHeaderSpec message_header;
    deserialize_message_header(reader, &dynamic_memory, &message_header);
    //do things with the header.
    if(callback->message_header)
        callback->message_header(&message_header);

    while(align_and_check_size_of_submessage_header(reader) < get_free_space(reader))
    {
        SubmessageHeaderSpec submessage_header;
        deserialize_submessage_header(reader, &dynamic_memory, &submessage_header);

        if(submessage_header.length > get_free_space(reader))
        {
            free_memory_buffer(&dynamic_memory);
            return 0;
        }

        //buffer->endian_mode =

        if(callback->submessage_header)
            callback->submessage_header(&submessage_header);

        PayloadSpec payload;
        #ifdef DEBUG
        memset(&payload, 0, sizeof(PayloadSpec));
        #endif
        switch(submessage_header.id)
        {
            case SUBMESSAGE_CREATE:
                deserialize_create_payload(reader, &dynamic_memory, &payload.create_resource);
                if(callback->create_resource)
                    callback->create_resource(&payload.create_resource);
            break;

            case SUBMESSAGE_DELETE:
                deserialize_delete_payload(reader, &dynamic_memory, &payload.delete_resource);
                if(callback->delete_resource)
                    callback->delete_resource(&payload.delete_resource);
            break;

            case SUBMESSAGE_WRITE_DATA:
                deserialize_write_data_payload(reader, &dynamic_memory, &payload.write_data);
                if(callback->write_data)
                    callback->write_data(&payload.write_data);
            break;

            case SUBMESSAGE_READ_DATA:
                deserialize_read_data_payload(reader, &dynamic_memory, &payload.read_data);
                if(callback->read_data)
                    callback->read_data(&payload.read_data);
            break;

            case SUBMESSAGE_DATA:
                deserialize_data_payload(reader, &dynamic_memory, &payload.data);
                if(callback->data)
                    callback->data(&payload.data);
            break;
        }

        reset_memory_buffer(&dynamic_memory);
    }

    free_memory_buffer(&dynamic_memory);

    if(sizeof(SubmessageHeaderSpec) > get_free_space(reader))
        return 0;
    return 1;
}
