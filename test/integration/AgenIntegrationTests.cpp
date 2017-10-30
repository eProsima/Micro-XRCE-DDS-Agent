#include "Common.h"

#include <agent/Root.h>
#include <agent/participant/Participant.h>

#include <gtest/gtest.h>

#include <thread>
#include <chrono>

namespace eprosima {
namespace micrortps {
namespace testing {

class AgentTests : public ::testing::Test
{
  protected:
    AgentTests()
    {
        agent_.init(2019, 2020);
    }

    virtual ~AgentTests() = default;

    ProxyClient* wait_client(ClientKey client_k, int trie_time, int max_tries)
    {
        ProxyClient* client = nullptr;
        int count = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(trie_time));
            client = agent_.get_client(client_k);
        } while (client == nullptr && ++count < max_tries );        
        EXPECT_LT (count, max_tries);  
        return client; 
    }

    ProxyClient* wait_delete_client(ClientKey client_k, int trie_time, int max_tries)
    {
        ProxyClient* client = nullptr;
        int count = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(trie_time));
            client = agent_.get_client(client_k);
        } while (client != nullptr && ++count < max_tries );        
        EXPECT_LT (count, max_tries);  
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
        } while (object == nullptr && ++count < max_tries );        
        EXPECT_LT (count, max_tries);  
        return object; 
    }

    XRCEObject* wait_delete_object(ProxyClient* client, const ObjectId& object_id, int trie_time, int max_tries)
    {
        XRCEObject* object = nullptr;
        int count = 0;
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(trie_time));
            object = client->get_object(object_id);
        } while (object != nullptr && ++count < max_tries );       
        EXPECT_LT (count, max_tries);  
        return object; 
    }

    eprosima::micrortps::Agent agent_;
    const ClientKey client_key = {{0xAA,0xBB,0xCC,0xDD}};
    std::thread agent_thread;
};

TEST_F(AgentTests, CreateDeleteClient)
{
    const int trie_time = 1; // In seconds
    const int max_tries = 10;
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);
    
    agent_.stop();
    agent_thread.join();  
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_EQ(delete_client, nullptr);
}

TEST_F(AgentTests, CreateDeleteParticipant)
{
    const int trie_time = 1; // In seconds
    const int max_tries = 10;
    const ObjectId participant_id = {{0x00,0x01}};
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread = std::thread(&Agent::run, &agent_);
    ProxyClient* client = wait_client(client_key, trie_time, max_tries);
    XRCEParticipant* participant = dynamic_cast<XRCEParticipant*>(wait_object(client, participant_id, trie_time, max_tries));
    XRCEParticipant* delete_participant = dynamic_cast<XRCEParticipant*>(wait_delete_object(client, participant_id, trie_time, max_tries));
    ProxyClient* delete_client = wait_delete_client(client_key, trie_time, max_tries);

    agent_.stop();
    agent_thread.join();  
    agent_.abort_execution();
    ASSERT_NE(client, nullptr);
    ASSERT_NE(participant, nullptr);
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