#include "micrortps/client/client.h"

#include "micrortps/client/topic.h"
#include "micrortps/client/message.h"
#include "micrortps/client/memory_cache.h"

#ifdef _VERBOSE_
#include "micrortps/client/debug/message_debugger.h"
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>


// ---------------------------------------------------------------------
//                          INTERNAL FUNCTIONS DEFINITION
// ---------------------------------------------------------------------
// Callbacks definition
int on_initialize_message(MessageHeaderSpec* header, void* data);
int on_message_header_received(const MessageHeaderSpec* header, void* data);
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
MemoryCache topic_cache;

// External input and output
DataOutEvent send_data_io;
DataInEvent receive_data_io;

void* data_io;
void* callback_object;


// ---------------------------------------------------------------------
//                          CLIENT INTERNAL STATE
// ---------------------------------------------------------------------
void add_xrce_object(XRCEObject* object)
{
    object->id = ++object_id_counter;
    object->status = OBJECT_STATUS_UNKNOWN;
    xrce_object_list[xrce_object_list_size++] = object;
}

void remove_xrce_object(XRCEObject* object)
{
    //TODO
}

void init_client(uint32_t buffer_size, DataOutEvent send_data_io_, DataInEvent receive_data_io_,
    void* data_io_, void* callback_object_)
{
    // Client message header
    current_sequence_number_sent = 0;
    expected_sequence_number_recieved = 0;

    // External input and output
    send_data_io = send_data_io_;
    receive_data_io = receive_data_io_;
    data_io = data_io_;

    callback_object = callback_object_;

    // Message manegement.
    MessageCallback callback = {};
    callback.on_initialize_message = on_initialize_message;
    callback.on_message_header = on_message_header_received;
    callback.on_status = on_status_received;
    callback.on_data = on_data_received;

    buffer_size = (buffer_size > 64) ? buffer_size : 64;
    init_message_manager(&message_manager, malloc(buffer_size), buffer_size,
        malloc(buffer_size), buffer_size, callback);

    init_memory_cache(&topic_cache, 0);

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

int on_initialize_message(MessageHeaderSpec* header, void* data)
{
    header->client_key = 0xF1F2F3F4;
    header->session_id = 0x01;
    header->stream_id = 0x01;
    header->sequence_number = current_sequence_number_sent++;

    return 1;
}

Participant* create_participant()
{
    Participant* participant = malloc(sizeof(Participant));
    participant->publisher_list_size = 0;
    participant->subscriber_list_size = 0;
    participant->object.last_operation = STATUS_LAST_OP_CREATE;
    participant_list[participant_list_size++] = participant;
    add_xrce_object(&participant->object);

    CreatePayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = participant->object.id;
    payload.object.kind = OBJECT_KIND_PARTICIPANT;
    payload.object.string_size = 0;

    add_create_submessage(&message_manager, &payload);

    #ifdef _VERBOSE_
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
    publisher->object.last_operation = STATUS_LAST_OP_CREATE;
    participant->publisher_list[participant->publisher_list_size++] = publisher;
    add_xrce_object(&publisher->object);

    CreatePayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = publisher->object.id;
    payload.object.kind = OBJECT_KIND_PUBLISHER;
    payload.object.string = topic->name;
    payload.object.string_size = strlen(topic->name) + 1;
    payload.object.variant.publisher.participant_id = participant->object.id;

    add_create_submessage(&message_manager, &payload);

    #ifdef _VERBOSE_
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
    subscriber->listener_list_size = 0;
    subscriber->topic = topic;
    subscriber->remaning_messages = 0;
    subscriber->object.last_operation = STATUS_LAST_OP_CREATE;
    participant->subscriber_list[participant->subscriber_list_size++] = subscriber;
    add_xrce_object(&subscriber->object);

    CreatePayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = subscriber->object.id;
    payload.object.kind = OBJECT_KIND_SUBSCRIBER;
    payload.object.string = topic->name;
    payload.object.string_size = strlen(topic->name) + 1;
    payload.object.variant.subscriber.participant_id = participant->object.id;

    add_create_submessage(&message_manager, &payload);

    #ifdef _VERBOSE_
    printf("==> ");
    printl_create_submessage(&payload, NULL);
    #endif

    return subscriber;
}

int send_topic(Publisher* publisher, void* topic_data)
{
    if(publisher->object.status != OBJECT_STATUS_AVALIABLE)
        return 0;

    uint32_t topic_size = publisher->topic->size_of(topic_data);
    uint8_t* topic_buffer = reset_memory_cache(&topic_cache, topic_size);

    SerializedBufferHandle writer;
    init_serialized_buffer(&writer, topic_buffer, topic_size);
    publisher->topic->serialize(&writer, topic_data, callback_object);

    WriteDataPayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = publisher->object.id;
    payload.data_writer.read_mode = READ_MODE_DATA;
    payload.data_writer.sample_kind.data.serialized_data = topic_buffer;
    payload.data_writer.sample_kind.data.serialized_data_size = topic_size;

    add_write_data_submessage(&message_manager, &payload);

    #ifdef _VERBOSE_
    printf("==> ");
    printl_write_data_submessage(&payload, NULL);
    #endif

    return 1;
}

int read_data(Subscriber* subscriber, uint16_t max_messages)
{
    if(subscriber->object.status != OBJECT_STATUS_AVALIABLE)
        return 0;

    subscriber->remaning_messages = max_messages;

    ReadDataPayloadSpec payload = {}; //compleate
    payload.request_id = ++request_counter;
    payload.object_id = subscriber->object.id;
    payload.max_messages = max_messages;
    payload.expression_size = 0;

    add_read_data_submessage(&message_manager, &payload);

    #ifdef _VERBOSE_
    printf("==> ");
    printl_read_data_submessage(&payload, NULL);
    #endif

    return 1;
}

void add_listener_topic(Subscriber* subscriber, OnListenerTopic on_listener_topic)
{
    subscriber->listener_list[subscriber->listener_list_size++] = on_listener_topic;
}

void delete_publisher(Publisher* publisher)
{
    if(publisher->object.status != OBJECT_STATUS_AVALIABLE)
        return;

    publisher->object.last_operation = STATUS_LAST_OP_DELETE;

    DeletePayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = publisher->object.id;

    add_delete_submessage(&message_manager, &payload);

    #ifdef _VERBOSE_
    printf("==> ");
    printl_delete_submessage(&payload, NULL);
    #endif
}

void delete_subscriber(Subscriber* subscriber)
{
    if(subscriber->object.status != OBJECT_STATUS_AVALIABLE)
        return;

    subscriber->object.last_operation = STATUS_LAST_OP_DELETE;

    DeletePayloadSpec payload;
    payload.request_id = ++request_counter;
    payload.object_id = subscriber->object.id;

    add_delete_submessage(&message_manager, &payload);

    #ifdef _VERBOSE_
    printf("==> ");
    printl_delete_submessage(&payload, NULL);
    #endif
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

    #ifdef _VERBOSE_
    printf("<-- [Received %u bytes]\n", in_length);
    #endif

    // SEND
    uint8_t* out_buffer = message_manager.writer.data;
    uint32_t out_length = message_manager.writer.iterator - out_buffer;
    if(out_length > 0)
    {
        send_data_io(out_buffer, out_length, data_io);
        reset_buffer_iterator(&message_manager.writer);

        #ifdef _VERBOSE_
        printf("--> [Send %u bytes]\n", out_length);
        #endif
    }

}

int on_message_header_received(const MessageHeaderSpec* header, void* data)
{
    #ifdef _VERBOSE_
        printf("    <<Sequence number | expected: %u | received: %u>>\n",
            expected_sequence_number_recieved, header->sequence_number);
     #endif

    if(expected_sequence_number_recieved == header->sequence_number)
    {
        expected_sequence_number_recieved = header->sequence_number + 1;
        return 1;
    }
    return 0;
}

void on_status_received(const StatusPayloadSpec* payload, void* data)
{
    #ifdef _VERBOSE_
    printf("<== ");
    printl_status_submessage(payload, NULL);
    #endif

    for(uint32_t i = 0; i < xrce_object_list_size; i++) //to hash table
        if(xrce_object_list[i]->id == payload->object_id)
        {
            switch(payload->result.last_operation)
            {
                case STATUS_LAST_OP_CREATE:
                    if(payload->result.status == STATUS_OK ||
                       payload->result.status == STATUS_OK_MATCHED)
                        xrce_object_list[i]->status = OBJECT_STATUS_AVALIABLE;
                    else
                        xrce_object_list[i]->status = OBJECT_STATUS_UNAVALIABLE;
                break;

                case STATUS_LAST_OP_DELETE:
                    if(payload->result.status == STATUS_OK)
                        xrce_object_list[i]->status = OBJECT_STATUS_UNAVALIABLE;
                break;
            }
            return;
        }
}

void on_data_received(const DataPayloadSpec* payload, void* data)
{
    #ifdef _VERBOSE_
    printf("<== ");
    printl_data_submessage(payload, NULL);
    #endif

    void* object;
    if(get_xrce_object(payload->object_id, &object) == OBJECT_SUBSCRIBER)
    {
        Subscriber* subscriber = (Subscriber*)object;

        if(subscriber->object.status != OBJECT_STATUS_AVALIABLE)
            return;

        if(subscriber->remaning_messages > 0)
            subscriber->remaning_messages--;

        uint32_t topic_size = payload->data_reader.sample_kind.data.serialized_data_size;
        uint8_t* topic_data = payload->data_reader.sample_kind.data.serialized_data;

        SerializedBufferHandle reader;
        init_serialized_buffer(&reader, topic_data, topic_size);
        void* user_topic = subscriber->topic->deserialize(&reader, callback_object);

        for(uint32_t i = 0; i < subscriber->listener_list_size; i++)
        {
            OnListenerTopic on_topic = subscriber->listener_list[i];
            on_topic(user_topic, callback_object);
        }
    }
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