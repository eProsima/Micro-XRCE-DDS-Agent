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

#include "../Common.h"

#include <uxr/agent/message/InputMessage.hpp>
#include <uxr/agent/message/OutputMessage.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdio.h>

#define BUFFER_LENGTH 200

namespace eprosima {
namespace uxr {
namespace testing {

bool operator==(const dds::xrce::MessageHeader& m1, const dds::xrce::MessageHeader& m2)
{
    bool returned_value = true;

    returned_value &= m1.client_key() == m2.client_key();
    returned_value &= m1.sequence_nr() == m2.sequence_nr();
    returned_value &= m1.session_id() == m2.session_id();
    returned_value &= m1.stream_id() == m2.stream_id();

    return returned_value;
}

class SerializerDeserializerTests : public CommonData, public ::testing::Test
{
  protected:
    SerializerDeserializerTests()
    {
    }

    virtual ~SerializerDeserializerTests()
    {
    }
};

TEST_F(SerializerDeserializerTests, MessageHeader)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    OutputMessage output(message_header, message_header.getCdrSerializedSize());

    InputMessage input(output.get_buf(), output.get_len());
    const dds::xrce::MessageHeader& deserialized_message_header = input.get_header();
    ASSERT_TRUE(message_header == deserialized_message_header);
}

TEST_F(SerializerDeserializerTests, SubmessageHeader)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    dds::xrce::CREATE_Payload create_payload;
    dds::xrce::SubmessageHeader submessage_header;
    size_t message_size = message_header.getCdrSerializedSize() +
                          submessage_header.getCdrSerializedSize() +
                          create_payload.getCdrSerializedSize();

    OutputMessage output(message_header, message_size);
    output.append_submessage(dds::xrce::CREATE, create_payload, 0x0001);

    InputMessage input(output.get_buf(), output.get_len());
    ASSERT_TRUE(input.prepare_next_submessage());
    const dds::xrce::SubmessageHeader& deserialized_submessage_header = input.get_subheader();

    ASSERT_EQ(dds::xrce::CREATE, deserialized_submessage_header.submessage_id());
    ASSERT_EQ(0x0001, deserialized_submessage_header.flags());
    ASSERT_EQ(create_payload.getCdrSerializedSize(), deserialized_submessage_header.submessage_length());
}

TEST_F(SerializerDeserializerTests, CreateSubMessage)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    dds::xrce::CREATE_Payload create_payload = generate_create_payload(dds::xrce::OBJK_PUBLISHER);
    dds::xrce::SubmessageHeader submessage_header;
    size_t message_size = message_header.getCdrSerializedSize() +
                          submessage_header.getCdrSerializedSize() +
                          create_payload.getCdrSerializedSize();

    OutputMessage output(message_header, message_size);
    output.append_submessage(dds::xrce::CREATE, create_payload);

    dds::xrce::CREATE_Payload deserialized_create;
    InputMessage input(output.get_buf(), output.get_len());
    ASSERT_TRUE(input.prepare_next_submessage());
    ASSERT_TRUE(input.get_payload(deserialized_create));

    ASSERT_EQ(create_payload.request_id(), deserialized_create.request_id());
    ASSERT_EQ(create_payload.object_id(), deserialized_create.object_id());
    ASSERT_EQ(create_payload.object_representation().publisher().participant_id(),
              deserialized_create.object_representation().publisher().participant_id());
}

TEST_F(SerializerDeserializerTests, ResourceStatusSubmessage)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    dds::xrce::STATUS_Payload resource_status = generate_resource_status_payload(dds::xrce::STATUS_OK, 0x00);
    dds::xrce::SubmessageHeader submessage_header;
    size_t message_size = message_header.getCdrSerializedSize() +
                          submessage_header.getCdrSerializedSize() +
                          resource_status.getCdrSerializedSize();

    OutputMessage output(message_header, message_size);
    output.append_submessage(dds::xrce::STATUS, resource_status);

    dds::xrce::STATUS_Payload deserialized_status;
    InputMessage input(output.get_buf(), output.get_len());
    ASSERT_TRUE(input.prepare_next_submessage());
    ASSERT_TRUE(input.get_payload(deserialized_status));

    ASSERT_EQ(resource_status.related_request().object_id(), deserialized_status.related_request().object_id());
    ASSERT_EQ(resource_status.related_request().request_id(), deserialized_status.related_request().request_id());
    ASSERT_EQ(resource_status.result().status(), deserialized_status.result().status());
    ASSERT_EQ(resource_status.result().implementation_status(), deserialized_status.result().implementation_status());
}

TEST_F(SerializerDeserializerTests, ReadDataSubmessageNoFilter)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    dds::xrce::READ_DATA_Payload read_data = generate_read_data_payload(Optional<std::string>(),
                                                                        dds::xrce::FORMAT_DATA_SEQ);
    dds::xrce::SubmessageHeader submessage_header;
    size_t message_size = message_header.getCdrSerializedSize() +
                          submessage_header.getCdrSerializedSize() +
                          read_data.getCdrSerializedSize();

    OutputMessage output(message_header, message_size);
    output.append_submessage(dds::xrce::READ_DATA, read_data);

    dds::xrce::READ_DATA_Payload deserialized_read_data;
    InputMessage input(output.get_buf(), output.get_len());
    ASSERT_TRUE(input.prepare_next_submessage());
    ASSERT_TRUE(input.get_payload(deserialized_read_data));

    ASSERT_THROW(read_data.read_specification().content_filter_expression(),
                 dds::xrce::XRCETypesException);
    ASSERT_EQ(read_data.object_id(), deserialized_read_data.object_id());
    ASSERT_EQ(read_data.request_id(), deserialized_read_data.request_id());
    ASSERT_EQ(read_data.read_specification().data_format(),
              deserialized_read_data.read_specification().data_format());
    ASSERT_EQ(read_data.read_specification().delivery_control().max_samples(),
              deserialized_read_data.read_specification().delivery_control().max_samples());
    ASSERT_EQ(read_data.read_specification().delivery_control().max_elapsed_time(),
              deserialized_read_data.read_specification().delivery_control().max_elapsed_time());
    ASSERT_EQ(read_data.read_specification().delivery_control().max_bytes_per_second(),
              deserialized_read_data.read_specification().delivery_control().max_bytes_per_second());
}

TEST_F(SerializerDeserializerTests, ReadDataSubmessageFilter)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    const Optional<std::string> test_filter = std::string("TEST");
    dds::xrce::READ_DATA_Payload read_data = generate_read_data_payload(test_filter, dds::xrce::FORMAT_DATA_SEQ);
    dds::xrce::SubmessageHeader submessage_header;
    size_t message_size = message_header.getCdrSerializedSize() +
                          submessage_header.getCdrSerializedSize() +
                          read_data.getCdrSerializedSize();

    OutputMessage output(message_header, message_size);
    output.append_submessage(dds::xrce::READ_DATA, read_data);

    dds::xrce::READ_DATA_Payload deserialized_read_data;
    InputMessage input(output.get_buf(), output.get_len());
    ASSERT_TRUE(input.prepare_next_submessage());
    ASSERT_TRUE(input.get_payload(deserialized_read_data));

    ASSERT_EQ(read_data.object_id(), deserialized_read_data.object_id());
    ASSERT_EQ(read_data.request_id(), deserialized_read_data.request_id());
    ASSERT_EQ(read_data.read_specification().content_filter_expression(),
              deserialized_read_data.read_specification().content_filter_expression());
    ASSERT_EQ(read_data.read_specification().data_format(),
              deserialized_read_data.read_specification().data_format());
    ASSERT_EQ(read_data.read_specification().delivery_control().max_samples(),
              deserialized_read_data.read_specification().delivery_control().max_samples());
    ASSERT_EQ(read_data.read_specification().delivery_control().max_elapsed_time(),
              deserialized_read_data.read_specification().delivery_control().max_elapsed_time());
    ASSERT_EQ(read_data.read_specification().delivery_control().max_bytes_per_second(),
              deserialized_read_data.read_specification().delivery_control().max_bytes_per_second());
}

TEST_F(SerializerDeserializerTests, WriteDataSubmessage)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    dds::xrce::WRITE_DATA_Payload_Data write_payload = generate_write_data_payload();
    dds::xrce::SubmessageHeader submessage_header;
    size_t message_size = message_header.getCdrSerializedSize() +
                          submessage_header.getCdrSerializedSize() +
                          write_payload.getCdrSerializedSize();

    OutputMessage output(message_header, message_size);
    output.append_submessage(dds::xrce::WRITE_DATA, write_payload);

    dds::xrce::WRITE_DATA_Payload_Data deserialized_write_data;
    deserialized_write_data.data().resize(write_payload.data().getCdrSerializedSize(0));
    InputMessage input(output.get_buf(), output.get_len());
    ASSERT_TRUE(input.prepare_next_submessage());
    ASSERT_TRUE(input.get_payload(deserialized_write_data));

    ASSERT_EQ(write_payload.request_id(), deserialized_write_data.request_id());
    ASSERT_EQ(write_payload.object_id(), deserialized_write_data.object_id());
    ASSERT_EQ(write_payload.data().serialized_data(),
              deserialized_write_data.data().serialized_data());
}

TEST_F(SerializerDeserializerTests, DataSubmessage)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    dds::xrce::DATA_Payload_Data data_payload = generate_data_payload_data();
    dds::xrce::SubmessageHeader submessage_header;
    size_t message_size = message_header.getCdrSerializedSize() +
                          submessage_header.getCdrSerializedSize() +
                          data_payload.getCdrSerializedSize();

    OutputMessage output(message_header, message_size);
    output.append_submessage(dds::xrce::DATA, data_payload);

    dds::xrce::DATA_Payload_Data deserialized_data;
    deserialized_data.data().resize(data_payload.data().getCdrSerializedSize(0));
    InputMessage input(output.get_buf(), output.get_len());
    ASSERT_TRUE(input.prepare_next_submessage());
    ASSERT_TRUE(input.get_payload(deserialized_data));

    ASSERT_EQ(data_payload.object_id(), deserialized_data.object_id());
    ASSERT_EQ(data_payload.request_id(), deserialized_data.request_id());
    ASSERT_EQ(data_payload.data().serialized_data(), deserialized_data.data().serialized_data());
}

TEST_F(SerializerDeserializerTests, DeleteSubmessage)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    dds::xrce::DELETE_Payload delete_payload = generate_delete_resource_payload(object_id);
    dds::xrce::SubmessageHeader submessage_header;
    size_t message_size = message_header.getCdrSerializedSize() +
                          submessage_header.getCdrSerializedSize() +
                          delete_payload.getCdrSerializedSize();

    OutputMessage output(message_header, message_size);
    output.append_submessage(dds::xrce::DELETE_ID, delete_payload);

    dds::xrce::DELETE_Payload deserialized_data;
    InputMessage input(output.get_buf(), output.get_len());
    ASSERT_TRUE(input.prepare_next_submessage());
    ASSERT_TRUE(input.get_payload(deserialized_data));

    ASSERT_EQ(delete_payload.object_id(), deserialized_data.object_id());
    ASSERT_EQ(delete_payload.request_id(), deserialized_data.request_id());
}

} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
