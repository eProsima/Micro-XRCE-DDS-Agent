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

#include <micrortps/agent/Root.hpp>
#include <micrortps/agent/client/ProxyClient.hpp>
#include <micrortps/agent/types/MessageHeader.hpp>
#include <micrortps/agent/types/SubMessageHeader.hpp>

#include <gtest/gtest.h>

namespace eprosima {
namespace micrortps {
namespace testing {

class RootUnitTests : public CommonData, public ::testing::Test
{
    protected:
        RootUnitTests() = default;

        virtual ~RootUnitTests() = default;

        eprosima::micrortps::Root root_;
};

TEST_F(RootUnitTests, CreateClientOk)
{
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::ResultStatus response = root_.create_client(generate_create_client_payload().client_representation(),
            agent_representation);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
}

TEST_F(RootUnitTests, CreateClientBadCookie)
{
    dds::xrce::CREATE_CLIENT_Payload create_data = generate_create_client_payload();
    dds::xrce::AGENT_Representation agent_representation;
    create_data.client_representation().xrce_cookie({0x00, 0x00});
    dds::xrce::ResultStatus response = root_.create_client(create_data.client_representation(),
            agent_representation);
    ASSERT_EQ(dds::xrce::STATUS_ERR_INVALID_DATA, response.status());
}

TEST_F(RootUnitTests, CreateClientCompatibleVersion)
{
    dds::xrce::CREATE_CLIENT_Payload create_data = generate_create_client_payload();
    create_data.client_representation().xrce_version({{dds::xrce::XRCE_VERSION_MAJOR, 0x20}});
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::ResultStatus response = root_.create_client(create_data.client_representation(),
            agent_representation);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
}

TEST_F(RootUnitTests, CreateClientIncompatibleVersion)
{
    dds::xrce::CREATE_CLIENT_Payload create_data = generate_create_client_payload();
    create_data.client_representation().xrce_version({{0x02, dds::xrce::XRCE_VERSION_MINOR}});
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::ResultStatus response = root_.create_client(create_data.client_representation(),
            agent_representation);
    ASSERT_EQ(dds::xrce::STATUS_ERR_INCOMPATIBLE, response.status());
}

TEST_F(RootUnitTests, DeleteExistingClient)
{
    dds::xrce::CREATE_CLIENT_Payload create_data = generate_create_client_payload();
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::ResultStatus response = root_.create_client(create_data.client_representation(),
            agent_representation);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = root_.delete_client(client_key);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
}

TEST_F(RootUnitTests, DeleteOnEmptyAgent)
{
    dds::xrce::ResultStatus response = root_.delete_client(client_key);
    ASSERT_EQ(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE, response.status());
}

TEST_F(RootUnitTests, DeleteNoExistingClient)
{
    const dds::xrce::ClientKey fake_client_key = {{0xFA, 0xFB, 0xFC, 0xFD}};

    dds::xrce::CREATE_CLIENT_Payload create_data = generate_create_client_payload();
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::ResultStatus response = root_.create_client(create_data.client_representation(), agent_representation);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = root_.delete_client(fake_client_key);
    ASSERT_EQ(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE, response.status());
}

TEST_F(RootUnitTests, CreateTree)
{
    /* Create client. */
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::CLIENT_Representation client_representaiton = generate_create_client_payload().client_representation();
    dds::xrce::ResultStatus response = root_.create_client(client_representaiton, agent_representation);
    std::shared_ptr<ProxyClient> client = root_.get_client(client_representaiton.client_key());

    /* Common creation mode. */
    dds::xrce::CreationMode creation_mode;
    creation_mode.reuse(false);
    creation_mode.replace(true);

    /*
     * Create participant.
     */
    dds::xrce::ObjectVariant object_variant;
    dds::xrce::OBJK_PARTICIPANT_Representation participant_representation;
    participant_representation.domain_id(0);
    participant_representation.representation().object_reference("default participant");
    object_variant.participant(participant_representation);

    dds::xrce::ObjectPrefix participant_id = {0x00, 0x01};
    response = client->create(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create topic.
     */
    std::string topic_xml = "<profiles><topic><name>HelloWorldTopic</name><dataType>HelloWorld</dataType></topic></profiles>";
    dds::xrce::OBJK_TOPIC_Representation topic_representation;
    topic_representation.participant_id(participant_id);
    topic_representation.representation().xml_string_representation(topic_xml);
    object_variant.topic(topic_representation);

    dds::xrce::ObjectPrefix topic_id = {0x00, 0x22};
    response = client->create(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create publisher.
     */
    std::string publisher_xml = "";
    dds::xrce::OBJK_PUBLISHER_Representation publisher_representation;
    publisher_representation.participant_id(participant_id);
    publisher_representation.representation().string_representation(publisher_xml);
    object_variant.publisher(publisher_representation);

    dds::xrce::ObjectPrefix publisher_id = {0x00, 0x13};
    response = client->create(creation_mode, publisher_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datawriter.
     */
    std::string datawriter_xml = "<profiles><publisher profile_name=\"default_xrce_publisher_profile\"><topic><kind>NO_KEY</kind><name>HelloWorldTopic</name><dataType>HelloWorld</dataType><historyQos><kind>KEEP_LAST</kind><depth>5</depth></historyQos><durability><kind>TRANSIENT_LOCAL</kind></durability></topic></publisher></profiles>";
    dds::xrce::DATAWRITER_Representation datawriter_representation;
    datawriter_representation.publisher_id(publisher_id);
    datawriter_representation.representation().string_representation(datawriter_xml);
    object_variant.data_writer(datawriter_representation);

    dds::xrce::ObjectPrefix datawriter_id = {0x00, 0x15};
    response = client->create(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create subscriber.
     */
    std::string subscriber_xml = "";
    dds::xrce::OBJK_SUBSCRIBER_Representation subscriber_representation;
    subscriber_representation.participant_id(participant_id);
    subscriber_representation.representation().string_representation(subscriber_xml);
    object_variant.subscriber(subscriber_representation);

    dds::xrce::ObjectPrefix subscriber_id = {0x00, 0x14};
    response = client->create(creation_mode, subscriber_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datareader.
     */
    std::string datareader_xml = "<profiles><subscriber profile_name=\"default_xrce_subscriber_profile\"><topic><kind>NO_KEY</kind><name>HelloWorldTopic</name><dataType>HelloWorld</dataType><historyQos><kind>KEEP_LAST</kind><depth>5</depth></historyQos><durability><kind>TRANSIENT_LOCAL</kind></durability></topic></subscriber></profiles>";
    dds::xrce::DATAREADER_Representation datareader_representation;
    datareader_representation.subscriber_id(subscriber_id);
    datareader_representation.representation().string_representation(datareader_xml);
    object_variant.data_reader(datareader_representation);

    dds::xrce::ObjectPrefix datareader_id = {0x00, 0x16};
    response = client->create(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /* Participant destruction. */
    response = client->delete_object(participant_id);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
}

/*
class ProxyClientTests : public CommonData, public ::testing::Test
{
    protected:
        ProxyClientTests()          = default;
        virtual ~ProxyClientTests() = default;

        ProxyClient client_;
};
*/

/* TODO (Julian): participant is need for creating a  subscriber. */
//TEST_F(ProxyClientTests, CreateSubscriberOK)
//{
//    dds::xrce::ResultStatus result = client_.create(dds::xrce::CreationMode{}, generate_create_payload(dds::xrce::OBJK_SUBSCRIBER));
//    ASSERT_EQ(dds::xrce::STATUS_OK, result.status());
//}

/* TODO (Julian): participant is need for creating a  subscriber. */
//TEST_F(ProxyClientTests, CreateSubscriberDuplicated)
//{
//    dds::xrce::ResultStatus result = client_.create(dds::xrce::CreationMode{}, generate_create_payload(dds::xrce::OBJK_SUBSCRIBER));
//    ASSERT_EQ(dds::xrce::STATUS_OK, result.status());
//
//    result = client_.create(dds::xrce::CreationMode{}, generate_create_payload(dds::xrce::OBJK_SUBSCRIBER));
//    ASSERT_EQ(dds::xrce::STATUS_ERR_ALREADY_EXISTS, result.status());
//}

/* TODO (Julian): participant is need for creating a  subscriber. */
//TEST_F(ProxyClientTests, CreateSubscriberDuplicatedReplaced)
//{
//    dds::xrce::ResultStatus result = client_.create(dds::xrce::CreationMode{}, generate_create_payload(dds::xrce::OBJK_SUBSCRIBER));
//    ASSERT_EQ(dds::xrce::STATUS_OK, result.status());
//
//    dds::xrce::CreationMode creation_mode;
//    creation_mode.reuse(false);
//    creation_mode.replace(true);
//    result = client_.create(creation_mode, generate_create_payload(dds::xrce::OBJK_SUBSCRIBER));
//    ASSERT_EQ(dds::xrce::STATUS_OK, result.status());
//}

/*
TEST_F(ProxyClientTests, DeleteOnEmpty)
{
    dds::xrce::ResultStatus result_status = client_.delete_object(object_id);
    ASSERT_EQ(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE, result_status.status());
}
*/

/* TODO (Julian): participant is need for creating a  subscriber. */
//TEST_F(ProxyClientTests, DeleteWrongId)
//{
//    dds::xrce::ResultStatus result = client_.create(dds::xrce::CreationMode{}, generate_create_payload(dds::xrce::OBJK_SUBSCRIBER));
//    ASSERT_EQ(dds::xrce::STATUS_OK, result.status());
//
//    const dds::xrce::ObjectId fake_object_id = {{0xFA, 0xFA}};
//    ASSERT_NE(object_id, fake_object_id);
//
//    result = client_.delete_object(generate_delete_resource_payload(fake_object_id));
//    ASSERT_EQ(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE, result.status());
//}

/* TODO (Julian): participant is need for creating a  subscriber. */
//TEST_F(ProxyClientTests, DeleteOK)
//{
//    dds::xrce::CREATE_Payload create_data = generate_create_payload(dds::xrce::OBJK_SUBSCRIBER);
//    dds::xrce::ResultStatus result        = client_.create(dds::xrce::CreationMode{}, create_data);
//    ASSERT_EQ(dds::xrce::STATUS_OK, result.status());
//
//    result = client_.delete_object(generate_delete_resource_payload(create_data.object_id()));
//    ASSERT_EQ(dds::xrce::STATUS_OK, result.implementation_status());
//}
} // namespace testing
} // namespace micrortps
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
