#include <agent/Root.h>

#include <agent/MessageHeader.h>
#include <agent/SubMessageHeader.h>

#include <gtest/gtest.h>

using namespace eprosima::micrortps;

class AgentTests : public testing::Test
{
protected:
    AgentTests() = default;

    virtual ~AgentTests() = default;

    eprosima::micrortps::Agent agent_;

};

TEST_F(AgentTests, CreateClientOk)
{
    int32_t client_key = 0xF1F2F3F4;
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie(XRCE_COOKIE);
    client_representation.xrce_version(XRCE_VERSION);
    client_representation.xrce_vendor_id();
    client_representation.client_timestamp();
    client_representation.session_id();

    ObjectVariant variant;
    variant.client(client_representation);
    Status response = agent_.create_client(client_key, variant);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_OK, response.result().implementation_status());
}

TEST_F(AgentTests, CreateClientNoClient)
{
    int32_t client_key = 0xF1F2F3F4;
    OBJK_PUBLISHER_Representation publisher_representation;

    ObjectVariant variant;
    variant.publisher(publisher_representation);
    Status response = agent_.create_client(client_key, variant);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.result().implementation_status());
}

TEST_F(AgentTests, CreateClientBadCookie)
{
    int32_t client_key = 0xF1F2F3F4;
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie();
    client_representation.xrce_version(XRCE_VERSION);
    client_representation.xrce_vendor_id();
    client_representation.client_timestamp();
    client_representation.session_id();

    ObjectVariant variant;
    variant.client(client_representation);
    Status response = agent_.create_client(client_key, variant);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.result().implementation_status());
}

TEST_F(AgentTests, CreateClientCompatibleVersion)
{
    int32_t client_key = 0xF1F2F3F4;
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie(XRCE_COOKIE);
    client_representation.xrce_version({ XRCE_VERSION_MAJOR, 0x20 });
    client_representation.xrce_vendor_id();
    client_representation.client_timestamp();
    client_representation.session_id();

    ObjectVariant variant;
    variant.client(client_representation);
    Status response = agent_.create_client(client_key, variant);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_OK, response.result().implementation_status());
}

TEST_F(AgentTests, CreateClientIncompatibleVersion)
{
    int32_t client_key = 0xF1F2F3F4;
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie(XRCE_COOKIE);
    client_representation.xrce_version({ 0x02, XRCE_VERSION_MINOR });
    client_representation.xrce_vendor_id();
    client_representation.client_timestamp();
    client_representation.session_id();

    ObjectVariant variant;
    variant.client(client_representation);
    Status response = agent_.create_client(client_key, variant);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_ERR_INCOMPATIBLE, response.result().implementation_status());
}

TEST_F(AgentTests, DeleteExistingClient)
{
    int32_t client_key = 0xF1F2F3F4;
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie(XRCE_COOKIE);
    client_representation.xrce_version(XRCE_VERSION);
    client_representation.xrce_vendor_id();
    client_representation.client_timestamp();
    client_representation.session_id();

    ObjectVariant variant;
    variant.client(client_representation);
    Status response = agent_.create_client(client_key, variant);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_OK, response.result().implementation_status());

    response = agent_.delete_client(client_key);
    ASSERT_EQ(STATUS_LAST_OP_DELETE, response.result().status());
    ASSERT_EQ(STATUS_OK, response.result().implementation_status());
}

TEST_F(AgentTests, DeleteOnEmptyAgent)
{
    int32_t client_key = 0xF1F2F3F4;
    Status response = agent_.delete_client(client_key);
    ASSERT_EQ(STATUS_LAST_OP_DELETE, response.result().status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.result().implementation_status());
}

TEST_F(AgentTests, DeleteNoExistingClient)
{
    int32_t client_key = 0xF1F2F3F4;
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie(XRCE_COOKIE);
    client_representation.xrce_version(XRCE_VERSION);
    client_representation.xrce_vendor_id();
    client_representation.client_timestamp();
    client_representation.session_id();

    ObjectVariant variant;
    variant.client(client_representation);
    Status response = agent_.create_client(client_key, variant);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_OK, response.result().implementation_status());

    response = agent_.delete_client(0xFAFBFCFD);
    ASSERT_EQ(STATUS_LAST_OP_DELETE, response.result().status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.result().implementation_status());
}


#define BUFFER_LENGTH 1024

class ProxyClientTests : public testing::Test
{
protected:
    ProxyClientTests()
    : test_buffer_(new char[BUFFER_LENGTH])
    {
        OBJK_CLIENT_Representation client_representation;
        client_representation.xrce_cookie(XRCE_COOKIE);
        client_representation.xrce_version(XRCE_VERSION);
        client_representation.xrce_vendor_id();
        client_representation.client_timestamp();
        client_representation.session_id();
    
        ObjectVariant variant;
        variant.client(client_representation);
        Status response = agent_.create_client(client_key, variant);
    }

    virtual ~ProxyClientTests()
    {
        delete[] test_buffer_;
    } 

    char* test_buffer_ = nullptr;
    eprosima::micrortps::Agent agent_;
    const int32_t client_key = 0xF1F2F3F4;
    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;

    const subMessageId submessage_id = CREATE;
    const uint8_t flags = 0x07;

};

TEST_F(ProxyClientTests, CreatePublisher)
{
    Serializer serializer(test_buffer_, BUFFER_LENGTH);
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    CREATE_PAYLOAD create_payload;
    create_payload.request_id({ 1,2 });
    create_payload.object_id({ 10,20,30 });
    ObjectVariant variant;
    OBJK_PUBLISHER_Representation pubRep;
    pubRep.as_string(std::string("PUBLISHER"));
    pubRep.participant_id({ 4,4,4 });
    variant.publisher(pubRep);
    create_payload.object_representation(variant);

    SubmessageHeader sub_header;
    sub_header.submessage_id(submessage_id); // STATUS
    sub_header.flags(flags);
    sub_header.submessage_length(create_payload.getCdrSerializedSize(create_payload));
    serializer.serialize(sub_header);
    serializer.serialize(create_payload);

    agent_.on_message(message_header, sub_header, create_payload);

    
}

TEST_F(ProxyClientTests, CreateSubscriber)
{

}

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}