#include "Common.h"

#include <agent/Root.h>
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

    eprosima::micrortps::Agent agent_;
    const ClientKey client_key = {{0xAA,0xBB,0xCC,0xDD}};
    std::thread agent_thread;
};

TEST_F(AgentTests, CreateClient)
{
    ASSERT_EQ(agent_.get_client(client_key), nullptr);
    agent_thread = std::thread(&Agent::run, &agent_);
    ProxyClient* client = nullptr;
    int count = 0;
    do
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        client = agent_.get_client(client_key);
    } while (client == nullptr && count++ < 5 );    
    agent_.stop();
    agent_thread.join();    
    EXPECT_NE(client, nullptr);
}

} // namespace testing
} // namespace micrortps
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}