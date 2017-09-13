#include "fastcdr/Cdr.h"
#include "fastcdr/FastCdr.h"
#include "fastcdr/exceptions/Exception.h"

#define OBJECT_ID_INVALID 0xFFFFFF
#define OBJECT_ID_CLIENT 0xFFFFF0
#define OBJECT_ID_SESSION 0xFFFFF1

typedef uint32_t uint24_t;

class MessageHeader
{
    uint32_t client_key;
    uint8_t session_id;
    uint8_t stream_id;
    uint16_t sequence_nr;
};

class SubmessageHeader
{
    uint8_t submessage_id;
    uint8_t flags;
    uint16_t submessage_length;
};

class Submessage
{
    SubmessageHeader submessage_header;
    uint16_t request_id; //CREATE
    uint24_t object_id; //CREATE
    uint32_t xrce_cookie; //CREATE
    uint16_t xrce_version; //CREATE
    uint16_t xrce_vendor_id; //CREATE
    uint32_t client_timestamp; //CREATE
    uint8_t session_id; //CREATE
};

class SerializedMessage
{
public:
    char* buffer;
	eprosima::fastcdr::FastBuffer* fast_buffer;
	eprosima::fastcdr::Cdr* fast_cdr;
};

void initSerializedMessage(SerializedMessage* serialized_message, char* buffer, uint32_t buffer_size);
void resetSerializedMessage(SerializedMessage* serialized_message);
void freeSerializedMessage(SerializedMessage* serialized_message);

int serialize_messageHeader(SerializedMessage* serialized_message, MessageHeader* message_header);
int serialize_submessage(SerializedMessage* serialized_message, Submessage* submessage);

int deserialize_messageHeader(SerializedMessage* serialized_message, MessageHeader* message_header);
int deserialize_submessage(SerializedMessage* serialized_message, Submessage* submessage);
