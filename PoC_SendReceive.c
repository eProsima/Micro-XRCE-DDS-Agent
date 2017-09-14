#include <client/message.h>
#include <transport/ddsxrce_transport.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 2000

int client_test_main(int argc, char *argv[]);
void create_message(SerializedBufferHandle* message);
void on_message_header_received(const MessageHeaderSpec* header);
void on_submessage_header_received(const SubmessageHeaderSpec* header);
void on_create_submessage_received(const CreatePayloadSpec* payload);
void on_delete_resource_received(const DeletePayloadSpec* payload);
void on_write_data_submessage_received(const WriteDataPayloadSpec* payload);
void on_read_data_received(const ReadDataPayloadSpec* payload);
void on_data_received(const DataPayloadSpec* payload);

int client_test_main(int argc, char *argv[])
{

    // Mesage creation
    octet out_buffer[1024] = {};
    octet in_buffer[1024] = {};
    size_t buffer_len = 1024;

    // Init transport
    locator_t loc =
    {
      LOC_SERIAL,
      "/dev/ttyACM0"
    };

    int ret = 0;
    channel_id_t ch_id = add_locator(&loc);

    // Init parser
    MessageCallback callback;
    callback.message_header = on_message_header_received;
    callback.submessage_header = on_submessage_header_received;
    callback.create_resource = on_create_submessage_received;
    callback.delete_resource = on_delete_resource_received;
    callback.write_data = on_write_data_submessage_received;
    callback.read_data = on_read_data_received;
    callback.data = on_data_received;

    printf("%s\n\n", "SERIALIZATION:");
    SerializedBufferHandle out_message, in_message;
    init_serialized_buffer(&out_message, out_buffer, buffer_len);
    create_message(&out_message);


    uint32_t seliarized_size = out_message.iterator - out_message.data;
    printf(" %u serialized bytes. \n", seliarized_size);

    int loops = 1000;
    while (loops--)
    {
        printf("----------------------------------------------\n");
        printf("                      SEND                    \n");
        printf("----------------------------------------------\n");

        if (0 < (ret = send(out_buffer, seliarized_size, loc.kind, ch_id)))
        {
            printf(" ");
            for(uint8_t* i = out_message.data; i < out_message.iterator; i++)
            {
                printf("%02X ", *i);
                if((i - out_message.data + 1) % 16 == 0)
                    printf("\n ");
            }
            printf("\n\n");
            printf(" %u serialized bytes. \n", seliarized_size);

            printf("SEND: %d bytes\n", ret);
        }
        else
        {
            printf("SEND ERROR: %d\n", ret);
        }

        usleep(2000000);

        printf("----------------------------------------------\n");
        printf("                    RECEIVED                  \n");
        printf("----------------------------------------------\n");

        if (0 < (ret = receive(in_buffer, buffer_len, loc.kind, ch_id)))
        {
            printf("RECV: %d bytes\n", ret);
            init_serialized_buffer(&in_message, in_buffer, ret);
            parse_message(&in_message, &callback);

            uint32_t parsed_seliarized_size = in_message.iterator - in_message.data;
            printf(" %u serialized bytes. \n", parsed_seliarized_size);
        }
        else
        {
            printf("RECV ERROR: %d\n", ret);
        }

        usleep(2000000);
    }

    printf("exiting...\n");
    return 0;
}

void create_message(SerializedBufferHandle* message)
{
    MessageHeaderSpec header;
    header.client_key = 0xF1F2F3F4;
    header.session_id = 0x01;
    header.stream_id = 0x02;
    header.sequence_nr = 1234;
    init_message(message, &header);


    // [CREATE] SUBMESSAGE
    {
        char string[] = "Hello world";

        CreatePayloadSpec payload;
        payload.request_id = 0xAABBCCDD;
        payload.object_id = 0x778899;
        payload.object.kind = OBJECT_KIND_DATAWRITER;

        payload.object.string = string;
        payload.object.string_size = strlen(string) + 1;

        switch(payload.object.kind)
        {
            case OBJECT_KIND_DATAWRITER:
                payload.object.variant.data_writer.participant_id = 0xAAAAAA;
                payload.object.variant.data_writer.publisher_id = 0xBBBBBB;
            break;

            case OBJECT_KIND_DATAREADER:
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

    /*
    // [DELETE] SUBMESSAGE
    {
        DeletePayloadSpec payload;
        payload.request_id = 0x12345678;
        payload.object_id = 0xABCDEF;

        add_delete_submessage(message, &payload);
    }


    // [WRITE_DATA] SUBMESSAGE
    {
        uint8_t data[] = "This data has been sent!";

        WriteDataPayloadSpec payload;
        payload.request_id = 0xAABBCCDD;
        payload.object_id = 0x778899;
        payload.data_writer.read_mode = READ_MODE_SAMPLE;

        SampleKindSpec* kind = &payload.data_writer.sample_kind;
        switch(payload.data_writer.read_mode)
        {
            case READ_MODE_DATA:
                kind->data.serialized_data = data;
                kind->data.serialized_data_size = strlen((char*)data) + 1;
            break;

            case READ_MODE_SAMPLE:
                kind->sample.info.state = 0x01;
                kind->sample.info.sequence_number = 0xFFFFFFFF;
                kind->sample.info.session_time_offset = 0xAAAAAAAA;
                kind->sample.data.serialized_data = data;
                kind->sample.data.serialized_data_size = strlen((char*)data) + 1;
            break;
        }

        add_write_data_submessage(message, &payload);
    }

    // [READ_DATA] SUBMESSAGE
    {
        char expresion[] = "EXPRESION";

        ReadDataPayloadSpec payload;
        payload.request_id = 0x11223344;
        payload.max_messages = 12345;
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
        uint8_t data[] = "This data has been recieved!";

        DataPayloadSpec payload;
        payload.request_id = 0x87654321;
        payload.object_id = 0xFEDCBA;
        payload.data_reader.read_mode = READ_MODE_SAMPLE;

        SampleKindSpec* kind = &payload.data_reader.sample_kind;
        switch(payload.data_reader.read_mode)
        {
            case READ_MODE_DATA:
                kind->data.serialized_data = data;
                kind->data.serialized_data_size = strlen((char*)data) + 1;
            break;

            case READ_MODE_SAMPLE:
                kind->sample.info.state = 0x08;
                kind->sample.info.sequence_number = 0xAAAAAAAA;
                kind->sample.info.session_time_offset = 0xBBBBBBBB;
                kind->sample.data.serialized_data = data;
                kind->sample.data.serialized_data_size = strlen((char*)data) + 1;
            break;
        }

        add_data_submessage(message, &payload);
    } */
}

void on_message_header_received(const MessageHeaderSpec* header)
{
    printf("<Header> \n");
    printf("  - client_key: 0x%08X\n", header->client_key);
    printf("  - session_id: 0x%02X\n", header->session_id);
    printf("  - stream_id: 0x%02X\n", header->stream_id);
    printf("  - sequence_nr: %u\n", header->sequence_nr);
    printf("\n\n");
}

void on_submessage_header_received(const SubmessageHeaderSpec* header)
{
    switch(header->id)
    {
        case SUBMESSAGE_CREATE:
            printf("<Submessage> [CREATE] \n");
        break;
        case SUBMESSAGE_DELETE:
            printf("<Submessage> [DELETE]\n");
        break;
        case SUBMESSAGE_WRITE_DATA:
            printf("<Submessage> [WRITE_DATA]\n");
        break;
        case SUBMESSAGE_READ_DATA:
            printf("<Submessage> [READ_DATA]\n");
        break;
        case SUBMESSAGE_DATA:
            printf("<Submessage> [DATA]\n");
        break;
    }

    printf("  <Submessage header> \n");
    printf("  - id: 0x%02X\n", header->id);
    printf("  - flags: 0x%02X\n", header->flags);
    printf("  - length: %u\n", header->length);
    printf("\n");
}

void on_create_submessage_received(const CreatePayloadSpec* payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload->request_id);
    printf("  - object_id: 0x%06X\n", payload->object_id);
    printf("  - kind: 0x%02X\n", payload->object.kind);
    printf("  - string_size: 0x%08X\n", payload->object.string_size);
    printf("  - string: %s\n", payload->object.string);

    const ObjectVariantSpec* variant = &payload->object.variant;
    switch(payload->object.kind)
    {
        case OBJECT_KIND_DATAWRITER:
            printf("    <Data writer>\n");
            printf("    - participan_id: 0x%06X\n", variant->data_writer.participant_id);
            printf("    - publisher_id: 0x%06X\n", variant->data_writer.publisher_id);
        break;

        case OBJECT_KIND_DATAREADER:
            printf("    <Data reader>\n");
            printf("    - participan_id: 0x%06X\n", variant->data_reader.participant_id);
            printf("    - subscriber_id: 0x%06X\n", variant->data_reader.subscriber_id);
        break;

        case OBJECT_KIND_SUBSCRIBER:
            printf("    <Data subscriber>\n");
            printf("    - participan_id: 0x%06X\n", variant->subscriber.participant_id);
        break;

        case OBJECT_KIND_PUBLISHER:
            printf("    <Data publisher>\n");
            printf("    - participan_id: 0x%06X\n", variant->publisher.participant_id);
        break;
    }
    printf("\n\n");
}

void on_delete_resource_received(const DeletePayloadSpec* payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload->request_id);
    printf("  - object_id: 0x%06X\n", payload->object_id);
    printf("\n\n");
}

void on_write_data_submessage_received(const WriteDataPayloadSpec* payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload->request_id);
    printf("  - object_id: 0x%06X\n", payload->object_id);
    printf("  - read_mode: 0x%02X\n", payload->data_writer.read_mode);

    const SampleKindSpec* kind = &payload->data_writer.sample_kind;
    switch(payload->data_writer.read_mode)
    {
        case READ_MODE_DATA:
            printf("    <Data>\n");
            printf("    - serialized_data_size: 0x%08X\n", kind->data.serialized_data_size);
            printf("    - serialized_data: %s\n", (char*)kind->data.serialized_data);
        break;

        case READ_MODE_SAMPLE:
            printf("    - state: %02X\n", kind->sample.info.state);
            printf("    - sequence_number: 0x%016lX\n", kind->sample.info.sequence_number);
            printf("    - session_time_offset: 0x%08X\n", kind->sample.info.session_time_offset);
            printf("    - serialized_data_size: 0x%08X\n", kind->sample.data.serialized_data_size);
            printf("    - serialized_data: %s\n", (char*)kind->sample.data.serialized_data);
        break;
    }
    printf("\n\n");
}

void on_read_data_received(const ReadDataPayloadSpec* payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload->request_id);
    printf("  - object_id: %hu\n", payload->max_messages);
    printf("  - object_id: %u\n", payload->max_elapsed_time);
    printf("  - object_id: %u\n", payload->max_rate);
    printf("  - object_id: %u\n", payload->expression_size);
    printf("  - object_id: %s\n", payload->content_filter_expression);
    printf("  - object_id: %hu\n", payload->max_samples);
    printf("  - object_id: 0x%02X\n", payload->include_sample_info);
    printf("\n\n");
}

void on_data_received(const DataPayloadSpec* payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload->request_id);
    printf("  - object_id: 0x%06X\n", payload->object_id);
    printf("  - read_mode: 0x%02X\n", payload->data_reader.read_mode);

    const SampleKindSpec* kind = &payload->data_reader.sample_kind;
    switch(payload->data_reader.read_mode)
    {
        case READ_MODE_DATA:
            printf("    <Data>\n");
            printf("    - serialized_data_size: 0x%08X\n", kind->data.serialized_data_size);
            printf("    - serialized_data: %s\n", (char*)kind->data.serialized_data);
        break;

        case READ_MODE_SAMPLE:
            printf("    - state: 0x%02X\n", kind->sample.info.state);
            printf("    - sequence_number: 0x%016lX\n", kind->sample.info.sequence_number);
            printf("    - session_time_offset: 0x%08X\n", kind->sample.info.session_time_offset);
            printf("    - serialized_data_size: 0x%08X\n", kind->sample.data.serialized_data_size);
            printf("    - serialized_data: %s\n", (char*)kind->sample.data.serialized_data);
        break;
    }
    printf("\n\n");
}

int main(int argc, char *argv[])
{
    return client_test_main(argc, argv);
}
