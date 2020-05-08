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

#include <uxr/agent/middleware/ced/CedMiddleware.hpp>

#include <gtest/gtest.h>

namespace eprosima {
namespace uxr {
namespace testing {

class CedMiddlewareUnitTests : public ::testing::Test
{
public:
    CedMiddlewareUnitTests()
        : middleware_(0xAABBCCDD)
    {}

    ~CedMiddlewareUnitTests() = default;

protected:
    CedMiddleware middleware_;
};

TEST_F(CedMiddlewareUnitTests, CreateParticipantByRef)
{
    std::string ref_one{"ref_one"};
    std::string ref_two{"ref_two"};

    /* Create Participant by REF. */
    EXPECT_TRUE(middleware_.create_participant_by_ref(0, 0, ref_one));

    /** Create:
     *      Id:         same
     *      Domain:     same
     *      Reference:  same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_participant_by_ref(0, 0, ref_one));

    /** Create:
     *      Id:         different
     *      Domain:     same
     *      Reference:  same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_participant_by_ref(1, 0, ref_one));

    /** Create:
     *      Id:         same
     *      Domain:     different
     *      Reference:  same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_participant_by_ref(0, 1, ref_one));

    /** Create:
     *      Id:         same
     *      Domain:     same
     *      Reference:  different
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_participant_by_ref(0, 0, ref_two));
}

TEST_F(CedMiddlewareUnitTests, CreateParticipantByXml)
{
    std::string xml_one{"xml_one"};
    std::string xml_two{"xml_two"};

    /* Create Participant by REF. */
    EXPECT_TRUE(middleware_.create_participant_by_xml(0, 0, xml_one));

    /* Create:
     *      Id:         same
     *      Domain:     same
     *      XML:        same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_participant_by_xml(0, 0, xml_one));

    /* Create:
     *      Id:         different
     *      Domain:     same
     *      XML:        same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_participant_by_xml(1, 0, xml_one));

    /* Create:
     *      Id:         same
     *      Domain:     different
     *      XML:        same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_participant_by_xml(0, 1, xml_one));

    /* Create:
     *      Id:         same
     *      Domain:     same
     *      XML:        different
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_participant_by_xml(0, 0, xml_two));
}

TEST_F(CedMiddlewareUnitTests, DeleteParticipant)
{
    std::string ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, ref);

    /* Delete
     *      Participant:    existent
     *      Expected:       TRUE
     */
    EXPECT_TRUE(middleware_.delete_participant(0));

    /* Delete
     *      Participant:    non-existent
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.delete_participant(0));

    /* Create
     *      Participant:    same
     *      Expected:       TRUE
     */
    EXPECT_TRUE(middleware_.create_participant_by_ref(0, 0, ref));
}

TEST_F(CedMiddlewareUnitTests, CreateTopicByRef)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);
    middleware_.create_participant_by_ref(1, 0, participant_ref);

    std::string topic_ref_one{"TopicOne"};
    std::string topic_ref_two{"TopicTwo"};

    /* Create Topic by REF. */
    EXPECT_TRUE(middleware_.create_topic_by_ref(0, 0, topic_ref_one));

    /* Create:
     *      Id:             same
     *      Participant:    same
     *      Reference:      same
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_topic_by_ref(0, 0, topic_ref_one));

    /* Create:
     *      Id:             different
     *      Participant:    same
     *      Reference:      same
     *      Expected:       TRUE
     */
    EXPECT_TRUE(middleware_.create_topic_by_ref(1, 0, topic_ref_one));

    /* Create:
     *      Id:             same
     *      Participant:    different
     *      Reference:      same
     *      Expected:       TRUE
     */
    EXPECT_FALSE(middleware_.create_topic_by_ref(0, 1, topic_ref_one));

    /* Create:
     *      Id:             same
     *      Participant:    same
     *      Reference:      different
     *      Expected:       TRUE
     */
    EXPECT_FALSE(middleware_.create_topic_by_ref(0, 0, topic_ref_two));

    /* Create:
     *      Id:             same
     *      Participant:    non-existent
     *      Reference:      same
     *      Expected:       TRUE
     */
    EXPECT_FALSE(middleware_.create_topic_by_ref(0, 2, topic_ref_one));
}

TEST_F(CedMiddlewareUnitTests, CreateTopicByXML)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);
    middleware_.create_participant_by_ref(1, 0, participant_ref);

    std::string topic_xml_one{"TopicOne"};
    std::string topic_xml_two{"TopicTwo"};

    /* Create Topic by XML. */
    EXPECT_TRUE(middleware_.create_topic_by_xml(0, 0, topic_xml_one));

    /* Create:
     *      Id:             same
     *      Participant:    same
     *      XML:            same
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_topic_by_xml(0, 0, topic_xml_one));

    /* Create:
     *      Id:             different
     *      Participant:    same
     *      XML:            same
     *      Expected:       TRUE
     */
    EXPECT_TRUE(middleware_.create_topic_by_xml(1, 0, topic_xml_one));

    /* Create:
     *      Id:             same
     *      Participant:    different
     *      XML:            same
     *      Expected:       TRUE
     */
    EXPECT_FALSE(middleware_.create_topic_by_xml(0, 1, topic_xml_one));

    /* Create:
     *      Id:             same
     *      Participant:    same
     *      XML:            different
     *      Expected:       TRUE
     */
    EXPECT_FALSE(middleware_.create_topic_by_xml(0, 0, topic_xml_two));

    /* Create:
     *      Id:             same
     *      Participant:    non-existent
     *      XML:            same
     *      Expected:       TRUE
     */
    EXPECT_FALSE(middleware_.create_topic_by_xml(0, 2, topic_xml_one));
}

TEST_F(CedMiddlewareUnitTests, DeleteTopic)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string topic_ref{"Topic"};
    middleware_.create_topic_by_ref(0, 0, topic_ref);

    /* Delete
     *      Topic:      existent
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.delete_topic(0));

    /* Delete
     *      Topic:      non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.delete_topic(0));

    /* Create
     *      Topic:      same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_topic_by_ref(0, 0, topic_ref));
}

TEST_F(CedMiddlewareUnitTests, CreatePublisherByXML)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);
    middleware_.create_participant_by_ref(1, 0, participant_ref);

    std::string publisher_xml_one{"PublisherOne"};
    std::string publisher_xml_two{"PublisherTwo"};

    /* Create Publisher by XML. */
    EXPECT_TRUE(middleware_.create_publisher_by_xml(0, 0, publisher_xml_one));

    /* Create:
     *      Id:             same
     *      Participant:    same
     *      XML:            same
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_publisher_by_xml(0, 0, publisher_xml_one));

    /* Create:
     *      Id:             different
     *      Participant:    same
     *      XML:            same
     *      Expected:       TRUE
     */
    EXPECT_TRUE(middleware_.create_publisher_by_xml(1, 0, publisher_xml_one));

    /* Create:
     *      Id:             same
     *      Participant:    different
     *      XML:            same
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_publisher_by_xml(0, 1, publisher_xml_one));

    /* Create:
     *      Id:             same
     *      Participant:    same
     *      XML:            different
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_publisher_by_xml(0, 0, publisher_xml_two));

    /* Create:
     *      Id:             same
     *      Participant:    non-existent
     *      XML:            same
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_publisher_by_xml(0, 2, publisher_xml_one));
}

TEST_F(CedMiddlewareUnitTests, DeletePublisher)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string publisher_xml{"Publisher"};
    middleware_.create_publisher_by_xml(0, 0, publisher_xml);

    /* Delete
     *      Publisher:  existent
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.delete_publisher(0));

    /* Delete
     *      Publisher:  non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.delete_topic(0));

    /* Create
     *      Publisher:  same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_publisher_by_xml(0, 0, publisher_xml));
}

TEST_F(CedMiddlewareUnitTests, CreateSubscriberByXML)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);
    middleware_.create_participant_by_ref(1, 0, participant_ref);

    std::string subscriber_xml_one{"SubscriberOne"};
    std::string subscriber_xml_two{"SubscriberTwo"};

    /* Create Publisher by REF. */
    EXPECT_TRUE(middleware_.create_subscriber_by_xml(0, 0, subscriber_xml_one));

    /* Create:
     *      Id:             same
     *      Participant:    same
     *      XML:            same
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_subscriber_by_xml(0, 0, subscriber_xml_one));

    /* Create:
     *      Id:             different
     *      Participant:    same
     *      XML:            same
     *      Expected:       TRUE
     */
    EXPECT_TRUE(middleware_.create_subscriber_by_xml(1, 0, subscriber_xml_one));

    /* Create:
     *      Id:             same
     *      Participant:    different
     *      XML:            same
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_subscriber_by_xml(0, 1, subscriber_xml_one));

    /* Create:
     *      Id:             same
     *      Participant:    same
     *      XML:            different
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_subscriber_by_xml(0, 0, subscriber_xml_two));

    /* Create:
     *      Id:             same
     *      Participant:    non-existent
     *      XML:            same
     *      Expected:       FALSE
     */
    EXPECT_FALSE(middleware_.create_subscriber_by_xml(0, 2, subscriber_xml_one));
}

TEST_F(CedMiddlewareUnitTests, DeleteSubscriber)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string subscriber_xml{"Publisher"};
    middleware_.create_subscriber_by_xml(0, 0, subscriber_xml);

    /* Delete
     *      Subscriber: existent
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.delete_subscriber(0));

    /* Delete
     *      Subscriber: non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.delete_subscriber(0));

    /* Create
     *      Subscriber: same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_subscriber_by_xml(0, 0, subscriber_xml));
}

TEST_F(CedMiddlewareUnitTests, CreateDataWriterByRef)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string topic_ref_one{"TopicOne"};
    std::string topic_ref_two{"TopicTwo"};
    uint16_t topic_id_one = 1;
    uint16_t topic_id_two = 2;
    middleware_.create_topic_by_ref(topic_id_one, 0, topic_ref_one);
    middleware_.create_topic_by_ref(topic_id_two, 0, topic_ref_two);

    std::string publisher_xml_one{"Publisher"};
    middleware_.create_publisher_by_xml(0, 0, publisher_xml_one);
    middleware_.create_publisher_by_xml(1, 0, publisher_xml_one);

    std::string datawriter_ref_one{topic_ref_one};
    std::string datawriter_ref_two{topic_ref_two};

    /* Create DataWriter by REF. */
    EXPECT_TRUE(middleware_.create_datawriter_by_ref(0, 0, datawriter_ref_one));

    /* Create
     *      Id:         same
     *      Publisher:  same
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_ref(0, 0, datawriter_ref_one));

    /* Create
     *      Id:         different
     *      Publisher:  same
     *      Topic:      same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datawriter_by_ref(1, 0, datawriter_ref_one));

    /* Create
     *      Id:         same
     *      Publisher:  different
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_ref(0, 1, datawriter_ref_one));

    /* Create
     *      Id:         same
     *      Publisher:  non-existent
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_ref(0, 2, datawriter_ref_one));

    /* Create
     *      Id:         same
     *      Publisher:  same
     *      Topic:      different
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_ref(0, 0, datawriter_ref_two));

    /* Create
     *      Id:         same
     *      Publisher:  same
     *      Topic:      non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_ref(0, 0, "OtherTopic"));

    /* Create
     *      Id:         different
     *      Publisher:  same
     *      Topic:      different
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datawriter_by_ref(2, 0, datawriter_ref_two));

    /* Create
     *      Id:         different
     *      Publisher:  same
     *      Topic:      non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_ref(3, 0, "OtherTopic"));
}

TEST_F(CedMiddlewareUnitTests, CreateDataWriterByXML)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string topic_ref_one{"TopicOne"};
    std::string topic_ref_two{"TopicTwo"};
    uint16_t topic_id_one = 1;
    uint16_t topic_id_two = 2;
    middleware_.create_topic_by_ref(topic_id_one, 0, topic_ref_one);
    middleware_.create_topic_by_ref(topic_id_two, 0, topic_ref_two);

    std::string publisher_xml{"Publisher"};
    middleware_.create_publisher_by_xml(0, 0, publisher_xml);
    middleware_.create_publisher_by_xml(1, 0, publisher_xml);

    std::string datawriter_xml_one{topic_ref_one};
    std::string datawriter_xml_two{topic_ref_two};

    /* Create DataWriter by XML. */
    EXPECT_TRUE(middleware_.create_datawriter_by_xml(0, 0, datawriter_xml_one));

    /* Create
     *      Id:         same
     *      Publisher:  same
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_xml(0, 0, datawriter_xml_one));

    /* Create
     *      Id:         different
     *      Publisher:  same
     *      Topic:      same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datawriter_by_xml(1, 0, datawriter_xml_one));

    /* Create
     *      Id:         same
     *      Publisher:  different
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_xml(0, 1, datawriter_xml_one));

    /* Create
     *      Id:         same
     *      Publisher:  non-existent
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_xml(0, 2, datawriter_xml_one));

    /* Create
     *      Id:         same
     *      Publisher:  same
     *      Topic:      different
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_xml(0, 0, datawriter_xml_two));

    /* Create
     *      Id:         same
     *      Publisher:  same
     *      Topic:      non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_xml(0, 0, "OtherTopic"));

    /* Create
     *      Id:         different
     *      Publisher:  same
     *      Topic:      different
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datawriter_by_xml(2, 0, datawriter_xml_two));

    /* Create
     *      Id:         different
     *      Publisher:  same
     *      Topic:      non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datawriter_by_ref(3, 0, "OtherTopic"));
}

TEST_F(CedMiddlewareUnitTests, DeleteDataWriter)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string topic_ref{"Topic"};
    uint16_t topic_id = 11;
    middleware_.create_topic_by_ref(topic_id, 0, topic_ref);

    std::string publisher_xml{"Publisher"};
    middleware_.create_publisher_by_xml(0, 0, publisher_xml);

    std::string datawriter_xml{topic_ref};
    middleware_.create_datawriter_by_xml(0, 0, datawriter_xml);

    /* Delete
     *      DataWriter: existent
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.delete_datawriter(0));

    /* Delete
     *      DataWriter: non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.delete_datawriter(0));

    /* Create
     *      DataWriter: same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datawriter_by_xml(0, 0, datawriter_xml));
}

TEST_F(CedMiddlewareUnitTests, CreateDataReaderByRef)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string topic_ref_one{"TopicOne"};
    std::string topic_ref_two{"TopicTwo"};
    uint16_t topic_id_one = 1;
    uint16_t topic_id_two = 2;
    middleware_.create_topic_by_ref(topic_id_one, 0, topic_ref_one);
    middleware_.create_topic_by_ref(topic_id_two, 0, topic_ref_two);

    std::string subscriber_xml_one{"Subscriber"};
    middleware_.create_subscriber_by_xml(0, 0, subscriber_xml_one);
    middleware_.create_subscriber_by_xml(1, 0, subscriber_xml_one);

    std::string datareader_ref_one{topic_ref_one};
    std::string datareader_ref_two{topic_ref_two};

    /* Create DataReader by REF. */
    EXPECT_TRUE(middleware_.create_datareader_by_ref(0, 0, datareader_ref_one));

    /* Create
     *      Id:         same
     *      Subscriber: same
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_ref(0, 0, datareader_ref_one));

    /* Create
     *      Id:         different
     *      Subscriber: same
     *      Topic:      same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datareader_by_ref(1, 0, datareader_ref_one));

    /* Create
     *      Id:         same
     *      Subscriber: different
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_ref(0, 1, datareader_ref_one));

    /* Create
     *      Id:         same
     *      Subscriber: non-existent
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_ref(0, 2, datareader_ref_one));

    /* Create
     *      Id:         same
     *      Subscriber: same
     *      Topic:      different
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_ref(0, 0, datareader_ref_two));

    /* Create
     *      Id:         same
     *      Subscriber: same
     *      Topic:      non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_ref(0, 0, "OtherTopic"));

    /* Create
     *      Id:         different
     *      Subscriber: same
     *      Topic:      different
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datareader_by_ref(2, 0, datareader_ref_two));

    /* Create
     *      Id:         different
     *      Subscriber: same
     *      Topic:      non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_ref(3, 0, "OtherTopic"));
}

TEST_F(CedMiddlewareUnitTests, CreateDataReaderByXML)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string topic_ref_one{"TopicOne"};
    std::string topic_ref_two{"TopicTwo"};
    uint16_t topic_id_one = 1;
    uint16_t topic_id_two = 2;
    middleware_.create_topic_by_ref(topic_id_one, 0, topic_ref_one);
    middleware_.create_topic_by_ref(topic_id_two, 0, topic_ref_two);

    std::string subscriber_xml{"Subscriber"};
    middleware_.create_subscriber_by_xml(0, 0, subscriber_xml);
    middleware_.create_subscriber_by_xml(1, 0, subscriber_xml);

    std::string datareader_xml_one{topic_ref_one};
    std::string datareader_xml_two{topic_ref_two};

    /* Create DataWriter by XML. */
    EXPECT_TRUE(middleware_.create_datareader_by_xml(0, 0, datareader_xml_one));

    /* Create
     *      Id:         same
     *      Subscriber: same
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_xml(0, 0, datareader_xml_one));

    /* Create
     *      Id:         different
     *      Subscriber: same
     *      Topic:      same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datareader_by_xml(1, 0, datareader_xml_one));

    /* Create
     *      Id:         same
     *      Subscriber: different
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_xml(0, 1, datareader_xml_one));

    /* Create
     *      Id:         same
     *      Subscriber: non-existent
     *      Topic:      same
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_xml(0, 2, datareader_xml_one));

    /* Create
     *      Id:         same
     *      Subscriber: same
     *      Topic:      different
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_xml(0, 0, datareader_xml_two));

    /* Create
     *      Id:         same
     *      Subscriber: same
     *      Topic:      non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_xml(0, 0, "OtherTopic"));

    /* Create
     *      Id:         different
     *      Subscriber: same
     *      Topic:      different
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datareader_by_xml(2, 0, datareader_xml_two));

    /* Create
     *      Id:         different
     *      Subscriber: same
     *      Topic:      non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.create_datareader_by_ref(3, 0, "OtherTopic"));
}

TEST_F(CedMiddlewareUnitTests, DeleteDataReader)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string topic_ref{"Topic"};
    uint16_t topic_id = 11;
    middleware_.create_topic_by_ref(topic_id, 0, topic_ref);

    std::string subscriber_xml{"Subscriber"};
    middleware_.create_subscriber_by_xml(0, 0, subscriber_xml);

    std::string datareader_xml{topic_ref};
    middleware_.create_datareader_by_xml(0, 0, datareader_xml);

    /* Delete
     *      DataReader: existent
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.delete_datareader(0));

    /* Delete
     *      DataReader: non-existent
     *      Expected:   FALSE
     */
    EXPECT_FALSE(middleware_.delete_datareader(0));

    /* Create
     *      DataReader: same
     *      Expected:   TRUE
     */
    EXPECT_TRUE(middleware_.create_datareader_by_xml(0, 0, datareader_xml));
}

TEST_F(CedMiddlewareUnitTests, WriteReadData)
{
    std::string participant_ref{"Participant"};
    middleware_.create_participant_by_ref(0, 0, participant_ref);

    std::string topic_ref{"Topic"};
    middleware_.create_topic_by_ref(0, 0, topic_ref);

    std::string subscriber_xml{"Subscriber"};
    middleware_.create_subscriber_by_xml(0, 0, subscriber_xml);

    std::string publisher_xml{"Publisher"};
    middleware_.create_publisher_by_xml(0, 0, publisher_xml);

    std::string datareader_ref{"Topic"};
    middleware_.create_datareader_by_ref(0, 0, datareader_ref);
    middleware_.create_datareader_by_ref(1, 0, datareader_ref);

    std::string datawriter_ref{"Topic"};
    middleware_.create_datawriter_by_ref(0, 0, datawriter_ref);
    middleware_.create_datawriter_by_ref(1, 0, datawriter_ref);

    std::vector<uint8_t> output_data_one{0, 1, 2};
    std::vector<uint8_t> output_data_two{3, 4, 5};
    std::vector<uint8_t> input_data{};

    /* Write 2 Topic, one with each DataWriter. */
    EXPECT_TRUE(middleware_.write_data(0, output_data_one));
    EXPECT_TRUE(middleware_.write_data(1, output_data_two));

    /* Read successfully with DataReader ONE. */
    EXPECT_TRUE(middleware_.read_data(0, input_data, std::chrono::milliseconds(0)));
    EXPECT_TRUE(std::equal(output_data_one.begin(), output_data_one.end(), input_data.begin()));
    EXPECT_TRUE(middleware_.read_data(0, input_data, std::chrono::milliseconds(0)));
    EXPECT_TRUE(std::equal(output_data_two.begin(), output_data_two.end(), input_data.begin()));

    /* Read unsuccessfully with DataReader ONE. */
    EXPECT_FALSE(middleware_.read_data(0, input_data, std::chrono::milliseconds(100)));

    /* Read successfully with DataReader ONE. */
    EXPECT_TRUE(middleware_.read_data(1, input_data, std::chrono::milliseconds(0)));
    EXPECT_TRUE(std::equal(output_data_one.begin(), output_data_one.end(), input_data.begin()));
    EXPECT_TRUE(middleware_.read_data(1, input_data, std::chrono::milliseconds(0)));
    EXPECT_TRUE(std::equal(output_data_two.begin(), output_data_two.end(), input_data.begin()));

    /* Read unsuccessfully with DataReader ONE. */
    EXPECT_FALSE(middleware_.read_data(1, input_data, std::chrono::milliseconds(100)));
}

} // namespace testing
} // namespace uxr
} // namespace testing

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
