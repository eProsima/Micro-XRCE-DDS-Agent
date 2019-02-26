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

} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
