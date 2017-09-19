#include "micrortps/client/message.h"
#include "micrortps/client/debug/message_debugger.h"

#include <stdio.h>

#define BUFFER_SIZE 2000

void create_message(MessageManager* message);


int main(int args, char** argv)
{
    uint8_t buffer[BUFFER_SIZE] = {};

    MessageCallback callback = {};
    callback.on_message_header = print_message_header;
    callback.on_submessage_header = print_submessage_header;
    callback.on_create_resource = print_create_submessage;
    callback.on_delete_resource = print_delete_submessage;
    callback.on_status = print_status_submessage;
    callback.on_write_data = print_write_data_submessage;
    callback.on_read_data = print_read_data_submessage;
    callback.on_data = print_data_submessage;
    callback.data = NULL;

    MessageManager message_manager;
    init_message_manager(&message_manager, buffer, BUFFER_SIZE, buffer, BUFFER_SIZE, callback);



    printf("%s\n\n", "SERIALIZATION:");
    create_message(&message_manager);

    printf(" ");
    for(uint8_t* i = message_manager.writer.data; i < message_manager.writer.iterator; i++)
    {
        printf("%02X ", *i);
        if((i - message_manager.writer.data + 1) % 16 == 0)
            printf("\n ");
    }
    printf("\n\n");

    uint32_t seliarized_size = message_manager.writer.iterator - message_manager.writer.data;
    printf(" %u serialized bytes. \n", seliarized_size);



    printf("\n%s\n", "DESERIALIZATION: \n");
    parse_message(&message_manager, seliarized_size);

    uint32_t deseliarized_size = message_manager.reader.iterator - message_manager.reader.data;
    printf(" %u deserialized bytes. \n", deseliarized_size);

    destroy_message_manager(&message_manager);
    return 0;
}

void create_message(MessageManager* message)
{
    MessageHeaderSpec header;
    header.client_key = 0xF1F2F3F4;
    header.session_id = 0x01;
    header.stream_id = 0x02;
    header.sequence_number = 1234;
    start_message(message, &header);


    // [CREATE] SUBMESSAGE
    {
        char string[] = "Hello world";

        CreatePayloadSpec payload;
        payload.request_id = 0xAABBCCDD;
        payload.object_id = 0x778899;
        payload.object.kind = OBJECT_KIND_DATA_WRITER;

        payload.object.string = string;
        payload.object.string_size = strlen(string) + 1;

        switch(payload.object.kind)
        {
            case OBJECT_KIND_DATA_WRITER:
                payload.object.variant.data_writer.participant_id = 0xAAAAAA;
                payload.object.variant.data_writer.publisher_id = 0xBBBBBB;
            break;

            case OBJECT_KIND_DATA_READER:
                payload.object.variant.data_reader.participant_id = 0xCCCCCC;
                payload.object.variant.data_reader.subscriber_id = 0xDDDDDD;
            break;

            case OBJECT_KIND_SUBSCRIBER:
                payload.object.variant.subscriber.participant_id = 0xEEEEEE;
            break;

            case OBJECT_KIND_PUBLISHER:
                payload.object.variant.publisher.participant_id = 0xFFFFFF;
            break;
        }

        add_create_submessage(message, &payload);
    }


    // [DELETE] SUBMESSAGE
    {
        DeletePayloadSpec payload;
        payload.request_id = 0x12345678;
        payload.object_id = 0xABCDEF;

        add_delete_submessage(message, &payload);
    }

    // [STATUS] SUBMESSAGE
    {
        StatusPayloadSpec payload;
        payload.result.request_id = 0x12345678;
        payload.result.status = 0x01;
        payload.result.implementation_status = 0x02;

        payload.object_id = 0xABCDEF;

        add_status_submessage(message, &payload);
    }

    // [WRITE_DATA] SUBMESSAGE
    {
        uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};

        WriteDataPayloadSpec payload;
        payload.request_id = 0xAABBCCDD;
        payload.object_id = 0x778899;
        payload.data_writer.read_mode = READ_MODE_SAMPLE;

        SampleKindSpec* kind = &payload.data_writer.sample_kind;
        switch(payload.data_writer.read_mode)
        {
            case READ_MODE_DATA:
                kind->data.serialized_data = data;
                kind->data.serialized_data_size = 5;
            break;

            case READ_MODE_SAMPLE:
                kind->sample.info.state = 0x01;
                kind->sample.info.sequence_number = 0x01234567;
                kind->sample.info.session_time_offset = 0xAAAABBBB;
                kind->sample.data.serialized_data = data;
                kind->sample.data.serialized_data_size = 5;
            break;
        }

        add_write_data_submessage(message, &payload);
    }

    // [READ_DATA] SUBMESSAGE
    {
        char expresion[] = "EXPRESION";

        ReadDataPayloadSpec payload;
        payload.request_id = 0x11223344;
        payload.object_id = 0x778899;
        payload.max_messages = 12345;
        payload.read_mode = 0x02;
        payload.max_elapsed_time = 987654321;
        payload.max_rate = 123123123;
        payload.content_filter_expression = expresion;
        payload.expression_size = strlen(expresion) + 1;
        payload.max_samples = 55555;
        payload.include_sample_info = 1;

        add_read_data_submessage(message, &payload);
    }

    // [DATA] SUBMESSAGE
    {
        uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};

        DataPayloadSpec payload;
        payload.request_id = 0x87654321;
        payload.object_id = 0x778899;
        payload.data_reader.read_mode = READ_MODE_SAMPLE;

        SampleKindSpec* kind = &payload.data_reader.sample_kind;
        switch(payload.data_reader.read_mode)
        {
            case READ_MODE_DATA:
                kind->data.serialized_data = data;
                kind->data.serialized_data_size = 5;
            break;

            case READ_MODE_SAMPLE:
                kind->sample.info.state = 0x08;
                kind->sample.info.sequence_number = 0x01234567;
                kind->sample.info.session_time_offset = 0xAAAABBBB;
                kind->sample.data.serialized_data = data;
                kind->sample.data.serialized_data_size = 5;
            break;
        }

        add_data_submessage(message, &payload);
    }
}

