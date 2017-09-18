#include <agent/Root.h>

#include <gtest/gtest.h>

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

class ProxyClientTests : public testing::Test
{
protected:
    ProxyClientTests() = default;

    virtual ~ProxyClientTests() = default;
};

TEST_F(ProxyClientTests, a)
{

}

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}