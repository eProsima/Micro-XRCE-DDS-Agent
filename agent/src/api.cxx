#include "api.h"

#include <stdio.h>
#include <stdlib.h>

#include "fastcdr/Cdr.h"
#include "fastcdr/FastCdr.h"
#include "fastcdr/exceptions/Exception.h"

// Message Header
// 0                8                16              24               31
// +---------------+----------------+----------------+----------------+
// |                          clientKey                               |
// +---------------+--------+-------+----------------+----------------+
// |   sessionId   |    streamId    |           sequenceNr            |
// +---------------+--------+-------+----------------+----------------+


// Submessage header
// 0       4       8               16               24               31
// +-------+-------+----------------+----------------+----------------+
// | submessageId  |      flags     |       subMessageLength          |
// +-------+-------+----------------+----------------+----------------+

void initSerializedMessage(SerializedMessage* serialized_message, char* buffer, uint32_t buffer_size)
{
    serialized_message->buffer = buffer;
	serialized_message->fast_buffer = new eprosima::fastcdr::FastBuffer;
	serialized_message->fast_cdr = new eprosima::fastcdr::Cdr(*serialized_message->fast_buffer);
}

void resetSerializedMessage(SerializedMessage* serialized_message)
{

}

void freeSerializedMessage(SerializedMessage* serialized_message)
{

}

int serialize_messageHeader(SerializedMessage* serialized_message, MessageHeader* message_header)
{

    return 0;
}

int serialize_submessage(SerializedMessage* serialized_message, Submessage* submessage)
{
    return 0;
}

int deserialize_messageHeader(SerializedMessage* serialized_message, MessageHeader* message_header)
{

    return 0;
}

int deserialize_submessage(SerializedMessage* serialized_message, Submessage* submessage)
{
    return 0;
}