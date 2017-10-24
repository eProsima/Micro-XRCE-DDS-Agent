#include "Common.h"

#include <agent/Root.h>
#include <agent/client/ProxyClient.h>

#include <MessageHeader.h>
#include <SubMessageHeader.h>

#include <gtest/gtest.h>

namespace eprosima {
namespace micrortps {
namespace testing {

using namespace eprosima::micrortps;

class AgentTests : public CommonData, public ::testing::Test
{
  protected:
    AgentTests() = default;

    virtual ~AgentTests() = default;

    eprosima::micrortps::Agent agent_;
};

TEST_F(AgentTests, CreateClientOk)
{
    ResultStatus response = agent_.create_client(generate_message_header(), generate_create_payload(OBJECTKIND::CLIENT));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_OK, response.implementation_status());
}

TEST_F(AgentTests, CreateClientNoClient)
{
    ResultStatus response = agent_.create_client(generate_message_header(), generate_create_payload(OBJECTKIND::PUBLISHER));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.implementation_status());
}

TEST_F(AgentTests, CreateClientBadCookie)
{
    CREATE_Payload create_data = generate_create_payload(OBJECTKIND::CLIENT);
    create_data.object_representation().client().xrce_cookie({0x00, 0x00});
    ResultStatus response = agent_.create_client(generate_message_header(), create_data);
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.implementation_status());
}

TEST_F(AgentTests, CreateClientCompatibleVersion)
{
    CREATE_Payload create_data = generate_create_payload(OBJECTKIND::CLIENT);
    create_data.object_representation().client().xrce_version({{XRCE_VERSION_MAJOR, 0x20}});
    ResultStatus response = agent_.create_client(generate_message_header(), create_data);
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_OK, response.implementation_status());
}

TEST_F(AgentTests, CreateClientIncompatibleVersion)
{
    CREATE_Payload create_data = generate_create_payload(OBJECTKIND::CLIENT);
    create_data.object_representation().client().xrce_version({{0x02, XRCE_VERSION_MINOR}});
    ResultStatus response = agent_.create_client(generate_message_header(), create_data);
    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.status());
    ASSERT_EQ(STATUS_ERR_INCOMPATIBLE, response.implementation_status());
}

TEST_F(AgentTests, DeleteExistingClient)
{
    CREATE_Payload create_data = generate_create_payload(OBJECTKIND::CLIENT);
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

    CREATE_Payload create_data = generate_create_payload(OBJECTKIND::CLIENT);
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
    ResultStatus result = client_.create(CreationMode{}, generate_create_payload(OBJECTKIND::SUBSCRIBER));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());
}

TEST_F(ProxyClientTests, CreateSubscriberDuplicated)
{
    ResultStatus result = client_.create(CreationMode{}, generate_create_payload(OBJECTKIND::SUBSCRIBER));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());

    result = client_.create(CreationMode{}, generate_create_payload(OBJECTKIND::SUBSCRIBER));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_ERR_ALREADY_EXISTS, result.implementation_status());
}

TEST_F(ProxyClientTests, CreateSubscriberDuplicatedReplaced)
{
    ResultStatus result = client_.create(CreationMode{}, generate_create_payload(OBJECTKIND::SUBSCRIBER));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());

    CreationMode creation_mode;
    creation_mode.reuse(false);
    creation_mode.replace(true);
    result = client_.create(creation_mode, generate_create_payload(OBJECTKIND::SUBSCRIBER));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());
}

TEST_F(ProxyClientTests, DeleteOnEmpty)
{
    ResultStatus result_status = client_.delete_object(generate_delete_resource_payload(object_id));
    ASSERT_EQ(STATUS_LAST_OP_DELETE, result_status.status());
    ASSERT_EQ(STATUS_ERR_UNKNOWN_REFERENCE, result_status.implementation_status());
}

TEST_F(ProxyClientTests, DeleteWrongId)
{
    ResultStatus result = client_.create(CreationMode{}, generate_create_payload(OBJECTKIND::SUBSCRIBER));
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());

    const ObjectId fake_object_id = {{0xFA, 0xFA}};
    ASSERT_NE(object_id, fake_object_id);

    result = client_.delete_object(generate_delete_resource_payload(fake_object_id));
    ASSERT_EQ(STATUS_LAST_OP_DELETE, result.status());
    ASSERT_EQ(STATUS_ERR_UNKNOWN_REFERENCE, result.implementation_status());
}

TEST_F(ProxyClientTests, DeleteOK)
{
    CREATE_Payload create_data = generate_create_payload(OBJECTKIND::SUBSCRIBER);
    ResultStatus result        = client_.create(CreationMode{}, create_data);
    ASSERT_EQ(STATUS_LAST_OP_CREATE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());

    result = client_.delete_object(generate_delete_resource_payload(create_data.object_id()));
    ASSERT_EQ(request_id, result.request_id());
    ASSERT_EQ(STATUS_LAST_OP_DELETE, result.status());
    ASSERT_EQ(STATUS_OK, result.implementation_status());
}
} // namespace testing
} // namespace micrortps
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}