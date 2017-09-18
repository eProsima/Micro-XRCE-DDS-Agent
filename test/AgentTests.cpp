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
    ClientKey client_key = {0xF1, 0xF2, 0xF3, 0xF4};
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
    ClientKey client_key = {0xF1, 0xF2, 0xF3, 0xF4};
    OBJK_PUBLISHER_Representation publisher_representation;

    ObjectVariant variant;
    variant.publisher(publisher_representation);
    Status response = agent_.create_client(client_key, variant);

    ASSERT_EQ(STATUS_LAST_OP_CREATE, response.result().status());
    ASSERT_EQ(STATUS_ERR_INVALID_DATA, response.result().implementation_status());
}

TEST_F(AgentTests, CreateClientBadCookie)
{
    ClientKey client_key = {0xF1, 0xF2, 0xF3, 0xF4};
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
    ClientKey client_key = {0xF1, 0xF2, 0xF3, 0xF4};
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
    ClientKey client_key = {0xF1, 0xF2, 0xF3, 0xF4};
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

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}