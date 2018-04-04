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

#include "Common.h"

#include "MessageHeader.h"
#include "XRCETypes.h"
#include "Serializer.h"
#include "SubMessageHeader.h"
#include "XRCEFactory.h"
//#include "XRCEParser.h"
#include <fastcdr/exceptions/BadParamException.h>

#include <gtest/gtest.h>

#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdio.h>

#define BUFFER_LENGTH 200

namespace eprosima {
namespace micrortps {
namespace testing {

class SerializerDeserializerTests : public CommonData, public ::testing::Test
{
  protected:
    SerializerDeserializerTests()
        : test_buffer_(new char[BUFFER_LENGTH]), serializer_(test_buffer_, BUFFER_LENGTH),
          deserializer_(test_buffer_, BUFFER_LENGTH)
    {
    }

    virtual ~SerializerDeserializerTests()
    {
        delete[] test_buffer_;
    }

    char* test_buffer_ = nullptr;

    Serializer serializer_;
    Serializer deserializer_;
};

TEST_F(SerializerDeserializerTests, BufferEndOnNotReadWritedBuffer)
{
    char buffer[1];
    Serializer serializer(buffer, 1);
    ASSERT_FALSE(serializer.bufferEnd());
}

TEST_F(SerializerDeserializerTests, BufferEndOnBufferNoSize)
{
    char buffer[1];
    Serializer serializer(buffer, 0);
    ASSERT_TRUE(serializer.bufferEnd());
}

TEST_F(SerializerDeserializerTests, BufferEndSerialization)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    char* buffer = new char[message_header.getCdrSerializedSize(message_header)];
    Serializer serializer(buffer, message_header.getCdrSerializedSize(message_header));
    ASSERT_FALSE(serializer.bufferEnd());
    serializer.serialize(message_header);
    ASSERT_TRUE(serializer.bufferEnd());
    delete[] buffer;
}

TEST_F(SerializerDeserializerTests, BufferEndDeserialization)
{
    dds::xrce::MessageHeader message_header = generate_message_header();

    char* buffer = new char[message_header.getCdrSerializedSize(message_header)];
    Serializer serializer(buffer, message_header.getCdrSerializedSize(message_header));
    Serializer deserializer(buffer, message_header.getCdrSerializedSize(message_header));

    serializer.serialize(message_header);
    ASSERT_FALSE(deserializer.bufferEnd());
    deserializer.deserialize(message_header);
    ASSERT_TRUE(deserializer.bufferEnd());

    delete[] buffer;
}

TEST_F(SerializerDeserializerTests, MessageHeader)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    serializer_.serialize(message_header);

    dds::xrce::MessageHeader deserialized_header;
    deserializer_.deserialize(deserialized_header);

    ASSERT_EQ(message_header.client_key(), deserialized_header.client_key());
    ASSERT_EQ(message_header.session_id(), deserialized_header.session_id());
    ASSERT_EQ(message_header.stream_id(), deserialized_header.stream_id());
    ASSERT_EQ(message_header.sequence_nr(), deserialized_header.sequence_nr());
}

TEST_F(SerializerDeserializerTests, SubmessageHeader)
{
    dds::xrce::SubmessageHeader submessage_header = generate_submessage_header(dds::xrce::CREATE, 0x0001);
    serializer_.serialize(submessage_header);

    dds::xrce::SubmessageHeader deserialized_submessage_header;
    deserializer_.deserialize(deserialized_submessage_header);

    ASSERT_EQ(submessage_header.submessage_id(), deserialized_submessage_header.submessage_id());
    ASSERT_EQ(submessage_header.flags(), deserialized_submessage_header.flags());
    ASSERT_EQ(submessage_header.submessage_length(), deserialized_submessage_header.submessage_length());
}

TEST_F(SerializerDeserializerTests, CreateSubMessage)
{
    dds::xrce::CREATE_Payload create_data = generate_create_payload(dds::xrce::OBJK_PUBLISHER);
    serializer_.serialize(create_data);

    dds::xrce::CREATE_Payload deserialized_create_data;
    deserializer_.deserialize(deserialized_create_data);

    ASSERT_EQ(create_data.request_id(), deserialized_create_data.request_id());
    ASSERT_EQ(create_data.object_id(), deserialized_create_data.object_id());
    ASSERT_EQ(create_data.object_representation().publisher().participant_id(),
              deserialized_create_data.object_representation().publisher().participant_id());
    /* TODO (Julian): publisher do not support refenrence representation in the new standard. */
//    ASSERT_EQ(create_data.object_representation().publisher().representation().object_reference(),
//              deserialized_create_data.object_representation().publisher().representation().object_reference());
}

TEST_F(SerializerDeserializerTests, ResourceStatusSubmessage)
{
    dds::xrce::STATUS_Payload resource_status = generate_resource_status_payload(dds::xrce::STATUS_OK, 0x00);
    serializer_.serialize(resource_status);

    dds::xrce::STATUS_Payload deserialized_status;
    deserializer_.deserialize(deserialized_status);

    ASSERT_EQ(resource_status.related_request().object_id(), deserialized_status.related_request().object_id());
    ASSERT_EQ(resource_status.related_request().request_id(), deserialized_status.related_request().request_id());
    ASSERT_EQ(resource_status.result().status(), deserialized_status.result().status());
    ASSERT_EQ(resource_status.result().implementation_status(), deserialized_status.result().implementation_status());
}

TEST_F(SerializerDeserializerTests, ReadDataSubmessageNoFilter)
{
    dds::xrce::READ_DATA_Payload read_data = generate_read_data_payload(Optional<std::string>(), dds::xrce::FORMAT_DATA_SEQ);
    serializer_.serialize(read_data);

    dds::xrce::READ_DATA_Payload deserialized_read_data;
    deserializer_.deserialize(deserialized_read_data);

    ASSERT_THROW(read_data.read_specification().content_filter_expression(),
                 eprosima::fastcdr::exception::BadParamException);
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
    const Optional<std::string> test_filter = std::string("TEST");
    dds::xrce::READ_DATA_Payload read_data = generate_read_data_payload(test_filter, dds::xrce::FORMAT_DATA_SEQ);
    serializer_.serialize(read_data);

    dds::xrce::READ_DATA_Payload deserialized_read_data;
    deserializer_.deserialize(deserialized_read_data);

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
    dds::xrce::WRITE_DATA_Payload_Data write_payload = generate_write_data_payload();
    serializer_.serialize(write_payload);

    dds::xrce::WRITE_DATA_Payload_Data deserialized_write_data;
    deserializer_.deserialize(deserialized_write_data);

    ASSERT_EQ(write_payload.request_id(), deserialized_write_data.request_id());
    ASSERT_EQ(write_payload.object_id(), deserialized_write_data.object_id());
    ASSERT_EQ(write_payload.data().serialized_data(),
              deserialized_write_data.data().serialized_data());
}

TEST_F(SerializerDeserializerTests, DataSubmessage)
{
    dds::xrce::DATA_Payload_Data data_payload = generate_data_payload_data();
    serializer_.serialize(data_payload);

    dds::xrce::DATA_Payload_Data deserialized_data;
    deserializer_.deserialize(deserialized_data);
    ASSERT_EQ(data_payload.object_id(), deserialized_data.object_id());
    ASSERT_EQ(data_payload.request_id(), deserialized_data.request_id());
    ASSERT_EQ(data_payload.data().serialized_data(), deserialized_data.data().serialized_data());
}

TEST_F(SerializerDeserializerTests, DeleteSubmessage)
{
    dds::xrce::DELETE_Payload delete_payload = generate_delete_resource_payload(object_id);
    serializer_.serialize(delete_payload);

    dds::xrce::DELETE_Payload deserialized_data;
    deserializer_.deserialize(deserialized_data);
    ASSERT_EQ(delete_payload.object_id(), deserialized_data.object_id());
    ASSERT_EQ(delete_payload.request_id(), deserialized_data.request_id());
}

class XRCEFactoryTests : public CommonData, public ::testing::Test
{
  protected:
    XRCEFactoryTests()
        : test_buffer_(new char[BUFFER_LENGTH]), serializer_(test_buffer_, BUFFER_LENGTH),
          deserializer_(test_buffer_, BUFFER_LENGTH)
    {
    }

    virtual ~XRCEFactoryTests()
    {
        delete[] test_buffer_;
    }

    char* test_buffer_ = nullptr;
    Serializer serializer_;
    Serializer deserializer_;
};

TEST_F(XRCEFactoryTests, MessageHeader)
{
    XRCEFactory newMessage{test_buffer_, BUFFER_LENGTH};
    dds::xrce::MessageHeader message_header = generate_message_header();
    newMessage.header(message_header);

    dds::xrce::MessageHeader deserialized_header;
    deserializer_.deserialize(deserialized_header);

    ASSERT_EQ(message_header.client_key(), deserialized_header.client_key());
    ASSERT_EQ(message_header.session_id(), deserialized_header.session_id());
    ASSERT_EQ(message_header.stream_id(), deserialized_header.stream_id());
    ASSERT_EQ(message_header.sequence_nr(), deserialized_header.sequence_nr());
}

TEST_F(XRCEFactoryTests, MessageSize)
{
    XRCEFactory newMessage{test_buffer_, BUFFER_LENGTH};
    dds::xrce::MessageHeader message_header = generate_message_header();
    newMessage.header(message_header);

    ASSERT_EQ(newMessage.get_total_size(), message_header.getCdrSerializedSize(message_header));

    dds::xrce::SubmessageHeader submessage_header;
    dds::xrce::DATA_Payload_Data data_payload = generate_data_payload_data();
    newMessage.data(data_payload);

    ASSERT_EQ(newMessage.get_total_size(), message_header.getCdrSerializedSize(message_header) +
                                               submessage_header.getCdrSerializedSize(submessage_header) +
                                               data_payload.getCdrSerializedSize());
}

TEST_F(XRCEFactoryTests, MessageData)
{
    XRCEFactory newMessage{test_buffer_, BUFFER_LENGTH};

    dds::xrce::MessageHeader message_header = generate_message_header();
    newMessage.header(message_header);
    dds::xrce::DATA_Payload_Data data_payload = generate_data_payload_data();
    newMessage.data(data_payload);
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::DATA, static_cast<uint16_t>(data_payload.getCdrSerializedSize()));

    dds::xrce::MessageHeader deserialized_header;
    dds::xrce::SubmessageHeader deserialized_submessage_header;
    dds::xrce::DATA_Payload_Data deserialized_data_payload;
    deserializer_.deserialize(deserialized_header);
    deserializer_.deserialize(deserialized_submessage_header);
    deserializer_.deserialize(deserialized_data_payload);

    ASSERT_EQ(client_key, deserialized_header.client_key());
    ASSERT_EQ(session_id, deserialized_header.session_id());
    ASSERT_EQ(stream_id, deserialized_header.stream_id());
    ASSERT_EQ(sequence_nr, deserialized_header.sequence_nr());

    ASSERT_EQ(submessage_header.submessage_id(), deserialized_submessage_header.submessage_id());
    ASSERT_EQ(submessage_header.flags(), deserialized_submessage_header.flags());
    ASSERT_EQ(submessage_header.submessage_length(), deserialized_submessage_header.submessage_length());

    ASSERT_EQ(data_payload.object_id(), deserialized_data_payload.object_id());
    ASSERT_EQ(data_payload.request_id(), deserialized_data_payload.request_id());
    ASSERT_EQ(data_payload.data().serialized_data(), deserialized_data_payload.data().serialized_data());
}

TEST_F(XRCEFactoryTests, StatusMessage)
{
    XRCEFactory newMessage{test_buffer_, BUFFER_LENGTH};

    dds::xrce::MessageHeader message_header = generate_message_header();
    newMessage.header(message_header);
    dds::xrce::STATUS_Payload resource_status = generate_resource_status_payload(dds::xrce::STATUS_OK, 0x00);
    newMessage.status(resource_status);
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::STATUS, static_cast<uint16_t>(resource_status.getCdrSerializedSize()));

    dds::xrce::MessageHeader deserialized_header;
    dds::xrce::SubmessageHeader deserialized_submessage_header;
    dds::xrce::STATUS_Payload deserialized_status;
    deserializer_.deserialize(deserialized_header);
    deserializer_.deserialize(deserialized_submessage_header);
    deserializer_.deserialize(deserialized_status);

    ASSERT_EQ(client_key, deserialized_header.client_key());
    ASSERT_EQ(session_id, deserialized_header.session_id());
    ASSERT_EQ(stream_id, deserialized_header.stream_id());
    ASSERT_EQ(sequence_nr, deserialized_header.sequence_nr());

    ASSERT_EQ(submessage_header.submessage_id(), deserialized_submessage_header.submessage_id());
    ASSERT_EQ(submessage_header.flags(), deserialized_submessage_header.flags());
    ASSERT_EQ(submessage_header.submessage_length(), deserialized_submessage_header.submessage_length());

    ASSERT_EQ(resource_status.related_request().object_id(), deserialized_status.related_request().object_id());
    ASSERT_EQ(resource_status.related_request().request_id(), deserialized_status.related_request().request_id());
    ASSERT_EQ(resource_status.result().status(), deserialized_status.result().status());
    ASSERT_EQ(resource_status.result().implementation_status(), deserialized_status.result().implementation_status());
}

class XRCEParserTests : public CommonData, public ::testing::Test
{
  public:
    class CountListener : public XRCEListener
    {
      public:
        void on_message(const dds::xrce::MessageHeader& /*header*/,
                        const dds::xrce::SubmessageHeader& /*sub_header*/,
                        const dds::xrce::CREATE_CLIENT_Payload& /*create_client_payload*/) override
        {
            ++clients;
        }

        void on_message(const dds::xrce::MessageHeader& /*header*/,
                        const dds::xrce::SubmessageHeader& /*sub_header*/,
                        const dds::xrce::CREATE_Payload& /*create_payload*/) override
        {
            ++creates;
        }

        void on_message(const dds::xrce::MessageHeader& /*header*/,
                        const dds::xrce::SubmessageHeader& /*sub_header*/,
                        const dds::xrce::DELETE_Payload& /*create_payload*/) override
        {
            ++deletes;
        }

        void on_message(const dds::xrce::MessageHeader& /*header*/,
                        const dds::xrce::SubmessageHeader& /*sub_header*/,
                        dds::xrce::WRITE_DATA_Payload_Data& /*write_payload*/) override
        {
            ++writes;
        }

        void on_message(const dds::xrce::MessageHeader& /*header*/,
                        const dds::xrce::SubmessageHeader& /*sub_header*/,
                        const dds::xrce::READ_DATA_Payload& /*read_payload*/) override
        {
            ++reads;
        }

        void on_message(const dds::xrce::MessageHeader& /*header*/,
                        const dds::xrce::SubmessageHeader& /*sub_header*/,
                        const dds::xrce::ACKNACK_Payload& /*read_payload*/) override
        {
            ++acknack;
        }

        void on_message(const dds::xrce::MessageHeader& /*header*/,
                        const dds::xrce::SubmessageHeader& /*sub_header*/,
                        const dds::xrce::HEARTBEAT_Payload& /*read_payload*/) override
        {
            ++heartbeat;
        }

        int clients 	= 0;
        int creates 	= 0;
        int writes  	= 0;
        int reads   	= 0;
        int deletes 	= 0;
        int acknack 	= 0;
        int heartbeat 	= 0;
    };

  protected:
    XRCEParserTests()
        : test_buffer_(new char[BUFFER_LENGTH * 2]), serializer_(test_buffer_, BUFFER_LENGTH * 2),
          deserializer_(test_buffer_, BUFFER_LENGTH * 2)
    {
    }

    virtual ~XRCEParserTests()
    {
        delete[] test_buffer_;
    }
    char* test_buffer_ = nullptr;

    CountListener count_listener_;
    Serializer serializer_;
    Serializer deserializer_;
};

TEST_F(XRCEParserTests, DISABLED_EmptyMessage)
{
    ::testing::internal::CaptureStderr();
    XRCEParser myParser{test_buffer_, BUFFER_LENGTH * 2, &count_listener_};
    ASSERT_FALSE(myParser.parse());
    ASSERT_TRUE(::testing::internal::GetCapturedStderr().find("Error submessage ID not recognized\n") !=
                std::string::npos);
}

TEST_F(XRCEParserTests, HeaderError)
{
    ::testing::internal::CaptureStderr();
    XRCEParser myParser{test_buffer_, 0, &count_listener_};
    ASSERT_FALSE(myParser.parse());
    ASSERT_TRUE(::testing::internal::GetCapturedStderr().find("Error reading message header\n") != std::string::npos);
}

TEST_F(XRCEParserTests, SubmessageHeaderError)
{
    ::testing::internal::CaptureStderr();
    dds::xrce::MessageHeader message_header;
    XRCEParser myParser{test_buffer_, message_header.getCdrSerializedSize(message_header), &count_listener_};
    ASSERT_FALSE(myParser.parse());
    ASSERT_TRUE(::testing::internal::GetCapturedStderr().find("Error reading submessage header\n") !=
                std::string::npos);
}

TEST_F(XRCEParserTests, DISABLED_SubmessagePayloadError)
{
    ::testing::internal::CaptureStderr();
    dds::xrce::MessageHeader message_header;
    dds::xrce::SubmessageHeader submessage_header;
    XRCEParser myParser{test_buffer_,
                        message_header.getCdrSerializedSize(message_header) +
                            submessage_header.getCdrSerializedSize(submessage_header),
                        &count_listener_};
    ASSERT_FALSE(myParser.parse());
    ASSERT_TRUE(::testing::internal::GetCapturedStderr().find("Error submessage ID not recognized\n") !=
                std::string::npos);
}

TEST_F(XRCEParserTests, CreateClientMessage)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    serializer_.serialize(message_header);
    dds::xrce::CREATE_CLIENT_Payload create_data = generate_create_client_payload();
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::CREATE_CLIENT, static_cast<uint16_t>(create_data.getCdrSerializedSize()));

    serializer_.serialize(submessage_header);
    serializer_.serialize(create_data);

    XRCEParser myParser{test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_EQ(count_listener_.clients, 0);
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.clients, 1);
}

TEST_F(XRCEParserTests, CreateMessage)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    serializer_.serialize(message_header);
    dds::xrce::CREATE_Payload create_data = generate_create_payload(dds::xrce::OBJK_PUBLISHER);
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::CREATE, static_cast<uint16_t>(create_data.getCdrSerializedSize()));

    serializer_.serialize(submessage_header);
    serializer_.serialize(create_data);

    XRCEParser myParser{test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_EQ(count_listener_.creates, 0);
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.creates, 1);
}

TEST_F(XRCEParserTests, DeleteMessage)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    serializer_.serialize(message_header);
    dds::xrce::DELETE_Payload delete_data = generate_delete_resource_payload(object_id);
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::DELETE, static_cast<uint16_t>(delete_data.getCdrSerializedSize()));

    serializer_.serialize(submessage_header);
    serializer_.serialize(delete_data);

    XRCEParser myParser{test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_EQ(count_listener_.deletes, 0);
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.deletes, 1);
}

TEST_F(XRCEParserTests, MultiCreateMessage)
{
    const int num_creates = 3;
    dds::xrce::MessageHeader message_header = generate_message_header();
    serializer_.serialize(message_header);

    dds::xrce::CREATE_Payload create_data = generate_create_payload(dds::xrce::OBJK_PARTICIPANT);
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::CREATE, static_cast<uint16_t>(create_data.getCdrSerializedSize()));
    for(int i = 0; i < num_creates; ++i)
    {
        serializer_.serialize(submessage_header);
        serializer_.serialize(create_data);
    }

    XRCEParser myParser{test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_EQ(count_listener_.creates, 0);
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.creates, num_creates);
}

TEST_F(XRCEParserTests, WriteMessage)
{
    dds::xrce::MessageHeader message_header     = generate_message_header();
    dds::xrce::WRITE_DATA_Payload_Data write_payload = generate_write_data_payload();
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::WRITE_DATA, static_cast<uint16_t>(write_payload.getCdrSerializedSize()));
    serializer_.serialize(message_header);
    serializer_.serialize(submessage_header);
    serializer_.serialize(write_payload);

    XRCEParser myParser{test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_EQ(count_listener_.writes, 0);
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.writes, 1);
}

TEST_F(XRCEParserTests, MultiWriteMessage)
{
    const int num_writes             = 3;
    dds::xrce::MessageHeader message_header     = generate_message_header();
    dds::xrce::WRITE_DATA_Payload_Data write_payload = generate_write_data_payload();
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::WRITE_DATA, static_cast<uint16_t>(write_payload.getCdrSerializedSize()));
    serializer_.serialize(message_header);
    for(int i = 0; i < num_writes; ++i)
    {
        serializer_.serialize(submessage_header);
        serializer_.serialize(write_payload);
    }

    XRCEParser myParser{test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_EQ(count_listener_.writes, 0);
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.writes, num_writes);
}

TEST_F(XRCEParserTests, ReadMessage)
{
    dds::xrce::MessageHeader message_header = generate_message_header();
    dds::xrce::READ_DATA_Payload read_data  = generate_read_data_payload(Optional<std::string>(), dds::xrce::FORMAT_DATA_SEQ);
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::READ_DATA, static_cast<uint16_t>(read_data.getCdrSerializedSize()));
    serializer_.serialize(message_header);
    serializer_.serialize(submessage_header);
    serializer_.serialize(read_data);

    XRCEParser myParser{test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_EQ(count_listener_.reads, 0);
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.reads, 1);
}

TEST_F(XRCEParserTests, MultiReadMessage)
{
    const int num_reads = 3;
    dds::xrce::MessageHeader message_header = generate_message_header();
    dds::xrce::READ_DATA_Payload read_data  = generate_read_data_payload(Optional<std::string>(), dds::xrce::FORMAT_DATA_SEQ);
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::READ_DATA, static_cast<uint16_t>(read_data.getCdrSerializedSize()));
    serializer_.serialize(message_header);
    for(int i = 0; i < num_reads; ++i)
    {
        serializer_.force_new_submessage_align();
        serializer_.serialize(submessage_header);
        serializer_.serialize(read_data);
    }

    XRCEParser myParser{test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_EQ(count_listener_.reads, 0);
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.reads, num_reads);
}

TEST_F(XRCEParserTests, MultiSubMessage)
{
    const int num_creates = 3;
    const int num_reads   = 3;
    const int num_writes  = 3;
    const int num_deletes  = 3;

    dds::xrce::MessageHeader message_header = generate_message_header();
    serializer_.serialize(message_header);

    dds::xrce::CREATE_Payload create_data = generate_create_payload(dds::xrce::OBJK_PUBLISHER);
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::CREATE, static_cast<uint16_t>(create_data.getCdrSerializedSize()));
    for(int i = 0; i < num_creates; ++i)
    {
        serializer_.force_new_submessage_align();
        serializer_.serialize(submessage_header);
        serializer_.serialize(create_data);
    }

    dds::xrce::READ_DATA_Payload read_data = generate_read_data_payload(Optional<std::string>(), dds::xrce::FORMAT_DATA_SEQ);
    submessage_header = generate_submessage_header(dds::xrce::READ_DATA, static_cast<uint16_t>(read_data.getCdrSerializedSize()));
    for(int i = 0; i < num_reads; ++i)
    {
        serializer_.force_new_submessage_align();
        serializer_.serialize(submessage_header);
        serializer_.serialize(read_data);
    }

    dds::xrce::WRITE_DATA_Payload_Data write_payload = generate_write_data_payload();
    submessage_header =
        generate_submessage_header(dds::xrce::WRITE_DATA, static_cast<uint16_t>(write_payload.getCdrSerializedSize()));
    for(int i = 0; i < num_writes; ++i)
    {
        serializer_.force_new_submessage_align();
        serializer_.serialize(submessage_header);
        serializer_.serialize(write_payload);
    }

    dds::xrce::DELETE_Payload delete_data = generate_delete_resource_payload(object_id);
    submessage_header = generate_submessage_header(dds::xrce::DELETE, static_cast<uint16_t>(delete_data.getCdrSerializedSize()));
    for(int i = 0; i < num_deletes; ++i)
    {
        serializer_.force_new_submessage_align();
        serializer_.serialize(submessage_header);
        serializer_.serialize(delete_data);
    }

    XRCEParser myParser{test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_EQ(count_listener_.creates, 0);
    ASSERT_EQ(count_listener_.writes, 0);
    ASSERT_EQ(count_listener_.reads, 0);
    ASSERT_EQ(count_listener_.deletes, 0);
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.creates, num_creates);
    ASSERT_EQ(count_listener_.reads, num_reads);
    ASSERT_EQ(count_listener_.writes, num_writes);
    ASSERT_EQ(count_listener_.deletes, num_deletes);
}

class XRCEFileTests : public CommonData, public ::testing::Test
{
  protected:
    XRCEFileTests() : test_buffer_(new char[BUFFER_LENGTH])
    {
    }

    virtual ~XRCEFileTests()
    {
        delete[] test_buffer_;
    }
    char* test_buffer_ = nullptr;
};

TEST_F(XRCEFileTests, Simple)
{
    XRCEFactory newMessage{test_buffer_, BUFFER_LENGTH};
    dds::xrce::MessageHeader message_header = generate_message_header();
    newMessage.header(message_header);
    dds::xrce::STATUS_Payload resource_status = generate_resource_status_payload(dds::xrce::STATUS_OK, 0x00);
    newMessage.status(resource_status);
    dds::xrce::SubmessageHeader submessage_header =
        generate_submessage_header(dds::xrce::STATUS, static_cast<uint16_t>(resource_status.getCdrSerializedSize()));

    std::string filename = "test.bin";
    std::fstream s(filename, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    if(!s.is_open())
    {
        std::cout << "failed to open " << filename << '\n';
    }
    else
    {
        s.write(test_buffer_, newMessage.get_total_size());
        // for fstream, this moves the file position pointer (both put and get)
        s.seekp(0);

        char* read_buffer = new char[newMessage.get_total_size()];
        // read
        s.read(read_buffer, newMessage.get_total_size()); // binary input

        Serializer deserializer_(read_buffer, newMessage.get_total_size());
        dds::xrce::MessageHeader deserialized_header;
        dds::xrce::SubmessageHeader deserialized_submessage_header;
        dds::xrce::STATUS_Payload deserialized_status;
        // RESOURCE_STATUS_PAYLOAD deserialized_status_payload;
        deserializer_.deserialize(deserialized_header);
        deserializer_.deserialize(deserialized_submessage_header);
        deserializer_.deserialize(deserialized_status);

        ASSERT_EQ(message_header.client_key(), deserialized_header.client_key());
        ASSERT_EQ(message_header.session_id(), deserialized_header.session_id());
        ASSERT_EQ(message_header.stream_id(), deserialized_header.stream_id());
        ASSERT_EQ(message_header.sequence_nr(), deserialized_header.sequence_nr());

        ASSERT_EQ(submessage_header.submessage_id(), deserialized_submessage_header.submessage_id());
        ASSERT_EQ(submessage_header.flags(), deserialized_submessage_header.flags());
        ASSERT_EQ(submessage_header.submessage_length(), deserialized_submessage_header.submessage_length());

        ASSERT_EQ(resource_status.related_request().object_id(), deserialized_status.related_request().object_id());
        ASSERT_EQ(resource_status.related_request().request_id(), deserialized_status.related_request().request_id());
        ASSERT_EQ(resource_status.result().status(), deserialized_status.result().status());
        ASSERT_EQ(resource_status.result().implementation_status(),
                  deserialized_status.result().implementation_status());
        delete[] read_buffer;
    }
}
} // namespace testing
} // namespace micrortps
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
