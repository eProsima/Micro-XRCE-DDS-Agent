#include "micrortps/client/client.h"

#include "micrortps/client/topic.h"
#include "micrortps/client/message.h"
#include "micrortps/client/dynamic_buffer.h"

#ifdef DEBUG
#include "micrortps/client/debug/message_debugger.h"
#endif

#include <stdlib.h>
#include <stdio.h>

// ---------------------------------------------------------------------
//                          INTERNAL FUNCTIONS DEFINITION
// ---------------------------------------------------------------------
// Callbacks definition
void on_message_header_received(const MessageHeaderSpec* header, void* data);
void on_status_received(const StatusPayloadSpec* payload, void* data);
void on_data_received(const DataPayloadSpec* payload, void* data);

//Util
void add_xrce_object(XRCEObject* object);
void remove_xrce_object(XRCEObject* object);

void create_message_header();

// For the prototype
Participant* create_participant();
// ---------------------------------------------------------------------
//                          CLIENT INTERNAL STATE
// ---------------------------------------------------------------------
// Client message header
uint32_t client_key;
uint8_t session_id;
uint8_t stream_id;

uint16_t current_sequence_number_sent;
uint16_t expected_sequence_number_recieved;

uint8_t message_header_prepared;

// Counters to create message ids
uint32_t request_counter;
uint32_t object_id_counter;

// DDS Objects;
XRCEObject* xrce_object_list[100] = {}; //make dynamically. Make it as a hash table.
uint32_t xrce_object_list_size;

Participant* participant_list[10] = {}; //make dynamically
uint32_t participant_list_size;

// Message manegement.
MessageManager message_manager;
DynamicBuffer topic_dynamic_buffer;

// External input and output
DataOutEvent send_data_io;
DataInEvent receive_data_io;

void* data_io;


// ---------------------------------------------------------------------
//                          CLIENT INTERNAL STATE
// ---------------------------------------------------------------------
void add_xrce_object(XRCEObject* object)
{
    object->id = ++object_id_counter;
    object->status = STATUS_UNKNOWN;
    xrce_object_list[xrce_object_list_size++] = object;
}

void remove_xrce_object(XRCEObject* object)
{
    //TODO
}

void init_client(uint32_t buffer_size, DataOutEvent send_data_io_, DataInEvent receive_data_io_,
    void* data_io_)
{
    // Client message header
    current_sequence_number_sent = 0;
    expected_sequence_number_recieved = 0;

    message_header_prepared = 0;

    // External input and output
    send_data_io = send_data_io_;
    receive_data_io = receive_data_io_;
    data_io = data_io_;

    // Message manegement.
    MessageCallback callback = {};
    callback.on_message_header = on_message_header_received;
    callback.on_status = on_status_received;
    callback.on_data = on_data_received;

    buffer_size = (buffer_size > 64) ? buffer_size : 64;
    init_message_manager(&message_manager, malloc(buffer_size), buffer_size,
        malloc(buffer_size), buffer_size, callback);

    init_memory_buffer(&topic_dynamic_buffer, 0);

    // Counters
    request_counter = 0;
    object_id_counter = 0;

    // Element lists
    participant_list_size = 0;
    xrce_object_list_size = 0;
}

void destroy_client()
{
    //TODO
}

void create_message_header()
{
    MessageHeaderSpec header;
    header.client_key = 0xF1F2F3F4;
    header.session_id = 0x01;
    header.stream_id = 0x01;
    header.sequence_number = current_sequence_number_sent++;

    start_message(&message_manager, &header);
    message_header_prepared = 1;
}

Participant* create_participant()
{
    Participant* participant = malloc(sizeof(Participant));
    participant->publisher_list_size = 0;
    participant->subscriber_list_size = 0;
    participant_list[participant_list_size++] = participant;
    add_xrce_object(&participant->object);

    CreatePayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = participant->object.id;
    payload.object.kind = OBJECT_KIND_PARTICIPANT;
    payload.object.string_size = 0;

    if(!message_header_prepared)
        create_message_header();

    add_create_submessage(&message_manager, &payload);

    #ifdef DEBUG
    printf("==> ");
    printl_create_submessage(&payload, NULL);
    #endif

    return participant;
}

Publisher* create_publisher(Topic* topic)
{
    Participant* participant = (participant_list[0]) ? participant_list[0] : create_participant();

    Publisher* publisher = malloc(sizeof(Publisher));
    publisher->participant = participant;
    publisher->topic = topic;
    participant->publisher_list[participant->publisher_list_size++] = publisher;
    add_xrce_object(&publisher->object);

    CreatePayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = publisher->object.id;
    payload.object.kind = OBJECT_KIND_PUBLISHER;
    payload.object.string = topic->name;
    payload.object.string_size = strlen(topic->name) + 1;
    payload.object.variant.publisher.participant_id = participant->object.id;

    if(!message_header_prepared)
        create_message_header();

    add_create_submessage(&message_manager, &payload);

    #ifdef DEBUG
    printf("==> ");
    printl_create_submessage(&payload, NULL);
    #endif

    return publisher;
}

Subscriber* create_subscriber(Topic* topic)
{
    Participant* participant = (participant_list[0]) ? participant_list[0] : create_participant();

    Subscriber* subscriber = malloc(sizeof(Subscriber));
    subscriber->participant = participant;
    subscriber->listeners_size = 0;
    subscriber->topic = topic;
    subscriber->remaning_messages = 0;
    participant->subscriber_list[participant->subscriber_list_size++] = subscriber;
    add_xrce_object(&subscriber->object);

    CreatePayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = subscriber->object.id;
    payload.object.kind = OBJECT_KIND_SUBSCRIBER;
    payload.object.string = topic->name;
    payload.object.string_size = strlen(topic->name) + 1;
    payload.object.variant.subscriber.participant_id = participant->object.id;

    if(!message_header_prepared)
        create_message_header();

    add_create_submessage(&message_manager, &payload);

    #ifdef DEBUG
    printf("==> ");
    printl_create_submessage(&payload, NULL);
    #endif

    return subscriber;
}

void send_topic(Publisher* publisher, void* topic_data)
{
    reset_memory_buffer(&topic_dynamic_buffer);

    uint32_t topic_size = publisher->topic->size_of(topic_data);
    uint8_t* topic_buffer = request_memory_buffer(&topic_dynamic_buffer, topic_size);

    SerializedBufferHandle writer;
    init_serialized_buffer(&writer, topic_buffer, topic_size);
    publisher->topic->serialize(&writer, topic_data);


    WriteDataPayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = publisher->object.id;
    payload.data_writer.read_mode = READ_MODE_DATA;
    payload.data_writer.sample_kind.data.serialized_data = topic_buffer;
    payload.data_writer.sample_kind.data.serialized_data_size = topic_size;

    if(!message_header_prepared)
        create_message_header();

    add_write_data_submessage(&message_manager, &payload);

    #ifdef DEBUG
    printf("==> ");
    printl_write_data_submessage(&payload, NULL);
    #endif
}

void read_data(Subscriber* subscriber, uint16_t max_messages)
{
    subscriber->remaning_messages = max_messages;

    ReadDataPayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = subscriber->object.id;
    payload.max_messages = max_messages;
    payload.expression_size = 0;

    if(!message_header_prepared)
        create_message_header();

    add_read_data_submessage(&message_manager, &payload);

    #ifdef DEBUG
    printf("==> ");
    printl_read_data_submessage(&payload, NULL);
    #endif
}

void add_listener_topic(Subscriber* subscriber, OnListenerTopic on_listener_topic, void* callback_object)
{
    SubscriberListener* listener = &subscriber->listener_list[subscriber->listeners_size++];
    listener->on_topic = on_listener_topic;
    listener->callback_object = callback_object;
}

void delete_publisher(Publisher* publisher)
{
    //TODO
}

void delete_subscriber(Subscriber* subscriber)
{
    //TODO
}

void update_communication()
{
    // RECEIVE
    uint8_t* in_buffer = message_manager.reader.data;
    uint32_t in_size = message_manager.reader.final - in_buffer;

    uint32_t in_length = receive_data_io(in_buffer, in_size, data_io);
    if(in_length > 0)
    {
        parse_message(&message_manager, in_length);
    }

    #ifdef DEBUG
    printf("<-- [Received %u bytes]\n", in_length);
    #endif

    // SEND
    uint8_t* out_buffer = message_manager.writer.data;
    uint32_t out_length = message_manager.writer.iterator - out_buffer;
    if(out_length > 0)
    {
        send_data_io(out_buffer, out_length, data_io);
        reset_buffer_iterator(&message_manager.writer);
        message_header_prepared = 0;

        #ifdef DEBUG
        printf("--> [Send %u bytes]\n", out_length);
        #endif
    }

}

void on_message_header_received(const MessageHeaderSpec* header, void* data)
{
    if(expected_sequence_number_recieved == header->sequence_number)
    {
        #ifdef DEBUG
        printf("    <<Sequence number | expected: %u | received: %u>>\n",
            expected_sequence_number_recieved, header->sequence_number);
        expected_sequence_number_recieved = header->sequence_number + 1;
        #endif
    }
    else
    {
        #ifdef DEBUG
        printf("\e[1;31mERROR:\e[0m expected s_nr: %u, found: %u\n",
            expected_sequence_number_recieved, header->sequence_number);
        #endif
    }
}

void on_status_received(const StatusPayloadSpec* payload, void* data)
{
    #ifdef DEBUG
    printf("<== ");
    printl_status_submessage(payload, NULL);
    #endif

    for(uint32_t i = 0; i < xrce_object_list_size; i++)
        if(xrce_object_list[i]->id == payload->object_id)
        {
            xrce_object_list[i]->status = payload->result.status;
            return;
        }
}

void on_data_received(const DataPayloadSpec* payload, void* data)
{
    //TODO
}

uint32_t get_xrce_object(uint32_t id, void** object)
{
    for(uint32_t i = 0; i < participant_list_size; i++)
    {
        Participant* participant = participant_list[i];
        if(participant->object.id == id)
        {
            *object = participant;
            return OBJECT_PARTICIPANT;
        }

        for(uint32_t p = 0; p < participant->publisher_list_size; p++)
        {
            if(participant->publisher_list[p]->object.id == id)
            {
                *object = participant->publisher_list[p];
                return OBJECT_PUBLISHER;
            }
        }

        for(uint32_t s = 0; s < participant->subscriber_list_size; s++)
        {
            if(participant->subscriber_list[s]->object.id == id)
            {
                *object = participant->subscriber_list[s];
                return OBJECT_SUBSCRIBER;
            }
        }
    }

    return 0;
}