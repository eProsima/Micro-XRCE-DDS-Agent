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

#include <agent/Root.h>
#include <agent/XRCEObject.hpp>
#include <agent/datareader/DataReader.h>
#include <agent/datawriter/DataWriter.h>
#include <agent/participant/Participant.h>
#include <agent/publisher/Publisher.h>
#include <agent/subscriber/Subscriber.h>
#include <agent/topic/Topic.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

namespace eprosima {
namespace micrortps {
namespace testing {

class AgentTests : public ::testing::Test
{
  protected:
    AgentTests()
    {
        agent_.init(4001, 2019, 2020, "127.0.0.1");
    }

    virtual ~AgentTests() = default;

    ProxyClient* wait_client(ClientKey client_k, int trie_time, int max_tries)
    {
        ProxyClient* client = nullptr;
        int count           = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(trie_time));
            client = agent_.get_client(client_k);
        } while (client == nullptr && ++count < max_tries);
        EXPECT_LT(count, max_tries);
        return client;
    }

    ProxyClient* wait_delete_client(ClientKey client_k, int trie_time, int max_tries)
    {
        ProxyClient* client = nullptr;
        int count           = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(trie_time));
            client = agent_.get_client(client_k);
        } while (client != nullptr && ++count < max_tries);
        EXPECT_LT(count, max_tries);
        return client;
    }

    XRCEObject* wait_object(ProxyClient* client, const ObjectId& object_id, int trie_time, int max_tries)
    {

        XRCEObject* object = nullptr;
        if (client == nullptr)
        {
            return object;
        }
        int count = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(trie_time));
            object = client->get_object(object_id);
        } while (object == nullptr && ++count < max_tries);
        EXPECT_LT(count, max_tries);
        return object;
    }

    XRCEObject* wait_delete_object(ProxyClient* client, const ObjectId& object_id, int trie_time, int max_tries)
    {
        XRCEObject* object = nullptr;
        if (client == nullptr)
        {
            return object;
        }
        int count = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(trie_time));
            object = client->get_object(object_id);
        } while (object != nullptr && ++count < max_tries);
        EXPECT_LT(count, max_tries);
        return object;
    }

    void wait_action(int trie_time, int max_tries)
    {
        int count = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(trie_time));
        } while (++count < max_tries);
    }

    eprosima::micrortps::Agent& agent_ = *root();
    const ClientKey client_key         = {{0xAA, 0xBB, 0xCC, 0xDD}};
    std::thread agent_thread;
};

TEST_F(AgentTests, CreateDeleteClient)
{
    const int trie_time = 1; // In seconds
    const int max_tries = 10;
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread               = std::thread(&Agent::run, &agent_);
    ProxyClient* client        = wait_client(client_key, trie_time, max_tries);
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_thread.join();
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, CreateDeleteParticipant)
{
    const int trie_time           = 1; // In seconds
    const int max_tries           = 10;
    const ObjectId participant_id = {{0x00, 0x01}};
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread        = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant =
        dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    XRCEParticipant* delete_participant =
        dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_thread.join();
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
    ASSERT_EQ(delete_participant, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, CreateDeleteTopic)
{
    const int trie_time           = 1; // In seconds
    const int max_tries           = 10;
    const ObjectId participant_id = {{0x00, 0x01}};
    const ObjectId topic_id       = {{0x00, 0x02}};

    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread        = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant =
        dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    Topic* topic = dynamic_cast<Topic*>(wait_object(client, topic_id, trie_time, max_tries));

    Topic* delete_topic = dynamic_cast<Topic*>(wait_delete_object(client, topic_id, trie_time, max_tries));
    XRCEParticipant* delete_participant =
        dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_thread.join();
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
    ASSERT_NE(topic, nullptr);
    ASSERT_EQ(delete_topic, nullptr);
    ASSERT_EQ(delete_participant, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, CreateDeletePublisher)
{
    const int trie_time           = 1; // In seconds
    const int max_tries           = 10;
    const ObjectId participant_id = {{0x00, 0x01}};
    const ObjectId publisher_id   = {{0x00, 0x02}};
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread        = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant =
        dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    Publisher* publisher = dynamic_cast<Publisher*>(wait_object(client, publisher_id, trie_time, max_tries));
    Publisher* delete_publisher =
        dynamic_cast<Publisher*>(wait_delete_object(client, publisher_id, trie_time, max_tries));
    XRCEParticipant* delete_participant =
        dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_thread.join();
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
    ASSERT_NE(publisher, nullptr);
    ASSERT_EQ(delete_publisher, nullptr);
    ASSERT_EQ(delete_participant, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, CreateDeleteSubscriber)
{
    const int trie_time           = 1; // In seconds
    const int max_tries           = 10;
    const ObjectId participant_id = {{0x00, 0x01}};
    const ObjectId subscriber_id  = {{0x00, 0x02}};
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread        = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant =
        dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    Subscriber* subscriber = dynamic_cast<Subscriber*>(wait_object(client, subscriber_id, trie_time, max_tries));
    Subscriber* delete_subscriber =
        dynamic_cast<Subscriber*>(wait_delete_object(client, subscriber_id, trie_time, max_tries));
    XRCEParticipant* delete_participant =
        dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_thread.join();
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
    ASSERT_NE(subscriber, nullptr);
    ASSERT_EQ(delete_subscriber, nullptr);
    ASSERT_EQ(delete_participant, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, CreateDeleteDataWriter)
{
    const int trie_time           = 1; // In seconds
    const int max_tries           = 10;
    const ObjectId participant_id = {{0x00, 0x01}};
    const ObjectId topic_id       = {{0x00, 0x02}};
    const ObjectId publisher_id   = {{0x00, 0x03}};
    const ObjectId datawriter_id  = {{0x00, 0x04}};
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread        = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant =
        dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    Topic* topic = dynamic_cast<Topic*>(wait_object(client, topic_id, trie_time, max_tries));
    Publisher* publisher    = dynamic_cast<Publisher*>(wait_object(client, publisher_id, trie_time, max_tries));
    DataWriter* data_writer = dynamic_cast<DataWriter*>(wait_object(client, datawriter_id, trie_time, max_tries));

    DataWriter* delete_data_writer =
        dynamic_cast<DataWriter*>(wait_delete_object(client, datawriter_id, trie_time, max_tries));
    Publisher* delete_publisher =
        dynamic_cast<Publisher*>(wait_delete_object(client, publisher_id, trie_time, max_tries));
    Topic* delete_topic = dynamic_cast<Topic*>(wait_delete_object(client, topic_id, trie_time, max_tries));
    XRCEParticipant* delete_participant =
        dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_thread.join();
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
    ASSERT_NE(topic, nullptr);
    ASSERT_NE(publisher, nullptr);
    ASSERT_NE(data_writer, nullptr);
    ASSERT_EQ(delete_data_writer, nullptr);
    ASSERT_EQ(delete_publisher, nullptr);
    ASSERT_EQ(delete_topic, nullptr);
    ASSERT_EQ(delete_participant, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, CreateDeleteDataReader)
{
    const int trie_time           = 1; // In seconds
    const int max_tries           = 10;
    const ObjectId participant_id = {{0x00, 0x01}};
    const ObjectId topic_id       = {{0x00, 0x02}};
    const ObjectId subscriber_id  = {{0x00, 0x03}};
    const ObjectId datareader_id  = {{0x00, 0x04}};
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread        = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant =
        dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    Topic* topic = dynamic_cast<Topic*>(wait_object(client, topic_id, trie_time, max_tries));
    Subscriber* subscriber  = dynamic_cast<Subscriber*>(wait_object(client, subscriber_id, trie_time, max_tries));
    DataReader* data_reader = dynamic_cast<DataReader*>(wait_object(client, datareader_id, trie_time, max_tries));

    DataReader* delete_data_reader =
        dynamic_cast<DataReader*>(wait_delete_object(client, datareader_id, trie_time, max_tries));
    Subscriber* delete_subscriber =
        dynamic_cast<Subscriber*>(wait_delete_object(client, subscriber_id, trie_time, max_tries));
    Topic* delete_topic = dynamic_cast<Topic*>(wait_delete_object(client, topic_id, trie_time, max_tries));
    XRCEParticipant* delete_participant =
        dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_thread.join();
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
    ASSERT_NE(topic, nullptr);
    ASSERT_NE(subscriber, nullptr);
    ASSERT_NE(data_reader, nullptr);
    ASSERT_EQ(delete_data_reader, nullptr);
    ASSERT_EQ(delete_subscriber, nullptr);
    ASSERT_EQ(delete_topic, nullptr);
    ASSERT_EQ(delete_participant, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, WriteData)
{
    const int trie_time           = 1; // In seconds
    const int max_tries           = 10;
    const ObjectId participant_id = {{0x00, 0x01}};
    const ObjectId topic_id       = {{0x00, 0x02}};
    const ObjectId publisher_id   = {{0x00, 0x03}};
    const ObjectId datawriter_id  = {{0x00, 0x04}};
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread        = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant =
        dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    Topic* topic = dynamic_cast<Topic*>(wait_object(client, topic_id, trie_time, max_tries));
    Publisher* publisher    = dynamic_cast<Publisher*>(wait_object(client, publisher_id, trie_time, max_tries));
    DataWriter* data_writer = dynamic_cast<DataWriter*>(wait_object(client, datawriter_id, trie_time, max_tries));

    DataWriter* delete_data_writer =
        dynamic_cast<DataWriter*>(wait_delete_object(client, datawriter_id, trie_time, max_tries));
    Publisher* delete_publisher =
        dynamic_cast<Publisher*>(wait_delete_object(client, publisher_id, trie_time, max_tries));
    Topic* delete_topic = dynamic_cast<Topic*>(wait_delete_object(client, topic_id, trie_time, max_tries));
    XRCEParticipant* delete_participant =
        dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_thread.join();
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
    ASSERT_NE(topic, nullptr);
    ASSERT_NE(publisher, nullptr);
    ASSERT_NE(data_writer, nullptr);
    ASSERT_EQ(delete_data_writer, nullptr);
    ASSERT_EQ(delete_publisher, nullptr);
    ASSERT_EQ(delete_topic, nullptr);
    ASSERT_EQ(delete_participant, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, ReadData)
{
    const int trie_time           = 1; // In seconds
    const int max_tries           = 10;
    const ObjectId participant_id = {{0x00, 0x01}};
    const ObjectId topic_id       = {{0x00, 0x02}};
    const ObjectId subscriber_id  = {{0x00, 0x03}};
    const ObjectId datareader_id  = {{0x00, 0x04}};
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread        = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant =
        dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    Topic* topic = dynamic_cast<Topic*>(wait_object(client, topic_id, trie_time, max_tries));
    Subscriber* subscriber  = dynamic_cast<Subscriber*>(wait_object(client, subscriber_id, trie_time, max_tries));
    DataReader* data_reader = dynamic_cast<DataReader*>(wait_object(client, datareader_id, trie_time, max_tries));
    wait_action(trie_time, 5);
    DataReader* delete_data_reader =
        dynamic_cast<DataReader*>(wait_delete_object(client, datareader_id, trie_time, max_tries));
    Subscriber* delete_reader =
        dynamic_cast<Subscriber*>(wait_delete_object(client, subscriber_id, trie_time, max_tries));
    Topic* delete_topic = dynamic_cast<Topic*>(wait_delete_object(client, topic_id, trie_time, max_tries));
    XRCEParticipant* delete_participant =
        dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_.abort_execution();
    agent_thread.join();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
    ASSERT_NE(topic, nullptr);
    ASSERT_NE(subscriber, nullptr);
    ASSERT_NE(data_reader, nullptr);
    ASSERT_EQ(delete_data_reader, nullptr);
    ASSERT_EQ(delete_reader, nullptr);
    ASSERT_EQ(delete_topic, nullptr);
    ASSERT_EQ(delete_participant, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, ReadMultiData)
{
    const int trie_time           = 1; // In seconds
    const int max_tries           = 10;
    const ObjectId participant_id = {{0x00, 0x01}};
    const ObjectId subscriber_id  = {{0x00, 0x02}};
    const ObjectId datareader_id  = {{0x00, 0x03}};
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread        = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant =
        dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    Subscriber* subscriber  = dynamic_cast<Subscriber*>(wait_object(client, subscriber_id, trie_time, max_tries));
    DataReader* data_reader = dynamic_cast<DataReader*>(wait_object(client, datareader_id, trie_time, max_tries));
    wait_action(trie_time, 10);
    DataReader* delete_data_reader =
        dynamic_cast<DataReader*>(wait_delete_object(client, datareader_id, trie_time, max_tries));
    Subscriber* delete_reader =
        dynamic_cast<Subscriber*>(wait_delete_object(client, subscriber_id, trie_time, max_tries));
    XRCEParticipant* delete_participant =
        dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);
    agent_.stop();
    agent_.abort_execution();
    agent_thread.join();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
    ASSERT_NE(subscriber, nullptr);
    ASSERT_NE(data_reader, nullptr);
    ASSERT_EQ(delete_data_reader, nullptr);
    ASSERT_EQ(delete_reader, nullptr);
    ASSERT_EQ(delete_participant, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

} // namespace testing
} // namespace micrortps
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
