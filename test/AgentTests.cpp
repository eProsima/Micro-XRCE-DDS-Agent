#include <agent/Root.h>
#include <agent/client/ProxyClient.h>

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
    const int32_t client_key = 0xF1F2F3F4;
    
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie(XRCE_COOKIE);
    client_representation.xrce_version(XRCE_VERSION);
    client_representation.xrce_vendor_id();
    client_representation.client_timestamp();
    client_representation.session_id();
    ObjectVariant variant;
    variant.client(client_representation);
    
    const RequestId request_id = { 1,2 };
    const ObjectId object_id = { 10,20,30 };
    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation().client(client_representation);

    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    Status response = agent_.create_client(message_header, create_data);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_OK, response.result().implementation_status());
}

TEST_F(AgentTests, CreateClientNoClient)
{
    int32_t client_key = 0xF1F2F3F4;
    OBJK_PUBLISHER_Representation publisher_representation;

    ObjectVariant variant;
    variant.publisher(publisher_representation);

    const RequestId request_id = { 1,2 };
    const ObjectId object_id = { 10,20,30 };
    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation().publisher(publisher_representation);

    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    Status response = agent_.create_client(message_header, create_data);

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

    const RequestId request_id = { 1,2 };
    const ObjectId object_id = { 10,20,30 };
    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation().client(client_representation);

    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    Status response = agent_.create_client(message_header, create_data);

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

    const RequestId request_id = { 1,2 };
    const ObjectId object_id = { 10,20,30 };
    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation().client(client_representation);

    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    Status response = agent_.create_client(message_header, create_data);

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

    const RequestId request_id = { 1,2 };
    const ObjectId object_id = { 10,20,30 };
    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation().client(client_representation);

    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    Status response = agent_.create_client(message_header, create_data);

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

    const RequestId request_id = { 1,2 };
    const ObjectId object_id = { 10,20,30 };
    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation().client(client_representation);

    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);

    Status response = agent_.create_client(message_header, create_data);

    DELETE_PAYLOAD delete_payload;
    delete_payload.object_id(object_id);
    delete_payload.request_id({2,2});

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_OK, response.result().implementation_status());

    response = agent_.delete_client(client_key, delete_payload);
    ASSERT_EQ(STATUS_LAST_OP_DELETE, response.result().status());
    ASSERT_EQ(STATUS_OK, response.result().implementation_status());
}

TEST_F(AgentTests, DeleteOnEmptyAgent)
{
    int32_t client_key = 0xF1F2F3F4;
    DELETE_PAYLOAD delete_payload;
    delete_payload.object_id({10,20,30});
    delete_payload.request_id({2,2});
    Status response = agent_.delete_client(client_key, delete_payload);
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

    const RequestId request_id = { 1,2 };
    const ObjectId object_id = { 10,20,30 };
    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation().client(client_representation);

    const uint8_t session_id = 0x01;
    const uint8_t stream_id = 0x04;
    const uint16_t sequence_nr = 0x0200;
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);


    Status response = agent_.create_client(message_header, create_data);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_OK, response.result().implementation_status());

    DELETE_PAYLOAD delete_payload;

    response = agent_.delete_client(0xFAFBFCFD, delete_payload);
    ASSERT_EQ(STATUS_LAST_OP_DELETE, response.result().status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.result().implementation_status());
}


#define BUFFER_LENGTH 1024

class ProxyClientTests : public testing::Test
{
protected:
    ProxyClientTests() = default;
    virtual ~ProxyClientTests() = default;
    
    ProxyClient client_;
};

TEST_F(ProxyClientTests, CreateSubscriberOK)
{
    const RequestId request_id = { 1,2 };
    const ObjectId object_id = { 10,20,30 };
    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id({ 10,20,30 });
    ObjectVariant variant;
    OBJK_SUBSCRIBER_Representation subs;
    subs.as_string(std::string("SUBSCRIBER"));
    subs.participant_id({ 4,4,4 });
    variant.subscriber(subs);
    create_data.object_representation(variant);

    Status result_status = client_.create(CreationMode{}, create_data);
    ASSERT_EQ(object_id, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result_status.result().status());
    ASSERT_EQ(STATUS_OK, result_status.result().implementation_status());
    ASSERT_EQ(request_id, result_status.result().request_id());
}

TEST_F(ProxyClientTests, CreateSubscriberDuplicated)
{
    const RequestId request_id = { 1,2 };
    const ObjectId object_id =  {10,20,30 };
    CreationMode creation_mode;
    creation_mode.reuse(false);
    creation_mode.replace(false);

    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    ObjectVariant variant;
    OBJK_SUBSCRIBER_Representation subs;
    subs.as_string(std::string("SUBSCRIBER"));
    subs.participant_id({ 4,4,4 });
    variant.subscriber(subs);
    create_data.object_representation(variant);

    Status result_status = client_.create(creation_mode, create_data);
    ASSERT_EQ(object_id, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result_status.result().status());
    ASSERT_EQ(STATUS_OK, result_status.result().implementation_status());
    ASSERT_EQ(request_id, result_status.result().request_id());

    const RequestId request_id_2 = { 2,2 };
    create_data.request_id(request_id_2);
    create_data.object_id(object_id);
    subs.as_string(std::string("SUBSCRIBER2"));
    subs.participant_id({ 4,4,4 });
    variant.subscriber(subs);
    create_data.object_representation(variant);

    result_status = client_.create(creation_mode, create_data);
    ASSERT_EQ(object_id, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result_status.result().status());
    ASSERT_EQ(STATUS_ERR_ALREADY_EXISTS, result_status.result().implementation_status());
    ASSERT_EQ(request_id_2, result_status.result().request_id());
}

TEST_F(ProxyClientTests, CreateSubscriberDuplicatedReplaced)
{
    const RequestId request_id = { 1,2 };
    const ObjectId object_id =  {10,20,30 };
    CreationMode creation_mode;
    creation_mode.reuse(false);
    creation_mode.replace(true);

    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    ObjectVariant variant;
    OBJK_SUBSCRIBER_Representation subs;
    subs.as_string(std::string("SUBSCRIBER"));
    subs.participant_id({ 4,4,4 });
    variant.subscriber(subs);
    create_data.object_representation(variant);

    Status result_status = client_.create(creation_mode, create_data);
    ASSERT_EQ(object_id, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result_status.result().status());
    ASSERT_EQ(STATUS_OK, result_status.result().implementation_status());
    ASSERT_EQ(request_id, result_status.result().request_id());

    const RequestId request_id_2 = { 2,2 };
    const ObjectId object_id_2 =  {20,20,30 };
    create_data.request_id(request_id_2);
    create_data.object_id(object_id_2);
    subs.as_string(std::string("SUBSCRIBER2"));
    subs.participant_id({ 4,4,4 });
    variant.subscriber(subs);
    create_data.object_representation(variant);

    result_status = client_.create(creation_mode, create_data);
    ASSERT_EQ(object_id_2, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result_status.result().status());
    ASSERT_EQ(STATUS_OK, result_status.result().implementation_status());
    ASSERT_EQ(request_id_2, result_status.result().request_id());
}

TEST_F(ProxyClientTests, DeleteOnEmpty)
{
    DELETE_PAYLOAD delete_payload;
    const RequestId request_id = { 1,2 };
    const ObjectId object_id =  {10,20,30 };
    delete_payload.request_id(request_id);
    delete_payload.object_id(object_id);

    Status result_status = client_.delete_object(delete_payload);
    ASSERT_EQ(object_id, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_DELETE, result_status.result().status());
    ASSERT_EQ(STATUS_ERR_UNKNOWN_REFERENCE, result_status.result().implementation_status());
    ASSERT_EQ(request_id, result_status.result().request_id());
}

TEST_F(ProxyClientTests, DeleteWrongId)
{
    DELETE_PAYLOAD delete_payload;
    const RequestId request_id = { 1,2 };
    const ObjectId object_id =  {10,20,30 };
    const RequestId create_request_id = { 2,1 };
    const ObjectId create_object_id =  {01,02,03 };
    delete_payload.request_id(request_id);
    delete_payload.object_id(object_id);

    CreationMode creation_mode;
    creation_mode.reuse(false);
    creation_mode.replace(true);

    CREATE_PAYLOAD create_data;
    create_data.request_id(create_request_id);
    create_data.object_id(create_object_id);
    ObjectVariant variant;
    OBJK_SUBSCRIBER_Representation subs;
    subs.as_string(std::string("SUBSCRIBER"));
    subs.participant_id({ 4,4,4 });
    variant.subscriber(subs);
    create_data.object_representation(variant);

    Status result_status = client_.create(creation_mode, create_data);
    ASSERT_EQ(create_object_id, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result_status.result().status());
    ASSERT_EQ(STATUS_OK, result_status.result().implementation_status());
    ASSERT_EQ(create_request_id, result_status.result().request_id());

    result_status = client_.delete_object(delete_payload);
    ASSERT_EQ(object_id, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_DELETE, result_status.result().status());
    ASSERT_EQ(STATUS_ERR_UNKNOWN_REFERENCE, result_status.result().implementation_status());
    ASSERT_EQ(request_id, result_status.result().request_id());
}

TEST_F(ProxyClientTests, DeletePubliserOK)
{
    const RequestId request_id = { 1,2 };
    const ObjectId object_id =  {10,20,30 };
    CreationMode creation_mode;
    creation_mode.reuse(false);
    creation_mode.replace(true);

    CREATE_PAYLOAD create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    ObjectVariant variant;
    OBJK_SUBSCRIBER_Representation subs;
    subs.as_string(std::string("SUBSCRIBER"));
    subs.participant_id({ 4,4,4 });
    variant.subscriber(subs);
    create_data.object_representation(variant);

    Status result_status = client_.create(creation_mode, create_data);
    ASSERT_EQ(object_id, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result_status.result().status());
    ASSERT_EQ(STATUS_OK, result_status.result().implementation_status());
    ASSERT_EQ(request_id, result_status.result().request_id());

    const RequestId request_id_2 = { 2,2 };
    DELETE_PAYLOAD delete_payload;
    delete_payload.request_id(request_id_2);
    delete_payload.object_id(object_id);

    result_status = client_.delete_object(delete_payload);
    ASSERT_EQ(object_id, result_status.object_id());
    ASSERT_EQ(STATUS_LAST_OP_DELETE, result_status.result().status());
    ASSERT_EQ(STATUS_OK, result_status.result().implementation_status());
    ASSERT_EQ(request_id_2, result_status.result().request_id());
}


int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}