#ifndef _MESSAGE_OUTPUT_H_
#define _MESSAGE_OUTPUT_H_

#include <cstdint>

namespace eprosima {
namespace micrortps {

class MessageHeader;
class SubmessageHeader;
class CREATE_Payload;
class DELETE_RESOURCE_Payload;
class RESOURCE_STATUS_Payload;
class WRITE_DATA_Payload;
class READ_DATA_Payload;
class DATA_Payload_Data;
class DATA_Payload_Sample;
class DATA_Payload_DataSeq;
class DATA_Payload_SampleSeq;
class DATA_Payload_PackedSamples;

namespace debug {

void print_message_header(const MessageHeader& header);
void print_submessage_header(const SubmessageHeader& header);

void print_create_submessage(const CREATE_Payload& payload);
void print_delete_submessage(const DELETE_RESOURCE_Payload& payload);
void print_status_submessage(const RESOURCE_STATUS_Payload& payload);
void print_write_data_submessage(const WRITE_DATA_Payload& payload);
void print_read_data_submessage(const READ_DATA_Payload& payload);
void print_data_submessage(const DATA_Payload_Data& payload);
void print_data_submessage(const DATA_Payload_Sample& payload);
void print_data_submessage(const DATA_Payload_DataSeq& payload);
void print_data_submessage(const DATA_Payload_SampleSeq& payload);
void print_data_submessage(const DATA_Payload_PackedSamples& payload);

void printl_create_submessage(const CREATE_Payload& payload);
void printl_delete_submessage(const DELETE_RESOURCE_Payload& payload);
void printl_status_submessage(const RESOURCE_STATUS_Payload& payload);
void printl_write_data_submessage(const WRITE_DATA_Payload& payload);
void printl_read_data_submessage(const READ_DATA_Payload& payload);
void printl_data_submessage(const DATA_Payload_Data& payload);
void printl_data_submessage(const DATA_Payload_Sample& payload);
void printl_data_submessage(const DATA_Payload_DataSeq& payload);
void printl_data_submessage(const DATA_Payload_SampleSeq& payload);
void printl_data_submessage(const DATA_Payload_PackedSamples& payload);

// UTIL
const char* data_to_string(const uint8_t* data, uint32_t size);

} // namespace debug
} // namespace micrortps
} // namespace eprosima

#endif //_MESSAGE_OUTPUT_H_
