#ifndef _DDS_XRCE_CLIENT_H_
#define _DDS_XRCE_CLIENT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define OBJECT_STATUS_UNKNOWN        0
#define OBJECT_STATUS_AVALIABLE      1
#define OBJECT_STATUS_UNAVALIABLE    2

#define OBJECT_PARTICIPANT   0x01
#define OBJECT_DATA_WRITER   0x03
#define OBJECT_DATA_READER   0x07
#define OBJECT_SUBSCRIBER    0x08
#define OBJECT_PUBLISHER     0x09

typedef void (*DataOutEvent)(uint8_t* buffer, uint32_t size, void* callback_object);
typedef uint32_t (*DataInEvent)(uint8_t* buffer, uint32_t length, void* callback_object);

typedef struct Topic Topic;
typedef void (*OnListenerTopic)(const void* topic_data, void* callback_object);


typedef struct XRCEObject
{
    uint32_t id;
    uint8_t status;
    uint8_t last_operation;

} XRCEObject;

typedef struct Participant Participant;

typedef struct Publisher
{
    XRCEObject object;
    Participant* participant;
    Topic* topic;

} Publisher;

typedef struct Subscriber
{
    XRCEObject object;
    Participant* participant;
    OnListenerTopic listener_list[10]; //make dynamically
    uint32_t listener_list_size;
    Topic* topic;

    uint32_t remaning_messages;

} Subscriber;

struct Participant
{
    XRCEObject object;
    Publisher* publisher_list[10]; //make dynamically
    uint32_t publisher_list_size;

    Subscriber* subscriber_list[10]; //make dynamically
    uint32_t subscriber_list_size;
};


//DDS XRCE API
void init_client(uint32_t buffer_size, DataOutEvent send_data_io, DataInEvent receive_data_io,
        void* data_io, void* callback_object);
void destroy_client();

Publisher* create_publisher(Topic* topic);
Subscriber* create_subscriber(Topic* topic);

int send_topic(Publisher* publisher, void* topic_data);

int read_data(Subscriber* subscriber, uint16_t max_messages);

void add_listener_topic(Subscriber* subscriber, OnListenerTopic on_listener_topic);

void delete_publisher(Publisher* publisher);
void delete_subscriber(Subscriber* subscriber);

void update_communication();

//UTIL
uint32_t get_xrce_object(uint32_t id, void** object);

#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_CLIENT_H_

