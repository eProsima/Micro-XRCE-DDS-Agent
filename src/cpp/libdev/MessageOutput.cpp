// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "MessageOutput.h"

#include <stdio.h>

#include <MessageHeader.h>
#include <Payloads.h>
#include <SubMessageHeader.h>

namespace eprosima {
namespace micrortps {
namespace debug {

const std::string GREEN         = "\x1B[1;32m";
const std::string YELLOW        = "\x1B[1;33m";
const std::string RESTORE_COLOR = "\x1B[0m";

uint16_t platform_array_to_num(const std::array<uint8_t, 2>& array)
{
#if __BIG_ENDIAN__
    return array[0] + (array[1] << 8);
#else
    return array[1] + (array[0] << 8);
#endif
}

void print_message_header(const MessageHeader& header)
{
    printf("<Header> \n");
    printf("  - client_key: 0x%08X\n", clientkey_to_uint(header.client_key()));
    printf("  - session_id: 0x%02X\n", header.session_id());
    printf("  - stream_id: 0x%02X\n", header.stream_id());
    printf("  - sequence_number: %u\n", header.sequence_nr());
    printf("\n\n");
}

void print_submessage_header(const SubmessageHeader& header)
{
    switch (header.submessage_id())
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
    printf("  - request_id: 0x%08X\n", requestid_to_uint(payload.request_id()));
    printf("  - object_id: 0x%06X\n", objectid_to_uint(payload.object_id()));
    printf("  - kind: 0x%02X\n", (unsigned int) payload.object_representation()._d());

    std::string str_data;

    switch (payload.object_representation()._d())
    {
        case OBJECTKIND::PARTICIPANT:
            printf("    <Participant>\n");
            printf("    - representation: 0x%02X\n",
                   payload.object_representation().participant().representation()._d());
            switch (payload.object_representation().participant().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08lX\n",
                           payload.object_representation().participant().representation().object_name().size());
                    printf("    - string: %s\n",
                           payload.object_representation().participant().representation().object_name().c_str());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf(
                        "    - string_size: 0x%08lX\n",
                        payload.object_representation().participant().representation().string_representation().size());
                    printf("    - string: %s\n",
                           payload.object_representation().participant().representation().string_representation().c_str());
                    break;
            }
            break;
        case OBJECTKIND::DATAWRITER:
            printf("    <Data writer>\n");
            printf("    - representation: 0x%02X\n",
                   payload.object_representation().data_writer().representation()._d());
            switch (payload.object_representation().data_writer().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08lX\n",
                           payload.object_representation().data_writer().representation().object_reference().size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_writer().representation().object_reference().c_str());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf("    - string_size: 0x%08lX\n", payload.object_representation()
                                                              .data_writer()
                                                              .representation()
                                                              .xml_string_representation()
                                                              .size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_writer().representation().xml_string_representation().c_str());
                    break;
                case REPRESENTATION_IN_BINARY:
                    printf(
                        "    - binary_size: 0x%08lX\n",
                        payload.object_representation().data_writer().representation().binary_representation().size());
                    str_data = std::string(
                        payload.object_representation().data_writer().representation().binary_representation().begin(),
                        payload.object_representation().data_writer().representation().binary_representation().end());
                    printf("    - string: %s\n", str_data.c_str());
                    break;
            }
            printf("    - participan_id: 0x%06X\n", objectid_to_uint(payload.object_representation().data_writer().participant_id()));
            printf("    - publisher_id: 0x%06X\n", objectid_to_uint(payload.object_representation().data_writer().publisher_id()));
            break;

        case OBJECTKIND::DATAREADER:
            printf("    <Data reader>\n");
            printf("    - representation: 0x%02X\n",
                   payload.object_representation().data_reader().representation()._d());
            switch (payload.object_representation().data_reader().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08lX\n",
                           payload.object_representation().data_reader().representation().object_reference().size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_reader().representation().object_reference().c_str());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf("    - string_size: 0x%08lX\n", payload.object_representation()
                                                              .data_reader()
                                                              .representation()
                                                              .xml_string_representation()
                                                              .size());
                    printf("    - string: %s\n",
                           payload.object_representation().data_reader().representation().xml_string_representation().c_str());
                    break;
                case REPRESENTATION_IN_BINARY:
                    printf(
                        "    - binary_size: 0x%08lX\n",
                        payload.object_representation().data_reader().representation().binary_representation().size());
                    str_data = std::string(
                        payload.object_representation().data_reader().representation().binary_representation().begin(),
                        payload.object_representation().data_reader().representation().binary_representation().end());
                    printf("    - string: %s\n", str_data.c_str());
                    std::string str;
                    break;
            }
            printf("    - participan_id: 0x%06X\n", objectid_to_uint(payload.object_representation().data_reader().participant_id()));
            printf("    - subscriber_id: 0x%06X\n", objectid_to_uint(payload.object_representation().data_reader().subscriber_id()));
            break;

        case OBJECTKIND::SUBSCRIBER:
            printf("    <Data subscriber>\n");
            printf("    - representation: 0x%02X\n",
                   payload.object_representation().subscriber().representation()._d());
            switch (payload.object_representation().subscriber().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08lX\n",
                           payload.object_representation().subscriber().representation().object_reference().size());
                    printf("    - string: %s\n",
                           payload.object_representation().subscriber().representation().object_reference().c_str());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf("    - string_size: 0x%08lX\n", payload.object_representation()
                                                              .subscriber()
                                                              .representation()
                                                              .xml_string_representation()
                                                              .size());
                    printf("    - string: %s\n",
                           payload.object_representation().subscriber().representation().xml_string_representation().c_str());
                    break;
                case REPRESENTATION_IN_BINARY:
                    printf(
                        "    - binary_size: 0x%08lX\n",
                        payload.object_representation().subscriber().representation().binary_representation().size());
                    str_data = std::string(
                        payload.object_representation().subscriber().representation().binary_representation().begin(),
                        payload.object_representation().subscriber().representation().binary_representation().end());
                    printf("    - string: %s\n", str_data.c_str());
                    break;
            }
            printf("    - participan_id: 0x%06X\n", objectid_to_uint(payload.object_representation().subscriber().participant_id()));
            break;

        case OBJECTKIND::PUBLISHER:
            printf("    <Data publisher>\n");
            printf("    - representation: 0x%02X\n", payload.object_representation().publisher().representation()._d());
            switch (payload.object_representation().publisher().representation()._d())
            {
                case REPRESENTATION_BY_REFERENCE:
                    printf("    - string_size: 0x%08lX\n",
                           payload.object_representation().publisher().representation().object_reference().size());
                    printf("    - string: %s\n",
                           payload.object_representation().publisher().representation().object_reference().c_str());
                    break;
                case REPRESENTATION_AS_XML_STRING:
                    printf("    - string_size: 0x%08lX\n", payload.object_representation()
                                                              .publisher()
                                                              .representation()
                                                              .xml_string_representation()
                                                              .size());
                    printf("    - string: %s\n",
                           payload.object_representation().publisher().representation().xml_string_representation().c_str());
                    break;
                case REPRESENTATION_IN_BINARY:
                    printf("    - binary_size: 0x%08lX\n",
                           payload.object_representation().publisher().representation().binary_representation().size());
                    str_data = std::string(
                        payload.object_representation().publisher().representation().binary_representation().begin(),
                        payload.object_representation().publisher().representation().binary_representation().end());
                    printf("    - string: %s\n", str_data.c_str());
                    break;
            }

            printf("    - participan_id: 0x%06X\n", objectid_to_uint(payload.object_representation().publisher().participant_id()));
            break;
        default:
            break;
    }
    printf("\n\n");
}

void print_delete_submessage(const DELETE_RESOURCE_Payload& payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", requestid_to_uint(payload.request_id()));
    printf("  - object_id: 0x%06X\n", objectid_to_uint(payload.object_id()));
    printf("\n\n");
}

void print_status_submessage(const RESOURCE_STATUS_Payload& payload)
{
    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", requestid_to_uint(payload.request_id()));
    printf("  - status: 0x%02X\n", payload.result().status());
    printf("  - implementation: 0x%02X\n", payload.result().implementation_status());
    printf("  - object_id: 0x%06X\n", objectid_to_uint(payload.object_id()));

    printf("\n\n");
}

void print_write_data_submessage(const WRITE_DATA_Payload& payload)
{

    payload.request_id();
    payload.object_id();
    payload.data_to_write()._d();

    printf("  <Payload>\n");
    printf("  - request_id: 0x%08X\n", requestid_to_uint(payload.request_id()));
    printf("  - object_id: 0x%06X\n", objectid_to_uint(payload.object_id()));
    printf("  - data format: 0x%02X\n", payload.data_to_write()._d());

    std::string str_data;

    switch (payload.data_to_write()._d())
    {
        case FORMAT_DATA:
            printf("    <Data>\n");
            printf("    - serialized_data_size: 0x%08lX\n", payload.data_to_write().data().serialized_data().size());
            str_data = std::string(payload.data_to_write().data().serialized_data().begin(),
                                   payload.data_to_write().data().serialized_data().end());
            printf("    - serialized_data: %s\n", str_data.c_str());
            break;
        case FORMAT_DATA_SEQ:
            printf("    <Data>\n");
            printf("    - num datas: 0x%08lX\n", payload.data_to_write().data_seq().size());
            for (auto data : payload.data_to_write().data_seq())
            {
                printf("    - serialized_data_size: 0x%08lX\n", data.serialized_data().size());
                str_data = std::string(data.serialized_data().begin(), 
                                       data.serialized_data().end());
                //printf("    - serialized_data: %s\n", data.serialized_data());
                printf("    - serialized_data: %s\n", str_data.c_str());
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
    printf("  - request_id: 0x%08X\n", requestid_to_uint(payload.request_id()));
    printf("  - object_id: 0x%06X\n", objectid_to_uint(payload.object_id()));
    if (payload.read_specification().has_content_filter_expresion())
    {
        printf("  - content filter size: 0x%08lX\n", payload.read_specification().content_filter_expression().size());
        printf("  - content filter: %s\n", payload.read_specification().content_filter_expression().c_str());
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
    printf("  - request_id: 0x%08X\n", requestid_to_uint(payload.request_id()));
    printf("  - status: 0x%02X\n", payload.result().status());
    printf("  - implementation: 0x%02X\n", payload.result().implementation_status());
    printf("  - object_id: 0x%06X\n", objectid_to_uint(payload.object_id()));
    printf("    <Data>\n");
    printf("    - serialized_data_size: 0x%08lX\n", payload.data().serialized_data().size());
    std::string str_data(payload.data().serialized_data().begin(), 
                         payload.data().serialized_data().end());
    printf("    - serialized_data: %s\n", str_data.c_str());
    printf("\n\n");
}

void print_data_submessage(const DATA_Payload_Sample& payload)
{
    // TODO.
    (void) payload;
}

void print_data_submessage(const DATA_Payload_DataSeq& payload)
{
    // TODO.
    (void) payload;
}

void print_data_submessage(const DATA_Payload_SampleSeq& payload)
{
    // TODO.
    (void) payload;
}

void print_data_submessage(const DATA_Payload_PackedSamples& payload)
{
    // TODO.
    (void) payload;
}

void printl_create_client_submessage(const CREATE_CLIENT_Payload& payload)
{
    printf("%s[Create client | id: 0x%04X | session: 0x%02X]%s\n", YELLOW.data(),
           platform_array_to_num(payload.object_id()), payload.object_representation().session_id(),
           RESTORE_COLOR.data());
}

void printl_create_submessage(const CREATE_Payload& payload)
{
    char content[128];
    switch (payload.object_representation()._d())
    {
        case OBJECTKIND::PARTICIPANT:
            sprintf(content, "PARTICIPANT");
            break;
        case OBJECTKIND::DATAWRITER:
            sprintf(content, "DATA_WRITER | participant id: 0x%04X | publisher id: 0x%04X | xml: %lu",
                    platform_array_to_num(payload.object_representation().data_writer().participant_id()),
                    platform_array_to_num(payload.object_representation().data_writer().publisher_id()),
                    payload.object_representation().data_writer().representation().xml_string_representation().size());
            break;

        case OBJECTKIND::DATAREADER:
            sprintf(content, "DATA_READER | participant id: 0x%04X | subscriber id: 0x%04X | xml: %lu",
                    platform_array_to_num(payload.object_representation().data_reader().participant_id()),
                    platform_array_to_num(payload.object_representation().data_reader().subscriber_id()),
                    payload.object_representation().data_reader().representation().xml_string_representation().size());
            break;

        case OBJECTKIND::SUBSCRIBER:
            sprintf(content, "SUBSCRIBER | participant id: 0x%04X", platform_array_to_num(payload.object_representation().subscriber().participant_id()));
            break;

        case OBJECTKIND::PUBLISHER:
            sprintf(content, "PUBLISHER | participant id: 0x%04X", platform_array_to_num(payload.object_representation().publisher().participant_id()));
            break;
        case OBJECTKIND::TOPIC:
            sprintf(content, "TOPIC | participant id: 0x%04X", platform_array_to_num(payload.object_representation().topic().participant_id()));
            break;
        default:
            sprintf(content, "UNKNOWN");
    }
    printf("%s[Create | #0x%04X | id: 0x%04X | %s]%s\n", YELLOW.data(), platform_array_to_num(payload.request_id()),
           platform_array_to_num(payload.object_id()), content, RESTORE_COLOR.data());
}

void printl_delete_submessage(const DELETE_RESOURCE_Payload& payload)
{
    printf("%s[Delete | #0x%04X | id: 0x%04X]%s\n", YELLOW.data(), platform_array_to_num(payload.request_id()), platform_array_to_num(payload.object_id()),
           RESTORE_COLOR.data());
}

void printl_status_submessage(const RESOURCE_STATUS_Payload& payload)
{
    char kind[64];
    switch (payload.result().status())
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
    switch (payload.result().implementation_status())
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

    printf("%s[Status | #0x%04X | id: 0x%04X | %s | %s]%s\n",
            GREEN.data(),
                platform_array_to_num(payload.result().request_id()),
                platform_array_to_num(payload.object_id()),
                kind,
                implementation,
            RESTORE_COLOR.data());
}

void printl_write_data_submessage(const WRITE_DATA_Payload& payload)
{
    char content[1024];
    switch (payload.data_to_write()._d())
    {
        case FORMAT_DATA:
            sprintf(content, "DATA | data size: %lu", payload.data_to_write().data().serialized_data().size());
            break;
        case FORMAT_DATA_SEQ:
        {
            int seq_counter = 0;
            for (auto data : payload.data_to_write().data_seq())
            {
                sprintf(content, "DATA | sq_nr: %u | data size: %lu", seq_counter++, data.serialized_data().size());
            }
            break;
        }
        case FORMAT_SAMPLE:
            break;
        case FORMAT_SAMPLE_SEQ:
            break;
        case FORMAT_PACKED_SAMPLES:
            break;
    }
    printf("%s[Write data | #0x%04X | id: 0x%04X | %s]%s\n",
            YELLOW.data(),
            platform_array_to_num(payload.request_id()),
            platform_array_to_num(payload.object_id()),
            content,
            RESTORE_COLOR.data());
}

void printl_read_data_submessage(const READ_DATA_Payload& payload)
{
    eprosima::micrortps::DataDeliveryControl read_control;
    switch (payload.read_specification().delivery_config()._d())
    {
        case FORMAT_DATA:
        case FORMAT_SAMPLE:
            read_control.max_elapsed_time(0);
            read_control.max_rate(0);
            read_control.max_samples(1);
            break;
        case FORMAT_DATA_SEQ:
        case FORMAT_SAMPLE_SEQ:
        case FORMAT_PACKED_SAMPLES:
            read_control.max_elapsed_time(payload.read_specification().delivery_config().delivery_control().max_elapsed_time());
            read_control.max_rate(payload.read_specification().delivery_config().delivery_control().max_rate());
            read_control.max_samples(payload.read_specification().delivery_config().delivery_control().max_samples());
            break;
        default:
            break;
    }
    printf("%s[Read data | #0x%04X | id: 0x%04X | Read Conf: max_time %u max_rate %u max_samples %u]%s\n",
        YELLOW.data(),
        platform_array_to_num(payload.request_id()),
        platform_array_to_num(payload.object_id()),
        read_control.max_elapsed_time(),
        read_control.max_rate(),
        read_control.max_samples(),
        RESTORE_COLOR.data());
}

void printl_data_submessage(const DATA_Payload_Data& payload)
{
    char content[64];
    sprintf(content, "DATA | data size: %lu", payload.data().serialized_data().size());
    printf("%s[Data | #0x%04X | id: 0x%04X | %s]%s\n",
        GREEN.data(),
        platform_array_to_num(payload.request_id()),
        platform_array_to_num(payload.object_id()),
        content,
        RESTORE_COLOR.data());
}

void printl_data_submessage(const DATA_Payload_DataSeq& payload)
{
    char content[1024];
    int seq_counter = 0;
    for (auto data : payload.data_seq())
    {
        sprintf(content, "DATA | sq_nr: %u | data size: %lu", seq_counter++, data.serialized_data().size());
    }
    printf("%s[Data | #0x%04X | id: 0x%04X | %s]%s\n",
        GREEN.data(),
        platform_array_to_num(payload.request_id()),
        platform_array_to_num(payload.object_id()),
        content,
        RESTORE_COLOR.data()
    );
}

unsigned int clientkey_to_uint(const ClientKey& key)
{
    return key[0] + (key[1] << 8) + (key[2] << 16) + (key[3] << 24);
}

unsigned int requestid_to_uint(const RequestId& id)
{
    return id[0] + (id[1] << 8);
}

unsigned int objectid_to_uint(const ObjectId& id)
{
    return id[0] + (id[1] << 8);
}

} // namespace debug
} // namespace micrortps
} // namespace eprosima
