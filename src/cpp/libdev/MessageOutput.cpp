#include "MessageOutput.h"

#include <stdio.h>

#include <MessageHeader.h>
#include <Payloads.h>
#include <SubMessageHeader.h>

namespace eprosima {
namespace micrortps {
namespace debug {

const std::string GREEN         = "\e[1;32m";
const std::string YELLOW        = "\e[1;33m";
const std::string RESTORE_COLOR = "\e[0m";

void print_message_header(const MessageHeader& header)
{
    printf("<Header> \n");
    printf("  - client_key: 0x%08X\n", header.client_key());
    printf("  - session_id: 0x%02X\n", header.session_id());
    printf("  - stream_id: 0x%02X\n", header.stream_id());
    printf("  - sequence_number: %u\n", header.sequence_nr());
    printf("\n\n");
}

void print_submessage_header(const SubmessageHeader& header)
{
    switch(header.submessage_id())
    {
        case CREATE_CLIENT:
            printf("<Submessage> [CREATE_CLIENT] \n");
            break;
        case CREATE:
            printf("<Submessage> [CREATE] \n");
            break;
        case DELETE:
            printf("<Submessage> [DELETE]\n");
            break;
        case STATUS:
            printf("<Submessage> [STATUS]\n");
            break;
        case WRITE_DATA:
            printf("<Submessage> [WRITE_DATA]\n");
            break;
        case READ_DATA:
            printf("<Submessage> [READ_DATA]\n");
            break;
        case DATA:
            printf("<Submessage> [DATA]\n");
            break;
        default:
            printf("<Submessage> [*UNKNOWN*]\n");
    }

    printf("  <Submessage header> \n");
    printf("  - id: 0x%02X\n", header.submessage_id());
    printf("  - flags: 0x%02X\n", header.flags());
    printf("  - length: %u\n", header.submessage_length());
    printf("\n");
}

void print_create_submessage(const CREATE_Payload& payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload.request_id());
    printf("  - object_id: 0x%06X\n", payload.object_id());
    printf("  - kind: 0x%02X\n", payload.object_representation()._d());

    switch(payload.object_representation()._d())
    {
        case OBJK_PARTICIPANT:
            printf("    <Participant>\n");
            printf("    - representation: 0x%02X\n",
                   payload.object_representation().participant().representation()._d());
            switch(payload.object_representation().participant().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08X\n",
                           payload.object_representation().participant().representation().object_name().size());
                    printf("    - string: %s\n",
                           payload.object_representation().participant().representation().object_name());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf(
                        "    - string_size: 0x%08X\n",
                        payload.object_representation().participant().representation().string_representation().size());
                    printf("    - string: %s\n",
                           payload.object_representation().participant().representation().string_representation());
                    break;
            }
            break;
        case OBJK_DATAWRITER:
            printf("    <Data writer>\n");
            printf("    - representation: 0x%02X\n",
                   payload.object_representation().data_writer().representation()._d());
            switch(payload.object_representation().data_writer().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08X\n",
                           payload.object_representation().data_writer().representation().object_reference().size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_writer().representation().object_reference());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf("    - string_size: 0x%08X\n", payload.object_representation()
                                                              .data_writer()
                                                              .representation()
                                                              .xml_string_representation()
                                                              .size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_writer().representation().xml_string_representation());
                    break;
                case REPRESENTATION_IN_BINARY:
                    printf(
                        "    - binary_size: 0x%08X\n",
                        payload.object_representation().data_writer().representation().binary_representation().size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_writer().representation().binary_representation());
                    break;
            }
            printf("    - participan_id: 0x%06X\n", payload.object_representation().data_writer().participant_id());
            printf("    - publisher_id: 0x%06X\n", payload.object_representation().data_writer().publisher_id());
            break;

        case OBJK_DATAREADER:
            printf("    <Data reader>\n");
            printf("    - representation: 0x%02X\n",
                   payload.object_representation().data_reader().representation()._d());
            switch(payload.object_representation().data_reader().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08X\n",
                           payload.object_representation().data_reader().representation().object_reference().size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_reader().representation().object_reference());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf("    - string_size: 0x%08X\n", payload.object_representation()
                                                              .data_reader()
                                                              .representation()
                                                              .xml_string_representation()
                                                              .size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_reader().representation().xml_string_representation());
                    break;
                case REPRESENTATION_IN_BINARY:
                    printf(
                        "    - binary_size: 0x%08X\n",
                        payload.object_representation().data_reader().representation().binary_representation().size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_reader().representation().binary_representation());
                    break;
            }
            printf("    - participan_id: 0x%06X\n", payload.object_representation().data_reader().participant_id());
            printf("    - subscriber_id: 0x%06X\n", payload.object_representation().data_reader().subscriber_id());
            break;

        case OBJK_SUBSCRIBER:
            printf("    <Data subscriber>\n");
            printf("    - representation: 0x%02X\n",
                   payload.object_representation().subscriber().representation()._d());
            switch(payload.object_representation().subscriber().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08X\n",
                           payload.object_representation().subscriber().representation().object_reference().size());
                    printf("    - string: %s\n",
                           payload.object_representation().subscriber().representation().object_reference());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf("    - string_size: 0x%08X\n", payload.object_representation()
                                                              .subscriber()
                                                              .representation()
                                                              .xml_string_representation()
                                                              .size());
                    printf("    - string: %s\n",
                           payload.object_representation().subscriber().representation().xml_string_representation());
                    break;
                case REPRESENTATION_IN_BINARY:
                    printf(
                        "    - binary_size: 0x%08X\n",
                        payload.object_representation().subscriber().representation().binary_representation().size());
                    printf("    - string: %s\n",
                           payload.object_representation().subscriber().representation().binary_representation());
                    break;
            }
            printf("    - participan_id: 0x%06X\n", payload.object_representation().subscriber().participant_id());
            break;

        case OBJK_PUBLISHER:
            printf("    <Data publisher>\n");
            printf("    - representation: 0x%02X\n", payload.object_representation().publisher().representation()._d());
            switch(payload.object_representation().publisher().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08X\n",
                           payload.object_representation().publisher().representation().object_reference().size());
                    printf("    - string: %s\n",
                           payload.object_representation().publisher().representation().object_reference());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf("    - string_size: 0x%08X\n", payload.object_representation()
                                                              .publisher()
                                                              .representation()
                                                              .xml_string_representation()
                                                              .size());
                    printf("    - string: %s\n",
                           payload.object_representation().publisher().representation().xml_string_representation());
                    break;
                case REPRESENTATION_IN_BINARY:
                    printf("    - binary_size: 0x%08X\n",
                           payload.object_representation().publisher().representation().binary_representation().size());
                    printf("    - string: %s\n",
                           payload.object_representation().publisher().representation().binary_representation());
                    break;
            }

            printf("    - participan_id: 0x%06X\n", payload.object_representation().publisher().participant_id());
            break;
    }
    printf("\n\n");
}

void print_delete_submessage(const DELETE_RESOURCE_Payload& payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload.request_id());
    printf("  - object_id: 0x%06X\n", payload.object_id());
    printf("\n\n");
}

void print_status_submessage(const RESOURCE_STATUS_Payload& payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload.request_id());
    printf("  - status: 0x%02X\n", payload.result().status());
    printf("  - implementation: 0x%02X\n", payload.result().implementation_status());
    printf("  - object_id: 0x%06X\n", payload.object_id());

    printf("\n\n");
}

void print_write_data_submessage(const WRITE_DATA_Payload& payload)
{

    payload.request_id();
    payload.object_id();
    payload.data_to_write()._d();

    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload.request_id());
    printf("  - object_id: 0x%06X\n", payload.object_id());
    printf("  - data format: 0x%02X\n", payload.data_to_write()._d());

    switch(payload.data_to_write()._d())
    {
        case FORMAT_DATA:
            printf("    <Data>\n");
            printf("    - serialized_data_size: 0x%08X\n", payload.data_to_write().data().serialized_data().size());
            printf("    - serialized_data: %s\n", payload.data_to_write().data().serialized_data());
            break;
        case FORMAT_DATA_SEQ:
            printf("    <Data>\n");
            printf("    - num datas: 0x%08X\n", payload.data_to_write().data_seq().size());
            for(auto data : payload.data_to_write().data_seq())
            {
                printf("    - serialized_data_size: 0x%08X\n", data.serialized_data().size());
                printf("    - serialized_data: %s\n", data.serialized_data());
            }
            break;
        case FORMAT_SAMPLE:
            break;
        case FORMAT_SAMPLE_SEQ:
            break;
        case FORMAT_PACKED_SAMPLES:
            break;
    }
    printf("\n\n");
}

void print_read_data_submessage(const READ_DATA_Payload& payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload.request_id());
    printf("  - object_id: 0x%06X\n", payload.object_id());
    if(payload.read_specification().has_content_filter_expresion())
    {
        printf("  - content filter size: 0x%08X\n", payload.read_specification().content_filter_expression().size());
        printf("  - content filter: 0x%06X\n", payload.read_specification().content_filter_expression());
    }
    printf("  - data format: 0x%02X\n", payload.read_specification().delivery_config()._d());
    printf("  - max_elapsed_time: %u\n",
           payload.read_specification().delivery_config().delivery_control().max_elapsed_time());
    printf("  - max_rate: %u\n", payload.read_specification().delivery_config().delivery_control().max_rate());
    printf("  - max_samples: %hu\n", payload.read_specification().delivery_config().delivery_control().max_samples());
    printf("\n\n");
}

void print_data_submessage(const DATA_Payload_Data& payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", payload.request_id());
    printf("  - status: 0x%02X\n", payload.result().status());
    printf("  - implementation: 0x%02X\n", payload.result().implementation_status());
    printf("  - object_id: 0x%06X\n", payload.object_id());
    printf("    <Data>\n");
    printf("    - serialized_data_size: 0x%08X\n", payload.data().serialized_data().size());
    printf("    - serialized_data: %s\n", payload.data().serialized_data());
    printf("\n\n");
}

void print_data_submessage(const DATA_Payload_Sample& payload)
{
}

void print_data_submessage(const DATA_Payload_DataSeq& payload)
{
}

void print_data_submessage(const DATA_Payload_SampleSeq& payload)
{
}

void print_data_submessage(const DATA_Payload_PackedSamples& payload)
{
}

void printl_create_submessage(const CREATE_Payload& payload)
{
    char content[128];
    switch(payload.object_representation()._d())
    {
        case OBJK_PARTICIPANT:
            sprintf(content, "PARTICIPANT");
            break;

        case OBJK_CLIENT:
            sprintf(content, "CLIENT | cookie: 0x%08X | v%u | vendor: %u | session: %u",
                    payload.object_representation().client().xrce_cookie(),
                    payload.object_representation().client().xrce_version(),
                    payload.object_representation().client().xrce_vendor_id(),
                    payload.object_representation().client().session_id());
            break;
        case OBJK_DATAWRITER:
            sprintf(content, "DATA_WRITER | id: %u | id: %u | topic: %s",
                    payload.object_representation().data_writer().participant_id(),
                    payload.object_representation().data_writer().publisher_id(),
                    payload.object_representation().data_writer().representation().object_reference());
            break;

        case OBJK_DATAREADER:
            sprintf(content, "DATA_READER | id: %u | id: %u | topic: %s",
                    payload.object_representation().data_writer().participant_id(),
                    payload.object_representation().data_reader().subscriber_id(),
                    payload.object_representation().data_reader().representation().object_reference());
            break;

        case OBJK_SUBSCRIBER:
            sprintf(content, "SUBSCRIBER | id: %u", payload.object_representation().subscriber().participant_id());
            break;

        case OBJK_PUBLISHER:
            sprintf(content, "PUBLISHER | id: %u", payload.object_representation().publisher().participant_id());
            break;
        default:
            sprintf(content, "UNKNOWN");
    }
    printf("%s[Create | #%08X | id: %u | %s]%s\n", YELLOW, payload.request_id(), payload.object_id(), content,
           RESTORE_COLOR);
}

void printl_delete_submessage(const DELETE_RESOURCE_Payload& payload)
{
    printf("%s[Delete | #%08X | id: %u]%s\n", YELLOW, payload.request_id(), payload.object_id(), RESTORE_COLOR);
}

void printl_status_submessage(const RESOURCE_STATUS_Payload& payload)
{
    char kind[64];
    switch(payload.result().status())
    {
        case STATUS_LAST_OP_NONE:
            sprintf(kind, "NONE");
            break;
        case STATUS_LAST_OP_CREATE:
            sprintf(kind, "CREATE");
            break;
        case STATUS_LAST_OP_UPDATE:
            sprintf(kind, "UPDATE");
            break;
        case STATUS_LAST_OP_DELETE:
            sprintf(kind, "DELETE");
            break;
        case STATUS_LAST_OP_LOOKUP:
            sprintf(kind, "LOOKUP");
            break;
        case STATUS_LAST_OP_READ:
            sprintf(kind, "READ");
            break;
        case STATUS_LAST_OP_WRITE:
            sprintf(kind, "WRITE");
            break;
        default:
            sprintf(kind, "UNKNOWN");
    }

    char implementation[64];
    switch(payload.result().implementation_status())
    {
        case STATUS_OK:
            sprintf(implementation, "OK");
            break;
        case STATUS_OK_MATCHED:
            sprintf(implementation, "OK_MATCHED");
            break;
        case STATUS_ERR_DDS_ERROR:
            sprintf(implementation, "ERR_DDS_ERROR");
            break;
        case STATUS_ERR_MISMATCH:
            sprintf(implementation, "ERR_MISMATCH");
            break;
        case STATUS_ERR_ALREADY_EXISTS:
            sprintf(implementation, "ERR_ALREADY_EXISTS");
            break;
        case STATUS_ERR_DENIED:
            sprintf(implementation, "ERR_DENIED");
            break;
        case STATUS_ERR_UNKNOWN_REFERENCE:
            sprintf(implementation, "ERR_UNKNOWN_REFERENCE");
            break;
        case STATUS_ERR_INVALID_DATA:
            sprintf(implementation, "ERR_INVALID_DATA");
            break;
        case STATUS_ERR_INCOMPATIBLE:
            sprintf(implementation, "ERR_INCOMPATIBLE");
            break;
        case STATUS_ERR_RESOURCES:
            sprintf(implementation, "ERR_RESOURCES");
            break;
        default:
            sprintf(implementation, "UNKNOWN");
    }

    printf("%s[Status | #%08X | id: %u | %s | %s]%s\n", GREEN, payload.result().request_id(), payload.object_id(), kind,
           implementation, RESTORE_COLOR);
}

void printl_write_data_submessage(const WRITE_DATA_Payload& payload)
{
    payload.request_id();
    payload.object_id();
    payload.data_to_write()._d();

    char content[1024];
    switch(payload.data_to_write()._d())
    {
        case FORMAT_DATA:
            sprintf(content, "DATA | data size: %u", payload.data_to_write().data().serialized_data().size());
            break;
        case FORMAT_DATA_SEQ:
            int seq_counter = 0;
            for(auto data : payload.data_to_write().data_seq())
            {
                sprintf(content, "DATA | sq_nr: %u | data size: %u", seq_counter++, data.serialized_data().size());
            }
            break;
        case FORMAT_SAMPLE:
            break;
        case FORMAT_SAMPLE_SEQ:
            break;
        case FORMAT_PACKED_SAMPLES:
            break;
    }
    printf("%s[Write data | #%08X | id: %u | %s]%s\n", YELLOW, payload.request_id(), payload.object_id(), content,
           RESTORE_COLOR);
}

void printl_read_data_submessage(const READ_DATA_Payload& payload)
{
    printf("%s[Read data | #%08X | id: %u | max samples: %u]%s\n", YELLOW, payload.request_id(), payload.object_id(),
           payload.read_specification().delivery_config().delivery_control().max_samples(), RESTORE_COLOR);
}

void printl_data_submessage(const DATA_Payload_Data& payload)
{
    char content[64];
    sprintf(content, "DATA | data size: %u", payload.data().serialized_data().size());
    printf("%s[Data | #%08X | id: %u | %s]%s\n", GREEN, payload.request_id(), payload.object_id(), content,
           RESTORE_COLOR);
}

void printl_data_submessage(const DATA_Payload_DataSeq& payload)
{
    char content[1024];
    int seq_counter = 0;
    for(auto data : payload.data_seq())
    {
        sprintf(content, "DATA | sq_nr: %u | data size: %u", seq_counter++, data.serialized_data().size());
    }
    printf("%s[Data | #%08X | id: %u | %s]%s\n", GREEN, payload.request_id(), payload.object_id(), content,
           RESTORE_COLOR);
}

} // namespace debug
} // namespace micrortps
} // namespace eprosima