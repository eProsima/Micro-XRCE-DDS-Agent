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
    AgentUnitTests() = default;

    ~AgentUnitTests()
    {
        eprosima::uxr::Agent::reset();
    }

    uint32_t client_key_ = 0xAABBCCDD;
};

TEST_F(AgentUnitTests, CreateClient)
{
    Agent::ErrorCode errcode;
    EXPECT_TRUE(Agent::create_client(client_key_, 0x01, 512, errcode));
    EXPECT_TRUE(Agent::create_client(client_key_, 0x01, 512, errcode));
}

TEST_F(AgentUnitTests, DeleteClient)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

    EXPECT_TRUE(Agent::delete_client(client_key_, errcode));
    EXPECT_FALSE(Agent::delete_client(client_key_, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);
}

TEST_F(AgentUnitTests, CreateParticipantByRef)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

    const char* ref_one = "default_xrce_participant";
    const char* ref_two = "default_xrce_participant_two";

    uint16_t participant_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    int16_t domain_id = 0x00;
    uint8_t flag = 0x00;

    /*
     * Create Participant.
     */
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, errcode));

    /*
     * Create Participant over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create Participant over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_FALSE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::MISMATCH_ERRCODE);

    /*
     * Create Participant over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create Participant over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Delete Participant.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, participant_id, errcode));

    /*
     * Create Participant with invalid REF.
     */
    EXPECT_FALSE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, "error", flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);

    /*
     * Create Participant with invalid ObjectId.
     */
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);
}

TEST_F(AgentUnitTests, CreateParticipantByXml)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

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
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, errcode));

    /*
     * Create Participant over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create Participant over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::MISMATCH_ERRCODE);

    /*
     * Create Participant over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create Participant over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_TRUE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Delete Participant.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, participant_id, errcode));

    /*
     * Create Participant with invalid XML.
     */
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, "error", flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);

    /*
     * Create Participant with invalid ObjectId.
     */
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_participant_by_xml(client_key_, participant_id, domain_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);
}

TEST_F(AgentUnitTests, CreateTopicByRef)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

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
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, errcode));
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, errcode));

    /*
     * Create Topic over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create Topic over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::MISMATCH_ERRCODE);

    /*
     * Create Topic over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create Topic over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_TRUE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Delete Topic.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, topic_id, errcode));

    /*
     * Create Topic with invalid REF.
     */
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, "error", flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);

    /*
     * Create Topic with invalid ObjectId.
     */
    topic_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);

    /*
     * Create Topic with invalid Participant Id.
     */
    topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_topic_by_ref(client_key_, topic_id, participant_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);
}

TEST_F(AgentUnitTests, CreateTopicByXml)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

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
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, errcode));
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, errcode));

    /*
     * Create Topic over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create Topic over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::MISMATCH_ERRCODE);

    /*
     * Create Topic over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create Topic over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_TRUE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Delete Topic.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, topic_id, errcode));

    /*
     * Create Topic with invalid XML.
     */
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, "error", flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);

    /*
     * Create Topic with invalid ObjectId.
     */
    topic_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);

    /*
     * Create Topic with invalid Participant Id.
     */
    topic_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_topic_by_xml(client_key_, topic_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);
}

TEST_F(AgentUnitTests, CreatePublisherByXml)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

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
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, errcode));
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, errcode));

    /*
     * Create Publisher over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create Publisher over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);

    /*
     * Create Publisher over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create Publisher over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);

    /*
     * Delete Publisher.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, publisher_id, errcode));

    /*
     * Create Publisher with invalid ObjectId.
     */
    publisher_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);

    /*
     * Create Publisher with invalid Participant Id.
     */
    publisher_id = Agent::get_object_id(0x00, Agent::PUBLISHER_OBJK);
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);
}

TEST_F(AgentUnitTests, CreateSubscriberByXml)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

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
    EXPECT_TRUE(Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, errcode));
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, errcode));

    /*
     * Create Subscriber over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create Subscriber over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);

    /*
     * Create Subscriber over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create Subscriber over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);

    /*
     * Delete Subscriber.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, subscriber_id, errcode));

    /*
     * Create Subscriber with invalid ObjectId.
     */
    subscriber_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);

    /*
     * Create Subscriber with invalid Participant Id.
     */
    subscriber_id = Agent::get_object_id(0x00, Agent::SUBSCRIBER_OBJK);
    participant_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);
}

TEST_F(AgentUnitTests, CreateDataWriterByRef)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

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
    Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, errcode);
    Agent::create_topic_by_ref(client_key_, topic_id, participant_id, topic_ref, flag, errcode);
    Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, publisher_xml, flag, errcode);
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, errcode));

    /*
     * Create DataWriter over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create DataWriter over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::MISMATCH_ERRCODE);

    /*
     * Create DataWriter over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create DataWriter over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_TRUE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Delete DataWriter.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, datawriter_id, errcode));

    /*
     * Create DataWriter with invalid REF.
     */
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, "error", flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);

    /*
     * Create DataWriter with invalid ObjectId.
     */
    datawriter_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);

    /*
     * Create DataWriter with invalid Participant Id.
     */
    datawriter_id = Agent::get_object_id(0x00, Agent::DATAWRITER_OBJK);
    publisher_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_datawriter_by_ref(client_key_, datawriter_id, publisher_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);
}

TEST_F(AgentUnitTests, CreateDataWriterByXml)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

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
                                      "<durability>"
                                          "<kind>TRANSIENT_LOCAL</kind>"
                                      "</durability>"
                                  "</topic>"
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
                                      "<durability>"
                                          "<kind>TRANSIENT_LOCAL</kind>"
                                      "</durability>"
                                  "</topic>"
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
    Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, errcode);
    Agent::create_topic_by_ref(client_key_, topic_id, participant_id, topic_ref, flag, errcode);
    Agent::create_publisher_by_xml(client_key_, publisher_id, participant_id, publisher_xml, flag, errcode);
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, errcode));

    /*
     * Create DataWriter over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create DataWriter over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::MISMATCH_ERRCODE);

    /*
     * Create DataWriter over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create DataWriter over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_TRUE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Delete DataWriter.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, datawriter_id, errcode));

    /*
     * Create DataWriter with invalid REF.
     */
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, "error", flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);

    /*
     * Create DataWriter with invalid ObjectId.
     */
    datawriter_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);

    /*
     * Create DataWriter with invalid Participant Id.
     */
    datawriter_id = Agent::get_object_id(0x00, Agent::DATAWRITER_OBJK);
    publisher_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_datawriter_by_xml(client_key_, datawriter_id, publisher_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);
}

TEST_F(AgentUnitTests, CreateDataReaderByRef)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

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
    Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, errcode);
    Agent::create_topic_by_ref(client_key_, topic_id, participant_id, topic_ref, flag, errcode);
    Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, subscriber_xml, flag, errcode);
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, errcode));

    /*
     * Create DataReader over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create DataReader over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::MISMATCH_ERRCODE);

    /*
     * Create DataReader over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create DataReader over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_TRUE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Delete DataReader.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, datareader_id, errcode));

    /*
     * Create DataReader with invalid REF.
     */
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, "error", flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);

    /*
     * Create DataReader with invalid ObjectId.
     */
    datareader_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);

    /*
     * Create DataReader with invalid Participant Id.
     */
    datareader_id = Agent::get_object_id(0x00, Agent::DATAREADER_OBJK);
    subscriber_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_datareader_by_ref(client_key_, datareader_id, subscriber_id, ref_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);
}


TEST_F(AgentUnitTests, CreateDataReaderByXml)
{
    Agent::ErrorCode errcode;
    Agent::create_client(client_key_, 0x01, 512, errcode);

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
                                      "<durability>"
                                          "<kind>TRANSIENT_LOCAL</kind>"
                                      "</durability>"
                                  "</topic>"
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
                                      "<durability>"
                                          "<kind>TRANSIENT_LOCAL</kind>"
                                      "</durability>"
                                  "</topic>"
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
    Agent::create_participant_by_ref(client_key_, participant_id, domain_id, participant_ref, flag, errcode);
    Agent::create_topic_by_ref(client_key_, topic_id, participant_id, topic_ref, flag, errcode);
    Agent::create_subscriber_by_xml(client_key_, subscriber_id, participant_id, subscriber_xml, flag, errcode);
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, errcode));

    /*
     * Create DataReader over an existing with 0x00 flag.
     */
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::ALREADY_EXISTS_ERRCODE);

    /*
     * Create DataReader over an existing with REUSE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::MISMATCH_ERRCODE);

    /*
     * Create DataReader over an existing with REPLACE flag.
     */
    flag = Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Create DataReader over an existing with REUSE & REPLACE flag.
     */
    flag = Agent::CreationFlag::REUSE_MODE | Agent::CreationFlag::REPLACE_MODE;
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_two, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_MATCHED_ERRCODE);
    EXPECT_TRUE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::OK_ERRCODE);

    /*
     * Delete DataReader.
     */
    EXPECT_TRUE(Agent::delete_object(client_key_, datareader_id, errcode));

    /*
     * Create DataReader with invalid REF.
     */
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, "error", flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);

    /*
     * Create DataReader with invalid ObjectId.
     */
    datareader_id = Agent::get_object_id(0x00, Agent::PARTICIPANT_OBJK);
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::INVALID_DATA_ERRCODE);

    /*
     * Create DataReader with invalid Participant Id.
     */
    datareader_id = Agent::get_object_id(0x00, Agent::DATAREADER_OBJK);
    subscriber_id = Agent::get_object_id(0x00, Agent::TOPIC_OBJK);
    EXPECT_FALSE(Agent::create_datareader_by_xml(client_key_, datareader_id, subscriber_id, xml_one, flag, errcode));
    EXPECT_EQ(errcode, Agent::ErrorCode::UNKNOWN_REFERENCE_ERRCODE);
}


} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
