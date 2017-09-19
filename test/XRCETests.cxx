// XRCE.cpp : Defines the entry point for the console application.


#include "agent/MessageHeader.h"
#include "agent/SubMessageHeader.h"
#include "agent/Serializer.h"
#include "agent/Payloads.h"
#include "agent/XRCEFactory.h"
#include "agent/XRCEParser.h"
#include "agent/ObjectVariant.h"
#include <gtest/gtest.h>

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <functional>
#include <fstream>

#define BUFFER_LENGTH 200

void print(const MessageHeader& header)
{
    std::cout << "=MESSAGE HEADER=" << std::endl;
    std::cout << "	client key:  " << std::hex << header.client_key() << std::endl;
    std::cout << "	session id:  " << std::hex << +header.session_id() << std::endl;
    std::cout << "	stream id:   " << std::hex << +header.stream_id() << std::endl;
    std::cout << "	sequence nr: " << std::hex << header.sequence_nr() << std::endl;
    std::cout << "===============" << std::endl;
}
void print(const SubmessageHeader& subheader)
{
    std::cout << "=SUBMESSAGE HEADER=" << std::endl;
    std::cout << "	submessage id:     " << std::hex << static_cast<int>(subheader.submessage_id()) << std::endl;
    std::cout << "	flags:             " << std::hex << static_cast<int>(subheader.flags()) << std::endl;
    std::cout << "	submessage length: " << std::hex << static_cast<int>(subheader.submessage_length()) << std::endl;
    std::cout << "=====================" << std::endl;
}

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}

class SerializerDeserializerTests : public testing::Test
{
protected:
    SerializerDeserializerTests() :
         test_buffer_(new char[BUFFER_LENGTH])
     {
     }

     virtual ~SerializerDeserializerTests() {
         delete[] test_buffer_;
     }


    char* test_buffer_ = nullptr;

    const uint32_t client_key = 0xF1F2F3F4;
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;

    const subMessageId submessage_id = CREATE;
    const uint8_t flags = 0x07;
    const uint8_t submessage_length = 0x0015;

};

TEST_F(SerializerDeserializerTests, BufferEndOnNotReadWritedBuffer)
{
    char buffer[1];
    Serializer serializer_(buffer, 1);
    ASSERT_FALSE(serializer_.bufferEnd());
}

TEST_F(SerializerDeserializerTests, BufferEndOnBufferNoSize)
{
    char buffer[1];
    Serializer serializer_(buffer, 0);
    ASSERT_TRUE(serializer_.bufferEnd());
}

TEST_F(SerializerDeserializerTests, BufferEndSerialization)
{
    MessageHeader message_header;
    char* buffer = new char[message_header.getCdrSerializedSize(message_header)];
    Serializer serializer_(buffer, message_header.getCdrSerializedSize(message_header));
    ASSERT_FALSE(serializer_.bufferEnd());
    serializer_.serialize(message_header);
    ASSERT_TRUE(serializer_.bufferEnd());
    delete[] buffer;
}

TEST_F(SerializerDeserializerTests, BufferEndDeserialization)
{
    MessageHeader message_header;
    char* buffer = new char[message_header.getCdrSerializedSize(message_header)];
    Serializer serializer_(buffer, message_header.getCdrSerializedSize(message_header));
    ASSERT_FALSE(serializer_.bufferEnd());
    serializer_.deserialize(message_header);
    ASSERT_TRUE(serializer_.bufferEnd());
    delete[] buffer;
}

TEST_F(SerializerDeserializerTests, MessageHeader)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);
    serializer_.serialize(message_header);

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader deserialized_header;
    deserializer_.deserialize(deserialized_header);

    ASSERT_EQ(message_header.client_key(), deserialized_header.client_key());
    ASSERT_EQ(message_header.session_id(), deserialized_header.session_id());
    ASSERT_EQ(message_header.stream_id(), deserialized_header.stream_id());
    ASSERT_EQ(message_header.sequence_nr(), deserialized_header.sequence_nr());
}

TEST_F(SerializerDeserializerTests, SubmessageHeader)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(submessage_id); // STATUS
    submessage_header.flags(flags);
    submessage_header.submessage_length(submessage_length);
    serializer_.serialize(submessage_header);

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    SubmessageHeader deserialized_submessage_header;
    deserializer_.deserialize(deserialized_submessage_header);

    ASSERT_EQ(submessage_header.submessage_id(), deserialized_submessage_header.submessage_id());
    ASSERT_EQ(submessage_header.flags(), deserialized_submessage_header.flags());
    ASSERT_EQ(submessage_header.submessage_length(), deserialized_submessage_header.submessage_length());
}

TEST_F(SerializerDeserializerTests, CreateSubMessage)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    CREATE_PAYLOAD createData;
    createData.request_id({ 1,2 });
    createData.object_id({ 10,20,30 });
    ObjectVariant variant;
    OBJK_PUBLISHER_Representation pubRep;
    pubRep.as_string(std::string("PUBLISHER"));
    pubRep.participant_id({ 4,4,4 });
    variant.publisher(pubRep);
    createData.object_representation(variant);

    serializer_.serialize(createData);

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    CREATE_PAYLOAD deserialized_create_data;
    deserializer_.deserialize(deserialized_create_data);

    ASSERT_EQ(createData.request_id(), deserialized_create_data.request_id());
    ASSERT_EQ(createData.object_id(), deserialized_create_data.object_id());
    ASSERT_EQ(createData.object_representation().publisher().as_string(), deserialized_create_data.object_representation().publisher().as_string());
    ASSERT_EQ(createData.object_representation().publisher().participant_id(), deserialized_create_data.object_representation().publisher().participant_id());
}

TEST_F(SerializerDeserializerTests, ResourceStatusSubmessage)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);

    OBJK_DATAWRITER_Status data_writer;
    data_writer.stream_seq_num(0x7FFF);
    data_writer.sample_seq_num(0x1234567890ABCDEF);
    StatusVariant variant;
    variant.data_writer(data_writer);
    ResultStatus result;
    result.request_id({ 0x10, 0x09, 0x08, 0x07 });
    result.status(STATUS_OK);
    result.implementation_status(STATUS_ERR_INCOMPATIBLE);
    Status status;
    status.status(variant);
    status.result(result);
    status.object_id({ 10,20,30 });

    RESOURCE_STATUS_PAYLOAD resource_status;
    resource_status.request_id({ 0x00, 0x01, 0x02, 0x03 });
    resource_status.request_status(status);
    serializer_.serialize(resource_status);

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    RESOURCE_STATUS_PAYLOAD deserialized_resource_status;
    deserializer_.deserialize(deserialized_resource_status);

    ASSERT_EQ(resource_status.request_id(), deserialized_resource_status.request_id());
    ASSERT_EQ(resource_status.request_status().object_id(), deserialized_resource_status.request_status().object_id());
    ASSERT_EQ(resource_status.request_status().result().request_id(), deserialized_resource_status.request_status().result().request_id());
    ASSERT_EQ(resource_status.request_status().result().status(), deserialized_resource_status.request_status().result().status());
    ASSERT_EQ(resource_status.request_status().result().implementation_status(), deserialized_resource_status.request_status().result().implementation_status());
    ASSERT_EQ(resource_status.request_status().status().data_writer().stream_seq_num(), deserialized_resource_status.request_status().status().data_writer().stream_seq_num());
    ASSERT_EQ(resource_status.request_status().status().data_writer().sample_seq_num(), deserialized_resource_status.request_status().status().data_writer().sample_seq_num());
}

TEST_F(SerializerDeserializerTests, DataSubmessage)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);

    RT_Data data_reader;
    SampleData sample_data;
    sample_data.serialized_data({ 0x00,0x11,0x22, 0x33 });
    data_reader.data(sample_data);
    
    DATA_PAYLOAD data;
    data.data_reader(data_reader);
    data.resource_id({ 0xF0,0xF1,0xF2 });
    data.request_id({ 0x00,0x11,0x22, 0x33 });
    serializer_.serialize(data);

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    DATA_PAYLOAD deserialized_data;
    deserializer_.deserialize(deserialized_data);

    ASSERT_EQ(data.request_id(), deserialized_data.request_id());
    ASSERT_EQ(data.resource_id(), deserialized_data.resource_id());
    ASSERT_EQ(data.data_reader().data().serialized_data(), deserialized_data.data_reader().data().serialized_data());
}

TEST_F(SerializerDeserializerTests, ReadDataSubmessage)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);

    READ_DATA_PAYLOAD read_data;
    read_data.request_id({ 0x89, 0x89, 0x89, 0x89 });
    read_data.object_id({ 0x05, 0x65, 0x65 });
    read_data.max_messages(0x0001);
    read_data.read_mode(READM_DATA);
    read_data.max_elapsed_time(16);//0x00000010);
    read_data.max_rate(0x00000001);
    read_data.content_filter_expression("FILTER EXPRESION");
    read_data.max_messages(0x0001);
    read_data.include_sample_info(false);
    serializer_.serialize(read_data);

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    READ_DATA_PAYLOAD deserialized_read_data;
    deserializer_.deserialize(deserialized_read_data);

    ASSERT_EQ(read_data.request_id(), deserialized_read_data.request_id());
    ASSERT_EQ(read_data.object_id(), deserialized_read_data.object_id());
    ASSERT_EQ(read_data.max_messages(), deserialized_read_data.max_messages());
    ASSERT_EQ(read_data.read_mode(), deserialized_read_data.read_mode());
    ASSERT_EQ(read_data.max_elapsed_time(), deserialized_read_data.max_elapsed_time());
    ASSERT_EQ(read_data.max_rate(), deserialized_read_data.max_rate());
    ASSERT_EQ(read_data.content_filter_expression(), deserialized_read_data.content_filter_expression());
    ASSERT_EQ(read_data.max_messages(), deserialized_read_data.max_messages());
    ASSERT_EQ(read_data.include_sample_info(), deserialized_read_data.include_sample_info());
}

TEST_F(SerializerDeserializerTests, WriteDataSubmessage)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);

    RT_Data data_writer;
    SampleData sample_data;
    sample_data.serialized_data({ 0x00,0x11,0x22, 0x33 });
    data_writer.data(sample_data);

    WRITE_DATA_PAYLOAD write_data;
    write_data.request_id({ 0x89, 0x89, 0x89, 0x89 });
    write_data.object_id({ 0x05, 0x65, 0x65 });
    write_data.data_writer(data_writer);

    serializer_.serialize(write_data);

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    WRITE_DATA_PAYLOAD deserialized_write_data;
    deserializer_.deserialize(deserialized_write_data);

    ASSERT_EQ(write_data.request_id(), deserialized_write_data.request_id());
    ASSERT_EQ(write_data.object_id(), deserialized_write_data.object_id());
    ASSERT_EQ(write_data.data_writer().data().serialized_data(), deserialized_write_data.data_writer().data().serialized_data());
}

class XRCEFactoryTests : public testing::Test
{
protected:
    XRCEFactoryTests() :
        test_buffer_(new char[BUFFER_LENGTH])
    {
    }

    virtual ~XRCEFactoryTests() {
        delete[] test_buffer_;
    }


    char* test_buffer_ = nullptr;

    const uint32_t client_key = 0xF1F2F3F4;
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
};

TEST_F(XRCEFactoryTests, MessageHeader)
{
    XRCEFactory newMessage{ test_buffer_, BUFFER_LENGTH };
    newMessage.header(client_key, session_id, stream_id, sequence_nr);

    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader deserialized_header;
    deserializer_.deserialize(deserialized_header);

    ASSERT_EQ(message_header.client_key(), deserialized_header.client_key());
    ASSERT_EQ(message_header.session_id(), deserialized_header.session_id());
    ASSERT_EQ(message_header.stream_id(), deserialized_header.stream_id());
    ASSERT_EQ(message_header.sequence_nr(), deserialized_header.sequence_nr());
}

TEST_F(XRCEFactoryTests, MessageSize)
{
    XRCEFactory newMessage{ test_buffer_, BUFFER_LENGTH };
    newMessage.header(client_key, session_id, stream_id, sequence_nr);

    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    ASSERT_EQ(newMessage.get_total_size(), message_header.getCdrSerializedSize(message_header));

    SubmessageHeader submessage_header;
    DATA_PAYLOAD data_payload;
    newMessage.data(data_payload);

    ASSERT_EQ(newMessage.get_total_size(), message_header.getCdrSerializedSize(message_header) + submessage_header.getCdrSerializedSize(submessage_header) + data_payload.getCdrSerializedSize(data_payload));
}

TEST_F(XRCEFactoryTests, MessageData)
{
    XRCEFactory newMessage{ test_buffer_, BUFFER_LENGTH };
    newMessage.header(client_key, session_id, stream_id, sequence_nr);

    RT_Data data_reader;
    SampleData sample_data;
    sample_data.serialized_data({ 0x00,0x11,0x22, 0x33 });
    data_reader.data(sample_data);

    DATA_PAYLOAD data;
    data.data_reader(data_reader);
    data.resource_id({ 0xF0,0xF1,0xF2 });
    data.request_id({ 0x00,0x11,0x22, 0x33 });

    newMessage.data(data);

    SubmessageHeader submessage_header;
    submessage_header.submessage_id(DATA);
    submessage_header.flags(0x07);
    submessage_header.submessage_length(static_cast<uint16_t>(data.getCdrSerializedSize(data)));

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader deserialized_header;
    SubmessageHeader deserialized_submessage_header;
    DATA_PAYLOAD deserialized_data_payload;
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

    ASSERT_EQ(data.request_id(), deserialized_data_payload.request_id());
    ASSERT_EQ(data.resource_id(), deserialized_data_payload.resource_id());
    ASSERT_EQ(data.data_reader().data().serialized_data(), deserialized_data_payload.data_reader().data().serialized_data());
}

TEST_F(XRCEFactoryTests, StatusMessage)
{
    XRCEFactory newMessage{ test_buffer_, BUFFER_LENGTH };
    newMessage.header(client_key, session_id, stream_id, sequence_nr);

    OBJK_DATAWRITER_Status data_writer;
    data_writer.stream_seq_num(0x7FFF);
    data_writer.sample_seq_num(0x1234567890ABCDEF);
    StatusVariant variant;
    variant.data_writer(data_writer);
    ResultStatus result;
    result.request_id({ 0x10, 0x09, 0x08, 0x07 });
    result.status(STATUS_OK);
    result.implementation_status(STATUS_ERR_INCOMPATIBLE);
    Status status;
    status.status(variant);
    status.result(result);
    status.object_id({ 10,20,30 });

    RESOURCE_STATUS_PAYLOAD resource_status;
    resource_status.request_id({ 0x00, 0x01, 0x02, 0x03 });
    resource_status.request_status(status);
    
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(STATUS);
    submessage_header.flags(0x07);
    submessage_header.submessage_length(static_cast<uint16_t>(resource_status.getCdrSerializedSize(resource_status)));

    newMessage.status(resource_status);

    Serializer deserializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader deserialized_header;
    SubmessageHeader deserialized_submessage_header;
    RESOURCE_STATUS_PAYLOAD deserialized_status_payload;
    deserializer_.deserialize(deserialized_header);
    deserializer_.deserialize(deserialized_submessage_header);
    deserializer_.deserialize(deserialized_status_payload);

    ASSERT_EQ(client_key, deserialized_header.client_key());
    ASSERT_EQ(session_id, deserialized_header.session_id());
    ASSERT_EQ(stream_id, deserialized_header.stream_id());
    ASSERT_EQ(sequence_nr, deserialized_header.sequence_nr());

    ASSERT_EQ(submessage_header.submessage_id(), deserialized_submessage_header.submessage_id());
    ASSERT_EQ(submessage_header.flags(), deserialized_submessage_header.flags());
    ASSERT_EQ(submessage_header.submessage_length(), deserialized_submessage_header.submessage_length());

    ASSERT_EQ(resource_status.request_id(), deserialized_status_payload.request_id());
    ASSERT_EQ(resource_status.request_status().object_id(), deserialized_status_payload.request_status().object_id());
    ASSERT_EQ(resource_status.request_status().result().request_id(), deserialized_status_payload.request_status().result().request_id());
    ASSERT_EQ(resource_status.request_status().result().status(), deserialized_status_payload.request_status().result().status());
    ASSERT_EQ(resource_status.request_status().result().implementation_status(), deserialized_status_payload.request_status().result().implementation_status());
    ASSERT_EQ(resource_status.request_status().status().data_writer().stream_seq_num(), deserialized_status_payload.request_status().status().data_writer().stream_seq_num());
    ASSERT_EQ(resource_status.request_status().status().data_writer().sample_seq_num(), deserialized_status_payload.request_status().status().data_writer().sample_seq_num());
}

class XRCEParserTests : public testing::Test
{
public:

    class BoolListener : public XRCEListener
    {
public:
    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const CREATE_PAYLOAD& create_payload) override
    {
        create_called = true;
    }

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const DELETE_PAYLOAD& create_payload)
    {
        //TODO write tests on this
    }

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const WRITE_DATA_PAYLOAD&  write_payload)
    {
        write_called = true;
    }

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const READ_DATA_PAYLOAD&   read_payload)
    {
        read_called = true;
    }

    bool create_called = false;
    bool write_called = false;
    bool read_called = false;
};
  
class CountListener : public XRCEListener
{
public:
    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const CREATE_PAYLOAD& create_payload) override
    {
        ++creates;
    }

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const DELETE_PAYLOAD& create_payload)
    {
        //TODO write tests on this
    }

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const WRITE_DATA_PAYLOAD&  write_payload)
    {
        ++writes;
    }

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const READ_DATA_PAYLOAD&   read_payload)
    {
        ++reads;
    }

    int creates = 0;
    int writes = 0;
    int reads = 0;
};

protected:
    XRCEParserTests() :
        test_buffer_(new char[BUFFER_LENGTH])
    {
    }

    virtual ~XRCEParserTests() {
        delete[] test_buffer_;
    }
    char* test_buffer_ = nullptr;

    BoolListener bool_listener_;
    CountListener count_listener_;
    
    const uint32_t client_key = 0xF1F2F3F4;
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
};

TEST_F(XRCEParserTests, EmptyMessage)
{
    testing::internal::CaptureStderr();
    XRCEParser myParser{ test_buffer_, BUFFER_LENGTH, &bool_listener_};
    ASSERT_FALSE(myParser.parse());
    ASSERT_EQ("Error submessage ID not recognized\n", testing::internal::GetCapturedStderr());
}

TEST_F(XRCEParserTests, HeaderError)
{
    testing::internal::CaptureStderr();
    XRCEParser myParser{ test_buffer_, 0, &bool_listener_};
    ASSERT_FALSE(myParser.parse());
    ASSERT_EQ("Error reading message header\n", testing::internal::GetCapturedStderr());
}

TEST_F(XRCEParserTests, SubmessageHeaderError)
{
    testing::internal::CaptureStderr();
    MessageHeader message_header;
    XRCEParser myParser{ test_buffer_, message_header.getCdrSerializedSize(message_header),&bool_listener_};
    ASSERT_FALSE(myParser.parse());
    ASSERT_EQ("Error reading submessage header\n", testing::internal::GetCapturedStderr());
}

TEST_F(XRCEParserTests, SubmessagePayloadError)
{
    testing::internal::CaptureStderr();
    MessageHeader message_header;
    SubmessageHeader submessage_header;
    XRCEParser myParser{ test_buffer_, message_header.getCdrSerializedSize(message_header) + submessage_header.getCdrSerializedSize(submessage_header), &bool_listener_};
    ASSERT_FALSE(myParser.parse());
    ASSERT_EQ("Error submessage ID not recognized\n", testing::internal::GetCapturedStderr());
}

TEST_F(XRCEParserTests, CreateMessage)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;;
    serializer_.serialize(message_header);
    CREATE_PAYLOAD createData;
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(CREATE);
    submessage_header.submessage_length(createData.getCdrSerializedSize(createData));
    serializer_.serialize(submessage_header);
    serializer_.serialize(createData);

    XRCEParser myParser{ test_buffer_, serializer_.get_serialized_size(), &bool_listener_ };
    ASSERT_TRUE(myParser.parse());
    ASSERT_TRUE(bool_listener_.create_called);
}

TEST_F(XRCEParserTests, CreateMessageNoCallback)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;;
    serializer_.serialize(message_header);
    CREATE_PAYLOAD createData;
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(CREATE);
    submessage_header.submessage_length(createData.getCdrSerializedSize(createData));
    serializer_.serialize(submessage_header);
    serializer_.serialize(createData);

    XRCEParser myParser{ test_buffer_, serializer_.get_serialized_size(), &bool_listener_};
    testing::internal::CaptureStderr();
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ("Error processing create\n", testing::internal::GetCapturedStderr());	
    ASSERT_FALSE(bool_listener_.create_called);

}

TEST_F(XRCEParserTests, MultiCreateMessage)
{
    const int num_creates = 3;
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;;
    serializer_.serialize(message_header);
    CREATE_PAYLOAD createData;
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(CREATE);
    submessage_header.submessage_length(createData.getCdrSerializedSize(createData));
    for (int i = 0; i < num_creates; ++i)
    {
        serializer_.serialize(submessage_header);
        serializer_.serialize(createData);
    }

    XRCEParser myParser{ test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.creates, num_creates);
}


TEST_F(XRCEParserTests, WriteMessage)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;;
    serializer_.serialize(message_header);
    WRITE_DATA_PAYLOAD write_payload;
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(WRITE_DATA);
    submessage_header.submessage_length(write_payload.getCdrSerializedSize(write_payload));
    serializer_.serialize(submessage_header);
    serializer_.serialize(write_payload);

    XRCEParser myParser{ test_buffer_, serializer_.get_serialized_size(), &bool_listener_};
    ASSERT_TRUE(myParser.parse());
    ASSERT_TRUE(bool_listener_.write_called);
}

TEST_F(XRCEParserTests, WriteMessageNoCallback)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;;
    serializer_.serialize(message_header);
    WRITE_DATA_PAYLOAD write_payload;
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(WRITE_DATA);
    submessage_header.submessage_length(write_payload.getCdrSerializedSize(write_payload));
    serializer_.serialize(submessage_header);
    serializer_.serialize(write_payload);

    XRCEParser myParser{ test_buffer_, serializer_.get_serialized_size(), &bool_listener_};
    testing::internal::CaptureStderr();
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ("Error processing write\n", testing::internal::GetCapturedStderr());
    ASSERT_FALSE(bool_listener_.write_called);
}

TEST_F(XRCEParserTests, MultiWriteMessage)
{
    const int num_writes = 3;
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;;
    serializer_.serialize(message_header);
    WRITE_DATA_PAYLOAD write_payload;
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(WRITE_DATA);
    submessage_header.submessage_length(write_payload.getCdrSerializedSize(write_payload));
    for (int i = 0; i < num_writes; ++i)
    { 
        serializer_.serialize(submessage_header);
        serializer_.serialize(write_payload);
    }

    XRCEParser myParser{ test_buffer_, serializer_.get_serialized_size(), &count_listener_};
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.writes, num_writes);
}

TEST_F(XRCEParserTests, ReadMessage)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;;
    serializer_.serialize(message_header);
    READ_DATA_PAYLOAD read_payload;
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(READ_DATA);
    submessage_header.submessage_length(read_payload.getCdrSerializedSize(read_payload));
    serializer_.serialize(submessage_header);
    serializer_.serialize(read_payload);

    XRCEParser myParser{ test_buffer_, serializer_.get_serialized_size(), &bool_listener_};
    ASSERT_TRUE(myParser.parse());
    ASSERT_TRUE(bool_listener_.read_called);
}

TEST_F(XRCEParserTests, ReadMessageNoCallback)
{
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;;
    serializer_.serialize(message_header);
    READ_DATA_PAYLOAD read_payload;
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(READ_DATA);
    submessage_header.submessage_length(read_payload.getCdrSerializedSize(read_payload));
    serializer_.serialize(submessage_header);
    serializer_.serialize(read_payload);

    XRCEParser myParser{ test_buffer_, serializer_.get_serialized_size(), &bool_listener_};
    testing::internal::CaptureStderr();
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ("Error processing read\n", testing::internal::GetCapturedStderr());
    ASSERT_FALSE(bool_listener_.read_called);
}

TEST_F(XRCEParserTests, MultiSubMessage)
{
    const int num_creates = 3;
    const int num_reads = 3;
    const int num_writes = 3;
    Serializer serializer_(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;
    serializer_.serialize(message_header);

    CREATE_PAYLOAD createData;
    SubmessageHeader create_submessage_header;
    create_submessage_header.submessage_id(CREATE);
    create_submessage_header.submessage_length(createData.getCdrSerializedSize(createData));
    for (int i = 0; i < num_creates; ++i)
    {
        serializer_.serialize(create_submessage_header);
        serializer_.serialize(createData);
    }

    READ_DATA_PAYLOAD read_payload;
    SubmessageHeader read_submessage_header;
    read_submessage_header.submessage_id(READ_DATA);
    read_submessage_header.submessage_length(read_payload.getCdrSerializedSize(read_payload));
    for (int i = 0; i < num_reads; ++i)
    {
        serializer_.serialize(read_submessage_header);
        serializer_.serialize(read_payload);
    }

    WRITE_DATA_PAYLOAD write_payload;
    SubmessageHeader write_submessage_header;
    write_submessage_header.submessage_id(WRITE_DATA);
    write_submessage_header.submessage_length(write_payload.getCdrSerializedSize(write_payload));
    for (int i = 0; i < num_writes; ++i)
    {
        serializer_.serialize(write_submessage_header);
        serializer_.serialize(write_payload);
    }

    XRCEParser myParser{ test_buffer_, serializer_.get_serialized_size(), &count_listener_ };
    ASSERT_TRUE(myParser.parse());
    ASSERT_EQ(count_listener_.creates, num_creates);
    ASSERT_EQ(count_listener_.writes, num_writes);
    ASSERT_EQ(count_listener_.reads, num_reads);
}

class XRCEFileTests : public testing::Test
{
protected:
    XRCEFileTests() :
        test_buffer_(new char[BUFFER_LENGTH])
    {
    }

    virtual ~XRCEFileTests() {
        delete[] test_buffer_;
    }
    char* test_buffer_ = nullptr;

    int creates = 0;
    int writes = 0;
    int reads = 0;
    bool create_called = false;
    bool write_called = false;
    bool read_called = false;
    const uint32_t client_key = 0xF1F2F3F4;
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
};

TEST_F(XRCEFileTests, Simple)
{
    XRCEFactory newMessage{ test_buffer_, BUFFER_LENGTH };
    newMessage.header(client_key, session_id, stream_id, sequence_nr);

    OBJK_DATAWRITER_Status data_writer;
    data_writer.stream_seq_num(0x7FFF);
    data_writer.sample_seq_num(0x1234567890ABCDEF);
    StatusVariant variant;
    variant.data_writer(data_writer);
    ResultStatus result;
    result.request_id({ 0x10, 0x09, 0x08, 0x07 });
    result.status(STATUS_OK);
    result.implementation_status(STATUS_ERR_INCOMPATIBLE);
    Status status;
    status.status(variant);
    status.result(result);
    status.object_id({ 10,20,30 });

    RESOURCE_STATUS_PAYLOAD resource_status;
    resource_status.request_id({ 0x00, 0x01, 0x02, 0x03 });
    resource_status.request_status(status);

    SubmessageHeader submessage_header;
    submessage_header.submessage_id(STATUS);
    submessage_header.flags(0x07);
    submessage_header.submessage_length(static_cast<uint16_t>(resource_status.getCdrSerializedSize(resource_status)));

    newMessage.status(resource_status);

    std::string filename = "test.bin";
    std::fstream s(filename, std::ios::binary | std::ios::trunc | std::ios::in | std::ios::out);
    if (!s.is_open()) {
        std::cout << "failed to open " << filename << '\n';
    }
    else {
        s.write(test_buffer_, newMessage.get_total_size());
        //for fstream, this moves the file position pointer (both put and get)
        s.seekp(0);

        char* read_buffer = new char[newMessage.get_total_size()];
        //read
        s.read(read_buffer, newMessage.get_total_size()); // binary input

        Serializer deserializer_(read_buffer, newMessage.get_total_size());
        MessageHeader deserialized_header;
        SubmessageHeader deserialized_submessage_header;
        RESOURCE_STATUS_PAYLOAD deserialized_status_payload;
        deserializer_.deserialize(deserialized_header);
        deserializer_.deserialize(deserialized_submessage_header);
        deserializer_.deserialize(deserialized_status_payload);

        ASSERT_EQ(client_key, deserialized_header.client_key());
        ASSERT_EQ(session_id, deserialized_header.session_id());
        ASSERT_EQ(stream_id, deserialized_header.stream_id());
        ASSERT_EQ(sequence_nr, deserialized_header.sequence_nr());

        ASSERT_EQ(submessage_header.submessage_id(), deserialized_submessage_header.submessage_id());
        ASSERT_EQ(submessage_header.flags(), deserialized_submessage_header.flags());
        ASSERT_EQ(submessage_header.submessage_length(), deserialized_submessage_header.submessage_length());

        ASSERT_EQ(resource_status.request_id(), deserialized_status_payload.request_id());
        ASSERT_EQ(resource_status.request_status().object_id(), deserialized_status_payload.request_status().object_id());
        ASSERT_EQ(resource_status.request_status().result().request_id(), deserialized_status_payload.request_status().result().request_id());
        ASSERT_EQ(resource_status.request_status().result().status(), deserialized_status_payload.request_status().result().status());
        ASSERT_EQ(resource_status.request_status().result().implementation_status(), deserialized_status_payload.request_status().result().implementation_status());
        ASSERT_EQ(resource_status.request_status().status().data_writer().stream_seq_num(), deserialized_status_payload.request_status().status().data_writer().stream_seq_num());
        ASSERT_EQ(resource_status.request_status().status().data_writer().sample_seq_num(), deserialized_status_payload.request_status().status().data_writer().sample_seq_num());


        delete[] read_buffer;
    }


}
// TODO padding entre un submensage y el siguiente para que el inicio siempre sea multipo de 4
// TODO multiples callbacks para diferentes create?
// 	//getStatus(StatusStruct*, buffer)
// 	//"4321000047150"

// 	test24();

// 	char* buffer = new char[BUFFER_LENGTH];

// 	XRCEFactory newMessage{ buffer, BUFFER_LENGTH };
// 	newMessage.header(0xF1F2F3F4, 0x01, 0x04, 0x0200);
// 	newMessage.data(DATA_PAYLOAD{});
	
// 	//int size = myHeader.getMaxCdrSerializedSize() + subMessage.getMaxCdrSerializedSize();
// 	//int i;
// 	//for (i = 0; i< size ; i++)
// 	//{
// 	//	printf("%02X", static_cast<uint8_t>(buffer[i]));
// 	//}
// 	//std::cout << std::endl;

// 	for (int i = 0; i< myHeader.getMaxCdrSerializedSize() + subMessage.getMaxCdrSerializedSize(); i++)
// 	{
// 		std::cout << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(static_cast<uint8_t>(buffer[i]));
// 	}
// 	// TODO mirar como resetear cout para que 10 no sea a(hex) resetear manipulators
// 	std::cout << 10;