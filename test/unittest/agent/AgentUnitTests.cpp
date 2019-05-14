// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/Agent.hpp>

#include <gtest/gtest.h>

namespace eprosima {
namespace uxr {
namespace testing {

using namespace eprosima::uxr;

class AgentUnitTests : public ::testing::Test
{
protected:
    AgentUnitTests()
    {
        eprosima::uxr::Agent::load_config_file("./agent.refs");
    }

    ~AgentUnitTests()
    {
        eprosima::uxr::Agent::reset();
    }

    const uint32_t client_key_ = 0xAABBCCDD;
};

TEST_F(AgentUnitTests, CreateClient)
{
    Agent::OpResult result;
    EXPECT_TRUE(Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result));
    EXPECT_TRUE(Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result));
}

TEST_F(AgentUnitTests, DeleteClient)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    EXPECT_TRUE(Agent::delete_client(client_key_, result));
    EXPECT_FALSE(Agent::delete_client(client_key_, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);
}

TEST_F(AgentUnitTests, CreateParticipantByRef)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* ref_one = "default_xrce_participant";
    const char* ref_two = "default_xrce_participant_two";

    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    int16_t domain_id = 0x00;
    uint8_t flag = 0x00;

    /*
     * Create Participant.
     */
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, result));

    /*
     * Create Participant over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create Participant over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_FALSE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::MISMATCH_ERROR);

    /*
     * Create Participant over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create Participant over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Delete Participant.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, participant_id, result));

    /*
     * Create Participant with invalid REF.
     */
    EXPECT_FALSE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, "error", flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);

    /*
     * Create Participant with invalid ObjectId.
     */
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);
}

TEST_F(AgentUnitTests, CreateParticipantByXml)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* xml_one = "<dds>"
                              "<participant>"
                                  "<rtps>"
                                      "<name>default_xrce_participant_one</name>"
                                  "</rtps>"
                              "</participant>"
                          "</dds>";
    const char* xml_two = "<dds>"
                              "<participant>"
                                  "<rtps>"
                                      "<name>default_xrce_participant_two</name>"
                                  "</rtps>"
                              "</participant>"
                          "</dds>";

    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    int16_t domain_id = 0x00;
    uint8_t flag = 0x00;

    /*
     * Create Participant.
     */
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, result));

    /*
     * Create Participant over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create Participant over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::MISMATCH_ERROR);

    /*
     * Create Participant over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create Participant over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Delete Participant.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, participant_id, result));

    /*
     * Create Participant with invalid XML.
     */
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, "error", flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);

    /*
     * Create Participant with invalid ObjectId.
     */
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);
}

TEST_F(AgentUnitTests, CreateTopicByRef)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* participant_ref = "default_xrce_participant";
    const char* ref_one = "shapetype_topic";
    const char* ref_two = "helloworld_topic";

    uint16_t topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    int16_t domain_id = 0x00;
    uint8_t flag = 0x00;

    /*
     * Create Topic.
     */
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, result));
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, result));

    /*
     * Create Topic over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create Topic over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::MISMATCH_ERROR);

    /*
     * Create Topic over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create Topic over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Delete Topic.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, topic_id, result));

    /*
     * Create Topic with invalid REF.
     */
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, "error", flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);

    /*
     * Create Topic with invalid ObjectId.
     */
    topic_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);

    /*
     * Create Topic with invalid Participant Id.
     */
    topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);
}

TEST_F(AgentUnitTests, CreateTopicByXml)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* participant_ref = "default_xrce_participant";
    const char* xml_one = "<dds>"
                              "<topic>"
                                  "<kind>WITH_KEY</kind>"
                                  "<name>Square</name>"
                                  "<dataType>ShapeType</dataType>"
                              "</topic>"
                          "</dds>";
    const char* xml_two = "<dds>"
                              "<topic>"
                                  "<name>HelloWorldTopic</name>"
                                  "<dataType>HelloWorld</dataType>"
                              "</topic>"
                          "</dds>";

    uint16_t topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    int16_t domain_id = 0x00;
    uint8_t flag = 0x00;

    /*
     * Create Topic.
     */
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, result));
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, result));

    /*
     * Create Topic over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create Topic over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::MISMATCH_ERROR);

    /*
     * Create Topic over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create Topic over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Delete Topic.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, topic_id, result));

    /*
     * Create Topic with invalid XML.
     */
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, "error", flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);

    /*
     * Create Topic with invalid ObjectId.
     */
    topic_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);

    /*
     * Create Topic with invalid Participant Id.
     */
    topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);
}

TEST_F(AgentUnitTests, CreatePublisherByXml)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* participant_ref = "default_xrce_participant";
    const char* xml_one = "publisher_one";
    const char* xml_two = "publisher_two";

    uint16_t publisher_id = Agent::get_object_id(0x00, Agent::PUBLISHER_OBJK);
    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    int16_t domain_id = 0x00;
    uint8_t flag = 0x00;

    /*
     * Create Publisher.
     */
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, result));
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, result));

    /*
     * Create Publisher over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create Publisher over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);

    /*
     * Create Publisher over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create Publisher over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);

    /*
     * Delete Publisher.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, publisher_id, result));

    /*
     * Create Publisher with invalid ObjectId.
     */
    publisher_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);

    /*
     * Create Publisher with invalid Participant Id.
     */
    publisher_id = Agent::get_object_id(0x00, Agent::PUBLISHER_OBJK);
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);
}

TEST_F(AgentUnitTests, CreateSubscriberByXml)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* participant_ref = "default_xrce_participant";
    const char* xml_one = "subscriber_one";
    const char* xml_two = "subscriber_two";

    uint16_t subscriber_id = Agent::get_object_id(0x00, Agent::SUBSCRIBER_OBJK);
    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    int16_t domain_id = 0x00;
    uint8_t flag = 0x00;

    /*
     * Create Subscriber.
     */
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, result));
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, result));

    /*
     * Create Subscriber over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create Subscriber over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);

    /*
     * Create Subscriber over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create Subscriber over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);

    /*
     * Delete Subscriber.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, subscriber_id, result));

    /*
     * Create Subscriber with invalid ObjectId.
     */
    subscriber_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);

    /*
     * Create Subscriber with invalid Participant Id.
     */
    subscriber_id = Agent::get_object_id(0x00, Agent::SUBSCRIBER_OBJK);
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);
}

TEST_F(AgentUnitTests, CreateDataWriterByRef)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* participant_ref = "default_xrce_participant";
    const char* topic_ref = "shapetype_topic";
    const char* publisher_xml = "publisher";
    const char* ref_one = "shapetype_data_writer";
    const char* ref_two = "shapetype_data_writer_two";

    int16_t domain_id = 0x00;
    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    uint16_t topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    uint16_t publisher_id = Agent::get_object_id(0x00, Agent::PUBLISHER_OBJK);
    uint16_t datawriter_id = Agent::get_object_id(0x00, Agent::DATAWRITER_OBJK);

    uint8_t flag = 0x00;

    /*
     * Create DataWriter.
     */
    Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, result);
    Agent::create_topic_by_ref(client_key_, topic_id, participant_id, topic_ref, flag, result);
    Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, publisher_xml, flag, result);
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, result));

    /*
     * Create DataWriter over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create DataWriter over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::MISMATCH_ERROR);

    /*
     * Create DataWriter over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create DataWriter over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Delete DataWriter.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, datawriter_id, result));

    /*
     * Create DataWriter with invalid REF.
     */
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, "error", flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);

    /*
     * Create DataWriter with invalid ObjectId.
     */
    datawriter_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);

    /*
     * Create DataWriter with invalid Participant Id.
     */
    datawriter_id = Agent::get_object_id(0x00, Agent::DATAWRITER_OBJK);
    publisher_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);
}

TEST_F(AgentUnitTests, CreateDataWriterByXml)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* participant_ref = "default_xrce_participant";
    const char* topic_ref = "helloworld_topic";
    const char* publisher_xml = "publisher";
    const char* xml_one = "<dds>"
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
    const char* xml_two = "<dds>"
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

    int16_t domain_id = 0x00;
    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    uint16_t topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    uint16_t publisher_id = Agent::get_object_id(0x00, Agent::PUBLISHER_OBJK);
    uint16_t datawriter_id = Agent::get_object_id(0x00, Agent::DATAWRITER_OBJK);

    uint8_t flag = 0x00;

    /*
     * Create DataWriter.
     */
    Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, result);
    Agent::create_topic_by_ref(client_key_, topic_id, participant_id, topic_ref, flag, result);
    Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, publisher_xml, flag, result);
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, result));

    /*
     * Create DataWriter over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create DataWriter over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::MISMATCH_ERROR);

    /*
     * Create DataWriter over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create DataWriter over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Delete DataWriter.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, datawriter_id, result));

    /*
     * Create DataWriter with invalid REF.
     */
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, "error", flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);

    /*
     * Create DataWriter with invalid ObjectId.
     */
    datawriter_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);

    /*
     * Create DataWriter with invalid Participant Id.
     */
    datawriter_id = Agent::get_object_id(0x00, Agent::DATAWRITER_OBJK);
    publisher_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);
}

TEST_F(AgentUnitTests, CreateDataReaderByRef)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* participant_ref = "default_xrce_participant";
    const char* topic_ref = "shapetype_topic";
    const char* subscriber_xml = "subscriber";
    const char* ref_one = "shapetype_data_reader";
    const char* ref_two = "shapetype_data_reader_two";

    int16_t domain_id = 0x00;
    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    uint16_t topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    uint16_t subscriber_id = Agent::get_object_id(0x00, Agent::SUBSCRIBER_OBJK);
    uint16_t datareader_id = Agent::get_object_id(0x00, Agent::DATAREADER_OBJK);

    uint8_t flag = 0x00;

    /*
     * Create DataReader.
     */
    Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, result);
    Agent::create_topic_by_ref(client_key_, topic_id, participant_id, topic_ref, flag, result);
    Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, subscriber_xml, flag, result);
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, result));

    /*
     * Create DataReader over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create DataReader over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::MISMATCH_ERROR);

    /*
     * Create DataReader over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create DataReader over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Delete DataReader.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, datareader_id, result));

    /*
     * Create DataReader with invalid REF.
     */
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, "error", flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);

    /*
     * Create DataReader with invalid ObjectId.
     */
    datareader_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);

    /*
     * Create DataReader with invalid Participant Id.
     */
    datareader_id = Agent::get_object_id(0x00, Agent::DATAREADER_OBJK);
    subscriber_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);
}


TEST_F(AgentUnitTests, CreateDataReaderByXml)
{
    Agent::OpResult result;
    Agent::create_client(client_key_, 0x01, 512, Middleware::Kind::FAST, result);

    const char* participant_ref = "default_xrce_participant";
    const char* topic_ref = "helloworld_topic";
    const char* subscriber_xml = "subscriber";
    const char* xml_one = "<dds>"
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
    const char* xml_two = "<dds>"
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

    int16_t domain_id = 0x00;
    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    uint16_t topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    uint16_t subscriber_id = Agent::get_object_id(0x00, Agent::SUBSCRIBER_OBJK);
    uint16_t datareader_id = Agent::get_object_id(0x00, Agent::DATAREADER_OBJK);

    uint8_t flag = 0x00;

    /*
     * Create DataReader.
     */
    Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, result);
    Agent::create_topic_by_ref(client_key_, topic_id, participant_id, topic_ref, flag, result);
    Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, subscriber_xml, flag, result);
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, result));

    /*
     * Create DataReader over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::ALREADY_EXISTS_ERROR);

    /*
     * Create DataReader over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::MISMATCH_ERROR);

    /*
     * Create DataReader over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Create DataReader over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_two, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK_MATCHED);
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::OK);

    /*
     * Delete DataReader.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, datareader_id, result));

    /*
     * Create DataReader with invalid REF.
     */
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, "error", flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);

    /*
     * Create DataReader with invalid ObjectId.
     */
    datareader_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::INVALID_DATA_ERROR);

    /*
     * Create DataReader with invalid Participant Id.
     */
    datareader_id = Agent::get_object_id(0x00, Agent::DATAREADER_OBJK);
    subscriber_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, result));
    EXPECT_EQ(result, Agent::OpResult::UNKNOWN_REFERENCE_ERROR);
}


} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
