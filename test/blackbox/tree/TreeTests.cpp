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

#include <uxr/agent/Root.hpp>

#include <gtest/gtest.h>

namespace eprosima {
namespace uxr {
namespace testing {

class TreeTests : public ::testing::Test
{
protected:
    TreeTests()
    {
        root_.load_config_file("./agent.refs");
    }

    ~TreeTests()
    {
        root_.reset();
    }

    eprosima::uxr::Root root_;
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
    client_representation.client_key(client_key_);
    client_representation.session_id(0x00);
    dds::xrce::ResultStatus response = root_.create_client(
                client_representation,
                agent_representation,
                Middleware::Kind::FASTDDS);
    std::shared_ptr<ProxyClient> client = root_.get_client(client_representation.client_key());

    /* Common creation mode. */
    dds::xrce::CreationMode creation_mode;
    creation_mode.reuse(false);
    creation_mode.replace(true);

    /*
     * Create participant.
     */
    dds::xrce::ObjectVariant object_variant;
    std::string participant_xml = "<dds>"
                                      "<participant>"
                                          "<rtps>"
                                              "<name>default_xrce_participant</name>"
                                          "</rtps>"
                                      "</participant>"
                                  "</dds>";
    dds::xrce::OBJK_PARTICIPANT_Representation participant_representation;
    participant_representation.domain_id(0);
    participant_representation.representation().xml_string_representation(participant_xml);
    object_variant.participant(participant_representation);

    dds::xrce::ObjectPrefix participant_id = {0x00, 0x01};
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create topic.
     */
    std::string topic_xml = "<dds>"
                                "<topic>"
                                    "<name>HelloWorldTopic</name>"
                                    "<dataType>HelloWorld</dataType>"
                                "</topic>"
                            "</dds>";
    dds::xrce::OBJK_TOPIC_Representation topic_representation;
    topic_representation.participant_id(participant_id);
    topic_representation.representation().xml_string_representation(topic_xml);
    object_variant.topic(topic_representation);

    dds::xrce::ObjectPrefix topic_id = {0x00, 0x22};
    response = client->create_object(creation_mode, topic_id, object_variant);
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
    response = client->create_object(creation_mode, publisher_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datawriter.
     */
    std::string datawriter_xml = "<dds>"
                                     "<data_writer>"
                                         "<topic>"
                                             "<kind>NO_KEY</kind>"
                                             "<name>HelloWorldTopic</name>"
                                             "<dataType>HelloWorld</dataType>"
                                         "</topic>"
                                     "</data_writer>"
                                 "</dds>";
    dds::xrce::DATAWRITER_Representation datawriter_representation;
    datawriter_representation.publisher_id(publisher_id);
    datawriter_representation.representation().xml_string_representation(datawriter_xml);
    object_variant.data_writer(datawriter_representation);

    dds::xrce::ObjectPrefix datawriter_id = {0x00, 0x15};
    response = client->create_object(creation_mode, datawriter_id, object_variant);
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
    response = client->create_object(creation_mode, subscriber_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datareader.
     */
    std::string datareader_xml = "<dds>"
                                     "<data_reader>"
                                         "<topic>"
                                             "<kind>NO_KEY</kind>"
                                             "<name>HelloWorldTopic</name>"
                                             "<dataType>HelloWorld</dataType>"
                                         "</topic>"
                                     "</data_reader>"
                                 "</dds>";
    dds::xrce::DATAREADER_Representation datareader_representation;
    datareader_representation.subscriber_id(subscriber_id);
    datareader_representation.representation().xml_string_representation(datareader_xml);
    object_variant.data_reader(datareader_representation);

    dds::xrce::ObjectPrefix datareader_id = {0x00, 0x16};
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create requester.
     */
    std::string requester_xml = R"(
        <dds>
            <requester profile_name="test_requester_profile"
                       service_name="service_name"
                       request_type="request_type"
                       reply_type="reply_type">
                <publisher>
                    <topic>
                        <name>otherSamplePubSubTopic</name>
                        <dataType>otherSamplePubSubTopicType</dataType>
                    </topic>
                    <qos>
                        <liveliness>
                            <kind>MANUAL_BY_TOPIC</kind>
                        </liveliness>
                    </qos>
                </publisher>
                <subscriber>
                    <qos>
                        <liveliness>
                            <kind>MANUAL_BY_TOPIC</kind>
                        </liveliness>
                    </qos>
                </subscriber>
            </requester>
        </dds>)";
    dds::xrce::REQUESTER_Representation requester_representation;
    requester_representation.participant_id(participant_id);
    requester_representation.representation().xml_string_representation(requester_xml);
    object_variant.requester(requester_representation);

    dds::xrce::ObjectPrefix requester_id = {0x00, 0x17};
    response = client->create_object(creation_mode, requester_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create replier.
     */
    std::string replier_xml = R"(
        <dds>
            <replier profile_name="test_replier_profile"
                     service_name="service_name"
                     request_type="request_type"
                     reply_type="reply_type">
                <publisher>
                    <topic>
                        <name>otherSamplePubSubTopic</name>
                        <dataType>otherSamplePubSubTopicType</dataType>
                    </topic>
                    <qos>
                        <liveliness>
                            <kind>MANUAL_BY_TOPIC</kind>
                        </liveliness>
                    </qos>
                </publisher>
                <subscriber>
                    <qos>
                        <liveliness>
                            <kind>MANUAL_BY_TOPIC</kind>
                        </liveliness>
                    </qos>
                </subscriber>
            </replier>
        </dds>)";
    dds::xrce::REPLIER_Representation replier_representation;
    replier_representation.participant_id(participant_id);
    replier_representation.representation().xml_string_representation(replier_xml);
    object_variant.replier(replier_representation);

    dds::xrce::ObjectPrefix replier_id = {0x00, 0x18};
    response = client->create_object(creation_mode, replier_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());


    /* Participant destruction. */
    response = client->delete_object(participant_id);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
}

TEST_F(TreeTests, REFTree)
{
    /* Create client. */
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::CLIENT_Representation client_representation;
    client_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    client_representation.xrce_version(dds::xrce::XRCE_VERSION);
    client_representation.xrce_vendor_id(vendor_id_);
    client_representation.client_key(client_key_);
    client_representation.session_id(0x00);
    dds::xrce::ResultStatus response = root_.create_client(
                client_representation,
                agent_representation,
                Middleware::Kind::FASTDDS);
    std::shared_ptr<ProxyClient> client = root_.get_client(client_representation.client_key());

    /* Common creation mode. */
    dds::xrce::CreationMode creation_mode;
    creation_mode.reuse(false);
    creation_mode.replace(true);

    /*
     * Create participant.
     */
    dds::xrce::ObjectVariant object_variant;
    std::string participant_ref = "default_xrce_participant";
    dds::xrce::OBJK_PARTICIPANT_Representation participant_representation;
    participant_representation.domain_id(0);
    participant_representation.representation().object_reference(participant_ref);
    object_variant.participant(participant_representation);

    dds::xrce::ObjectPrefix participant_id = {0x00, 0x01};
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create topic.
     */
    std::string topic_ref = "shapetype_topic";
    dds::xrce::OBJK_TOPIC_Representation topic_representation;
    topic_representation.participant_id(participant_id);
    topic_representation.representation().object_reference(topic_ref);
    object_variant.topic(topic_representation);

    dds::xrce::ObjectPrefix topic_id = {0x00, 0x22};
    response = client->create_object(creation_mode, topic_id, object_variant);
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
    response = client->create_object(creation_mode, publisher_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datawriter.
     */
    std::string datawriter_ref = "shapetype_data_writer";
    dds::xrce::DATAWRITER_Representation datawriter_representation;
    datawriter_representation.publisher_id(publisher_id);
    datawriter_representation.representation().object_reference(datawriter_ref);
    object_variant.data_writer(datawriter_representation);

    dds::xrce::ObjectPrefix datawriter_id = {0x00, 0x15};
    response = client->create_object(creation_mode, datawriter_id, object_variant);
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
    response = client->create_object(creation_mode, subscriber_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datareader.
     */
    std::string datareader_ref = "shapetype_data_reader";
    dds::xrce::DATAREADER_Representation datareader_representation;
    datareader_representation.subscriber_id(subscriber_id);
    datareader_representation.representation().object_reference(datareader_ref);
    object_variant.data_reader(datareader_representation);

    dds::xrce::ObjectPrefix datareader_id = {0x00, 0x16};
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create requester.
     */
    std::string requester_ref = "shapetype_requester";
    dds::xrce::REQUESTER_Representation requester_representation;
    requester_representation.participant_id(participant_id);
    requester_representation.representation().object_reference(requester_ref);
    object_variant.requester(requester_representation);

    dds::xrce::ObjectPrefix requester_id = {0x00, 0x17};
    response = client->create_object(creation_mode, requester_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create replier.
     */
    std::string replier_ref = "shapetype_replier";
    dds::xrce::REPLIER_Representation replier_representation;
    replier_representation.participant_id(participant_id);
    replier_representation.representation().object_reference(replier_ref);
    object_variant.replier(replier_representation);

    dds::xrce::ObjectPrefix replier_id = {0x00, 0x18};
    response = client->create_object(creation_mode, replier_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /* Participant destruction. */
    response = client->delete_object(participant_id);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
}

TEST_F(TreeTests, CreationModeXMLTree)
{
    /* Participant XMLs. */
    std::string participant_xml = "<dds>"
                                      "<participant>"
                                          "<domainId>0</domainId>"
                                          "<rtps>"
                                              "<builtin>"
                                                  "<discovery_config>"
                                                      "<leaseDuration>"
                                                          "<sec>DURATION_INFINITY</sec>"
                                                      "</leaseDuration>"
                                                  "</discovery_config>"
                                              "</builtin>"
                                              "<name>default_xrce_participant</name>"
                                          "</rtps>"
                                      "</participant>"
                                  "</dds>";

    std::string participant_xml_two = "<dds>"
                                          "<participant>"
                                              "<domainId>0</domainId>"
                                              "<rtps>"
                                                  "<builtin>"
                                                      "<discovery_config>"
                                                          "<leaseDuration>"
                                                              "<sec>DURATION_INFINITY</sec>"
                                                          "</leaseDuration>"
                                                      "</discovery_config>"
                                                  "</builtin>"
                                                  "<name>default_xrce_participant_two</name>"
                                              "</rtps>"
                                          "</participant>"
                                      "</dds>";

    /* Topic XMLs. */
    std::string shapetype_topic_xml = "<dds>"
                                          "<topic>"
                                              "<kind>WITH_KEY</kind>"
                                              "<name>Square</name>"
                                              "<dataType>ShapeType</dataType>"
                                          "</topic>"
                                      "</dds>";
    std::string helloworld_topic_xml = "<dds>"
                                           "<topic>"
                                               "<name>HelloWorldTopic</name>"
                                               "<dataType>HelloWorld</dataType>"
                                           "</topic>"
                                       "</dds>";

    /* Datawriter XMLs. */
    std::string datawriter_xml = "<dds>"
                                     "<data_writer>"
                                         "<topic>"
                                             "<kind>NO_KEY</kind>"
                                             "<name>HelloWorldTopic</name>"
                                             "<dataType>HelloWorld</dataType>"
                                             "<historyQos>"
                                                 "<kind>KEEP_LAST</kind>"
                                                 "<depth>5</depth>"
                                             "</historyQos>"
                                         "</topic>"
                                         "<qos>"
                                             "<durability>"
                                                 "<kind>TRANSIENT_LOCAL</kind>"
                                             "</durability>"
                                         "</qos>"
                                     "</data_writer>"
                                 "</dds>";
    std::string datawriter_xml_two = "<dds>"
                                         "<data_writer>"
                                             "<topic>"
                                                 "<kind>NO_KEY</kind>"
                                                 "<name>HelloWorldTopic</name>"
                                                 "<dataType>HelloWorld</dataType>"
                                                 "<historyQos>"
                                                     "<kind>KEEP_LAST</kind>"
                                                     "<depth>10</depth>"
                                                 "</historyQos>"
                                             "</topic>"
                                             "<qos>"
                                                 "<durability>"
                                                     "<kind>TRANSIENT_LOCAL</kind>"
                                                 "</durability>"
                                             "</qos>"
                                         "</data_writer>"

                                     "</dds>";
    /* Datareader XMLs. */
    std::string datareader_xml = "<dds>"
                                     "<data_reader>"
                                         "<topic>"
                                             "<kind>NO_KEY</kind>"
                                             "<name>HelloWorldTopic</name>"
                                             "<dataType>HelloWorld</dataType>"
                                             "<historyQos>"
                                                 "<kind>KEEP_LAST</kind>"
                                                 "<depth>5</depth>"
                                             "</historyQos>"
                                         "</topic>"
                                         "<qos>"
                                             "<durability>"
                                                 "<kind>TRANSIENT_LOCAL</kind>"
                                             "</durability>"
                                         "</qos>"
                                     "</data_reader>"
                                 "</dds>";
    std::string datareader_xml_two = "<dds>"
                                         "<data_reader>"
                                             "<topic>"
                                                 "<kind>NO_KEY</kind>"
                                                 "<name>HelloWorldTopic</name>"
                                                 "<dataType>HelloWorld</dataType>"
                                                 "<historyQos>"
                                                     "<kind>KEEP_LAST</kind>"
                                                     "<depth>10</depth>"
                                                 "</historyQos>"
                                             "</topic>"
                                             "<qos>"
                                                 "<durability>"
                                                     "<kind>TRANSIENT_LOCAL</kind>"
                                                 "</durability>"
                                             "</qos>"
                                         "</data_reader>"
                                     "</dds>";

    /* Create client. */
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::CLIENT_Representation client_representation;
    client_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    client_representation.xrce_version(dds::xrce::XRCE_VERSION);
    client_representation.xrce_vendor_id(vendor_id_);
    client_representation.client_key(client_key_);
    client_representation.session_id(0x00);
    dds::xrce::ResultStatus response = root_.create_client(
                client_representation,
                agent_representation,
                eprosima::uxr::Middleware::Kind::FASTDDS);
    std::shared_ptr<ProxyClient> client = root_.get_client(client_representation.client_key());

    /* Common creation mode. */
    dds::xrce::CreationMode creation_mode;

    /*
     * Create participant.
     */
    dds::xrce::ObjectVariant object_variant;
    dds::xrce::OBJK_PARTICIPANT_Representation participant_representation;
    participant_representation.domain_id(0);
    participant_representation.representation().xml_string_representation(participant_xml);
    object_variant.participant(participant_representation);

    dds::xrce::ObjectPrefix participant_id = {0x00, 0x01};

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(participant_id);
    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(participant_id);
    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(participant_id);
    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_ALREADY_EXISTS, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());
    participant_representation.representation().xml_string_representation(participant_xml_two);
    object_variant.participant(participant_representation);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_MISMATCH, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());

    /*
     * Create topic.
     */
    dds::xrce::OBJK_TOPIC_Representation topic_representation;
    topic_representation.participant_id(participant_id);
    topic_representation.representation().xml_string_representation(shapetype_topic_xml);
    object_variant.topic(topic_representation);

    creation_mode.reuse(false);
    creation_mode.replace(false);
    dds::xrce::ObjectPrefix topic_id = {0x00, 0x22};
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(topic_id);
    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(topic_id);
    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(topic_id);
    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_ALREADY_EXISTS, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());
    topic_representation.representation().xml_string_representation(helloworld_topic_xml);
    object_variant.topic(topic_representation);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_MISMATCH, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());

    /*
     * Create publisher.
     */
    std::string publisher_xml = "";
    dds::xrce::OBJK_PUBLISHER_Representation publisher_representation;
    publisher_representation.participant_id(participant_id);
    publisher_representation.representation().string_representation(publisher_xml);
    object_variant.publisher(publisher_representation);

    dds::xrce::ObjectPrefix publisher_id = {0x00, 0x13};
    response = client->create_object(creation_mode, publisher_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datawriter.
     */
    dds::xrce::DATAWRITER_Representation datawriter_representation;
    datawriter_representation.publisher_id(publisher_id);
    datawriter_representation.representation().xml_string_representation(datawriter_xml);
    object_variant.data_writer(datawriter_representation);

    dds::xrce::ObjectPrefix datawriter_id = {0x00, 0x15};
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datawriter_id);
    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datawriter_id);
    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datawriter_id);
    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_ALREADY_EXISTS, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());
    datawriter_representation.representation().xml_string_representation(datawriter_xml_two);
    object_variant.data_writer(datawriter_representation);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_MISMATCH, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());

    /*
     * Create subscriber.
     */
    std::string subscriber_xml = "";
    dds::xrce::OBJK_SUBSCRIBER_Representation subscriber_representation;
    subscriber_representation.participant_id(participant_id);
    subscriber_representation.representation().string_representation(subscriber_xml);
    object_variant.subscriber(subscriber_representation);

    dds::xrce::ObjectPrefix subscriber_id = {0x00, 0x14};
    response = client->create_object(creation_mode, subscriber_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datareader.
     */
    dds::xrce::DATAREADER_Representation datareader_representation;
    datareader_representation.subscriber_id(subscriber_id);
    datareader_representation.representation().xml_string_representation(datareader_xml);
    object_variant.data_reader(datareader_representation);

    dds::xrce::ObjectPrefix datareader_id = {0x00, 0x16};
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datareader_id);
    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datareader_id);
    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datareader_id);
    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_ALREADY_EXISTS, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());
    datareader_representation.representation().xml_string_representation(datareader_xml_two);
    object_variant.data_reader(datareader_representation);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_MISMATCH, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());

    /*
     * Create requester.
     */
    // TODO (julianbermudez)

    /*
     * Create replier.
     */
    // TODO (julianbermudez)
}

TEST_F(TreeTests, CreationModeREFTree)
{
    /* Create client. */
    dds::xrce::AGENT_Representation agent_representation;
    dds::xrce::CLIENT_Representation client_representation;
    client_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    client_representation.xrce_version(dds::xrce::XRCE_VERSION);
    client_representation.xrce_vendor_id(vendor_id_);
    client_representation.client_key(client_key_);
    client_representation.session_id(0x00);
    dds::xrce::ResultStatus response = root_.create_client(
                client_representation,
                agent_representation,
                Middleware::Kind::FASTDDS);
    std::shared_ptr<ProxyClient> client = root_.get_client(client_representation.client_key());

    /* Common creation mode. */
    dds::xrce::CreationMode creation_mode;

    /*
     * Create participant.
     */
    dds::xrce::ObjectVariant object_variant;
    std::string participant_ref = "default_xrce_participant";
    dds::xrce::OBJK_PARTICIPANT_Representation participant_representation;
    participant_representation.domain_id(0);
    participant_representation.representation().object_reference(participant_ref);
    object_variant.participant(participant_representation);

    dds::xrce::ObjectPrefix participant_id = {0x00, 0x01};

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(participant_id);
    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(participant_id);
    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(participant_id);
    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_ALREADY_EXISTS, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());
    participant_ref = "default_xrce_participant_two";
    participant_representation.representation().object_reference(participant_ref);
    object_variant.participant(participant_representation);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_MISMATCH, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
    response = client->create_object(creation_mode, participant_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());

    /*
     * Create topic.
     */
    std::string topic_ref = "helloworld_topic";
    dds::xrce::OBJK_TOPIC_Representation topic_representation;
    topic_representation.participant_id(participant_id);
    topic_representation.representation().object_reference(topic_ref);
    object_variant.topic(topic_representation);

    creation_mode.reuse(false);
    creation_mode.replace(false);
    dds::xrce::ObjectPrefix topic_id = {0x00, 0x22};
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(topic_id);
    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(topic_id);
    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(topic_id);
    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_ALREADY_EXISTS, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());
    topic_ref = "shapetype_topic";
    topic_representation.representation().object_reference(topic_ref);
    object_variant.topic(topic_representation);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_MISMATCH, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
    response = client->create_object(creation_mode, topic_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());

    /*
     * Create publisher.
     */
    std::string publisher_xml = "";
    dds::xrce::OBJK_PUBLISHER_Representation publisher_representation;
    publisher_representation.participant_id(participant_id);
    publisher_representation.representation().string_representation(publisher_xml);
    object_variant.publisher(publisher_representation);

    dds::xrce::ObjectPrefix publisher_id = {0x00, 0x13};
    response = client->create_object(creation_mode, publisher_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datawriter.
     */
    std::string datawriter_ref = "shapetype_data_writer";
    dds::xrce::DATAWRITER_Representation datawriter_representation;
    datawriter_representation.publisher_id(publisher_id);
    datawriter_representation.representation().object_reference(datawriter_ref);
    object_variant.data_writer(datawriter_representation);

    dds::xrce::ObjectPrefix datawriter_id = {0x00, 0x15};
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datawriter_id);
    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datawriter_id);
    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datawriter_id);
    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_ALREADY_EXISTS, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());
    datawriter_ref = "shapetype_data_writer_two";
    datawriter_representation.representation().object_reference(datawriter_ref);
    object_variant.data_writer(datawriter_representation);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_MISMATCH, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
    response = client->create_object(creation_mode, datawriter_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());

    /*
     * Create subscriber.
     */
    std::string subscriber_xml = "";
    dds::xrce::OBJK_SUBSCRIBER_Representation subscriber_representation;
    subscriber_representation.participant_id(participant_id);
    subscriber_representation.representation().string_representation(subscriber_xml);
    object_variant.subscriber(subscriber_representation);

    dds::xrce::ObjectPrefix subscriber_id = {0x00, 0x14};
    response = client->create_object(creation_mode, subscriber_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    /*
     * Create datareader.
     */
    std::string datareader_ref = "shapetype_data_reader";
    dds::xrce::DATAREADER_Representation datareader_representation;
    datareader_representation.subscriber_id(subscriber_id);
    datareader_representation.representation().object_reference(datareader_ref);
    object_variant.data_reader(datareader_representation);

    dds::xrce::ObjectPrefix datareader_id = {0x00, 0x16};
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datareader_id);
    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datareader_id);
    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    response = client->delete_object(datareader_id);
    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_ALREADY_EXISTS, response.status());

    creation_mode.reuse(false);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(false);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());
    datareader_ref = "shapetype_data_reader_two";
    datareader_representation.representation().object_reference(datareader_ref);
    object_variant.data_reader(datareader_representation);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_ERR_MISMATCH, response.status());

    creation_mode.reuse(true);
    creation_mode.replace(true);
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK, response.status());
    response = client->create_object(creation_mode, datareader_id, object_variant);
    ASSERT_EQ(dds::xrce::STATUS_OK_MATCHED, response.status());

    /*
     * Create requester.
     */
    // TODO (julianbermudez)

    /*
     * Create replier.
     */
    // TODO (julianbermudez)
}

} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
