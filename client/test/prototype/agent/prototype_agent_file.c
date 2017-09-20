#include <micrortps/client/message.h>
#include <micrortps/client/debug/message_debugger.h>

#include "prototype/shape_topic.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 2000

typedef struct Agent
{
    MessageManager message_manager;
    MemoryCache cache;
    uint16_t current_sequence_number_sent;
    uint16_t expected_sequence_number_recieved;
    uint32_t messages_to_send;
    uint32_t subscriber_request_id;
    uint32_t subscriber_object_id;

} Agent;


void send_topic(Agent* agent, uint32_t key);

int initialize_message(MessageHeaderSpec* header, void* data);
int on_message_header_received(const MessageHeaderSpec* header, void* data);
void on_create_submessage_received(const CreatePayloadSpec* payload, void* data);
void on_delete_submessage_received(const DeletePayloadSpec* payload, void* data);
void on_write_data_submessage_received(const WriteDataPayloadSpec* payload, void* data);
void on_read_data_submessage_received(const ReadDataPayloadSpec* payload, void* data);

int main(int args, char** argv)
{
    uint8_t in_buffer[BUFFER_SIZE] = {};
    uint8_t out_buffer[BUFFER_SIZE] = {};

    Agent agent = {};
    agent.current_sequence_number_sent = 0;
    agent.expected_sequence_number_recieved = 0;
    init_memory_cache(&agent.cache, 0);
    agent.messages_to_send = 0;
    agent.subscriber_request_id = 0;
    agent.subscriber_object_id = 0;

    MessageCallback callback = {};
    callback.on_initialize_message = initialize_message;
    callback.on_message_header = on_message_header_received;
    callback.on_create_resource = on_create_submessage_received;
    callback.on_delete_resource = on_delete_submessage_received;
    callback.on_write_data = on_write_data_submessage_received;
    callback.on_read_data = on_read_data_submessage_received;
    callback.data = &agent;

    init_message_manager(&agent.message_manager, out_buffer, BUFFER_SIZE, in_buffer, BUFFER_SIZE, callback);

    char in_file_name[256] = "client_to_agent.bin";
    char out_file_name[256] = "agent_to_client.bin";
    fclose(fopen(in_file_name, "wb"));

    uint32_t dt = 2;
    char time_string[80];
    while(1)
    {
        time_t t = time(NULL);
        struct tm* timeinfo = localtime(&t);
        strftime(time_string, 80, "%T", timeinfo);
        printf("============================ AGENT ========================> %s\n", time_string);

        // Recieved file
        FILE* in_file = fopen(in_file_name, "rb");
        fseek(in_file, 0L, SEEK_END);
        uint32_t in_file_size = ftell(in_file);
        fseek(in_file, 0L, SEEK_SET);
        fread(in_buffer, 1, in_file_size, in_file);
        fclose(in_file);
        fclose(fopen(in_file_name,"wb"));

        parse_message(&agent.message_manager, in_file_size);

        // send topic if clien request it.
        if(agent.messages_to_send)
            send_topic(&agent, timeinfo->tm_sec);

        printf("<-- [Received %u bytes]\n", in_file_size);

        // Send file
        uint32_t out_file_size = agent.message_manager.writer.iterator - agent.message_manager.writer.data;
        if(out_file_size > 0)
        {
            FILE* out_file = fopen(out_file_name, "wb");

            fwrite(out_buffer, 1, out_file_size, out_file);
            fclose(out_file);

            reset_buffer_iterator(&agent.message_manager.writer);
            printf("--> [Send %u bytes]\n", out_file_size);
        }

        // waiting...
        usleep(1000000 * dt);
    }

    return 0;
}

int initialize_message(MessageHeaderSpec* header, void* data)
{
    Agent* agent = (Agent*)data;

    header->client_key = 0xF1F2F3F4;
    header->session_id = 0x01;
    header->stream_id = 0x01;
    header->sequence_number = agent->current_sequence_number_sent++;

    return 1;
}


int on_message_header_received(const MessageHeaderSpec* header, void* data)
{
    Agent* agent = (Agent*)data;

    printf("    <<Sequence number | expected: %u | received: %u>>\n",
            agent->expected_sequence_number_recieved, header->sequence_number);

    if(agent->expected_sequence_number_recieved == header->sequence_number)
    {
        agent->expected_sequence_number_recieved = header->sequence_number + 1;
        return 1;
    }
    return 0;
}

void on_create_submessage_received(const CreatePayloadSpec* recv_payload, void* data)
{
    printf("<== ");
    printl_create_submessage(recv_payload, NULL);

    Agent* agent = (Agent*)data;

    StatusPayloadSpec payload;
    payload.result.request_id = recv_payload->request_id;
    payload.result.status = STATUS_OK;
    payload.result.implementation_status = STATUS_LAST_OP_CREATE;
    payload.object_id = recv_payload->object_id;

    add_status_submessage(&agent->message_manager, &payload);

    printf("==> ");
    printl_status_submessage(&payload, NULL);
}

void on_delete_submessage_received(const DeletePayloadSpec* recv_payload, void* data)
{
    print_delete_submessage(recv_payload, NULL);
    //TODO
}

void on_write_data_submessage_received(const WriteDataPayloadSpec* recv_payload, void* data)
{
    printf("<== ");
    printl_write_data_submessage(recv_payload, NULL);

    Agent* agent = (Agent*)data;

    uint32_t topic_size = recv_payload->data_writer.sample_kind.data.serialized_data_size;
    uint8_t* topic_data = recv_payload->data_writer.sample_kind.data.serialized_data;

    SerializedBufferHandle reader;
    init_serialized_buffer(&reader, topic_data, topic_size);

    Topic topic =
    {
        "SQUARE",
        serialize_shape_topic,
        deserialize_shape_topic,
        size_of_shape_topic
    };

    void* user_topic = topic.deserialize(&reader, agent);

    if(topic_size > 0)
        print_shape_topic(user_topic);
}

void on_read_data_submessage_received(const ReadDataPayloadSpec* recv_payload, void* data)
{
    printf("<== ");
    printl_read_data_submessage(recv_payload, NULL);

    Agent* agent = (Agent*)data;
    agent->messages_to_send = recv_payload->max_messages;
    agent->subscriber_request_id = recv_payload->request_id;
    agent->subscriber_object_id = recv_payload->object_id;
}

void send_topic(Agent* agent, uint32_t key)
{
    agent->messages_to_send--;
    Topic topic =
    {
        "SQUARE",
        serialize_shape_topic,
        deserialize_shape_topic,
        size_of_shape_topic
    };

    ShapeTopic topic_data = {6, "GREEN", 0, 0, key};

    uint8_t topic_buffer[1024];
    uint32_t topic_size = topic.size_of(&topic_data);

    SerializedBufferHandle writer;
    init_serialized_buffer(&writer, topic_buffer, topic_size);
    topic.serialize(&writer, &topic_data, agent);

    DataPayloadSpec payload;
    payload.request_id = agent->subscriber_request_id;
    payload.object_id = agent->subscriber_object_id;
    payload.data_reader.read_mode = READ_MODE_DATA;
    payload.data_reader.sample_kind.data.serialized_data = topic_buffer;
    payload.data_reader.sample_kind.data.serialized_data_size = topic_size;

    add_data_submessage(&agent->message_manager, &payload);

    printf("==> ");
    printl_data_submessage(&payload, NULL);

    print_shape_topic(&topic_data);
}