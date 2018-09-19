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

#include <micrortps/agent/Root.hpp>

#include <gtest/gtest.h>

namespace eprosima {
namespace micrortps {
namespace testing {

class TreeTests : public ::testing::Test
{
protected:
    TreeTests() = default;

    virtual ~TreeTests() = default;

    eprosima::micrortps::Root root_;
    const dds::xrce::ClientKey client_key_      = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const dds::xrce::XrceVendorId vendor_id_    = {{0x00, 0x01}};
};

TEST_F(TreeTests, XMLTree)
{
    /* Create client. */
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::CLIENT_Representation client_representation;
    client_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    client_representation.xrce_version(dds::xrce::XRCE_VERSION);
    client_representation.xrce_vendor_id(vendor_id_);
    client_representation.client_timestamp().seconds(0x00);
    client_representation.client_timestamp().nanoseconds(0x00);
    client_representation.client_key(client_key_);
    client_representation.session_id(0x00);
    dds::xrce::ResultStatus response = root_.create_client(client_representation, agent_representation);
    std::shared_ptr<ProxyClient> client = root_.get_client(client_representation.client_key());

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
    // TODO (julian): change when
    participant_representation.representation().object_reference("default participant");
    object_variant.participant(participant_representation);

    dds::xrce::ObjectPrefix participant_id = {0x00, 0x01};
    response = client->create(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create topic.
     */
    std::string topic_xml = "<dds><topic><name>HelloWorldTopic</name><dataType>HelloWorld</dataType></topic></dds>";
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

} // namespace testing
} // namespace micrortps
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
