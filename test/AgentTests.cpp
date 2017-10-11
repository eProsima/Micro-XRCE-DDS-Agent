#include <agent/Root.h>
#include <agent/client/ProxyClient.h>

#include <MessageHeader.h>
#include <SubMessageHeader.h>

#include <gtest/gtest.h>

namespace eprosima{
namespace micrortps{
namespace testing{

using namespace eprosima::micrortps;

class CommonData
{
  protected:
    const ClientKey client_key   = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const XrceVendorId vendor_id = {{0x00, 0x01}};
    const RequestId request_id   = {{1, 2}};
    const ObjectId object_id     = {{10, 20}};
    const uint8_t session_id     = 0x01;
    const uint8_t stream_id      = 0x04;
    const uint16_t sequence_nr   = 0x0001;

    MessageHeader generate_message_header()
    {
        MessageHeader message_header;
        message_header.client_key(client_key);
        message_header.session_id(session_id);
        message_header.stream_id(stream_id);
        message_header.sequence_nr(sequence_nr);
        return message_header;
    }

    CREATE_Payload generate_create_payload(const ObjectKind& object_kind)
    {
        CREATE_Payload create_data;
        create_data.request_id(request_id);
        create_data.object_id(object_id);
        create_data.object_representation(generate_object_variant(object_kind));
        return create_data;
    }

    DELETE_RESOURCE_Payload generate_delete_resource_payload(const ObjectId& obj_id)
    {
        DELETE_RESOURCE_Payload delete_payload;
        delete_payload.object_id(obj_id);
        delete_payload.request_id(request_id);
    }

    OBJK_CLIENT_Representation generate_client_representation()
    {
        OBJK_CLIENT_Representation client_representation;
        client_representation.xrce_cookie(XRCE_COOKIE);
        client_representation.xrce_version(XRCE_VERSION);
        client_representation.xrce_vendor_id(vendor_id);
        client_representation.client_timestamp();
        client_representation.session_id();
        return client_representation;
    }

    OBJK_PUBLISHER_Representation generate_publisher_representation()
    {
        return OBJK_PUBLISHER_Representation{};
    }

    OBJK_SUBSCRIBER_Representation generate_subscriber_representation()
    {
        OBJK_SUBSCRIBER_Representation subscriber_rep;
        subscriber_rep.representation().object_reference("SUBSCRIBER");
        subscriber_rep.participant_id({{4, 4}});
        return subscriber_rep;
    }

    ObjectVariant generate_object_variant(const ObjectKind& object_kind)
    {
        ObjectVariant variant;
        switch(object_kind)
        {
            case OBJK_CLIENT:
            {
                variant.client(generate_client_representation());
                break;
            }
            case OBJK_PUBLISHER:
            {
                variant.publisher(generate_publisher_representation());
                break;
            }
            case OBJK_SUBSCRIBER:
            {
                variant.subscriber(generate_subscriber_representation());
                break;
            }
            case OBJK_INVALID:
            case OBJK_PARTICIPANT:
            case OBJK_TOPIC:
            case OBJK_DATAWRITER:
            case OBJK_DATAREADER:
            case OBJK_TYPE:
            case OBJK_QOSPROFILE:
            case OBJK_APPLICATION:
            default:
                break;
        }
        return variant;
    }
};

class AgentTests : public CommonData, public ::testing::Test
{
  protected:
    AgentTests() = default;

    virtual ~AgentTests() = default;

    eprosima::micrortps::Agent agent_;
};

TEST_F(AgentTests, CreateClientOk)
{
    ResultStatus response = agent_.create_client(generate_message_header(), generate_create_payload(OBJK_CLIENT));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_OK, response.implementation_status());
}

TEST_F(AgentTests, CreateClientNoClient)
{
    ResultStatus response = agent_.create_client(generate_message_header(), generate_create_payload(OBJK_PUBLISHER));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.implementation_status());
}

TEST_F(AgentTests, CreateClientBadCookie)
{
    CREATE_Payload create_data = generate_create_payload(OBJK_CLIENT);
    create_data.object_representation().client().xrce_cookie({0x00, 0x00});
    ResultStatus response = agent_.create_client(generate_message_header(), create_data);
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.implementation_status());
}

TEST_F(AgentTests, CreateClientCompatibleVersion)
{
    CREATE_Payload create_data = generate_create_payload(OBJK_CLIENT);
    create_data.object_representation().client().xrce_version({{XRCE_VERSION_MAJOR, 0x20}});
    ResultStatus response = agent_.create_client(generate_message_header(), create_data);
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_OK, response.implementation_status());
}

TEST_F(AgentTests, CreateClientIncompatibleVersion)
{
    CREATE_Payload create_data = generate_create_payload(OBJK_CLIENT);
    create_data.object_representation().client().xrce_version({{0x02, XRCE_VERSION_MINOR}});
    ResultStatus response = agent_.create_client(generate_message_header(), create_data);
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_ERR_INCOMPATIBLE, response.implementation_status());
}

TEST_F(AgentTests, DeleteExistingClient)
{
    CREATE_Payload create_data = generate_create_payload(OBJK_CLIENT);
    ResultStatus response      = agent_.create_client(generate_message_header(), create_data);
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_OK, response.implementation_status());

    response = agent_.delete_client(client_key, generate_delete_resource_payload(create_data.object_id()));
    ASSERT_EQ(STATUS_LAST_OP_DELETE, response.status());
    ASSERT_EQ(STATUS_OK, response.implementation_status());
}

TEST_F(AgentTests, DeleteOnEmptyAgent)
{
    ResultStatus response = agent_.delete_client(client_key, generate_delete_resource_payload(object_id));
    ASSERT_EQ(STATUS_LAST_OP_DELETE, response.status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.implementation_status());
}

TEST_F(AgentTests, DeleteNoExistingClient)
{
    const ClientKey fake_client_key = {{0xFA, 0xFB, 0xFC, 0xFD}};

    CREATE_Payload create_data = generate_create_payload(OBJK_CLIENT);
    ResultStatus response      = agent_.create_client(generate_message_header(), create_data);
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_OK, response.implementation_status());

    response = agent_.delete_client(fake_client_key, generate_delete_resource_payload(object_id));
    ASSERT_EQ(STATUS_LAST_OP_DELETE, response.status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.implementation_status());
}

class ProxyClientTests : public CommonData, public ::testing::Test
{
  protected:
    ProxyClientTests()          = default;
    virtual ~ProxyClientTests() = default;

    ProxyClient client_;
};

TEST_F(ProxyClientTests, CreateSubscriberOK)
{
    ResultStatus result = client_.create(CreationMode{}, generate_create_payload(OBJK_SUBSCRIBER));
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());
}

TEST_F(ProxyClientTests, CreateSubscriberDuplicated)
{
    ResultStatus result = client_.create(CreationMode{}, generate_create_payload(OBJK_SUBSCRIBER));
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());

    result = client_.create(CreationMode{}, generate_create_payload(OBJK_SUBSCRIBER));
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_ERR_ALREADY_EXISTS, result.implementation_status());
}

TEST_F(ProxyClientTests, CreateSubscriberDuplicatedReplaced)
{
    ResultStatus result = client_.create(CreationMode{}, generate_create_payload(OBJK_SUBSCRIBER));
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());

    CreationMode creation_mode;
    creation_mode.reuse(false);
    creation_mode.replace(true);
    result = client_.create(creation_mode, generate_create_payload(OBJK_SUBSCRIBER));
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());
}

TEST_F(ProxyClientTests, DeleteOnEmpty)
{
    ResultStatus result_status = client_.delete_object(generate_delete_resource_payload(object_id));
    ASSERT_EQ(request_id, result_status.request_id());
    ASSERT_EQ(STATUS_LAST_OP_DELETE, result_status.status());
    ASSERT_EQ(STATUS_ERR_UNKNOWN_REFERENCE, result_status.implementation_status());
}

TEST_F(ProxyClientTests, DeleteWrongId)
{
    ResultStatus result = client_.create(CreationMode{}, generate_create_payload(OBJK_SUBSCRIBER));
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());

    const ObjectId fake_object_id = {{0xFA, 0xFA}};
    ASSERT_NE(object_id, fake_object_id);

    result = client_.delete_object(generate_delete_resource_payload(fake_object_id));
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_DELETE, result.status());
    ASSERT_EQ(STATUS_ERR_UNKNOWN_REFERENCE, result.implementation_status());
}

TEST_F(ProxyClientTests, DeleteOK)
{
    CREATE_Payload create_data = generate_create_payload(OBJK_SUBSCRIBER);
    ResultStatus result        = client_.create(CreationMode{}, create_data);
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());

    result = client_.delete_object(generate_delete_resource_payload(create_data.object_id()));
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_DELETE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());
}
}
}
}

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}