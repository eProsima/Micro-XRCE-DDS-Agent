#include <cstdint>
#include <thread>
#include <array>

#define AGENT_MAX_NUM_ATTEMPTS 100
#define AGENT_MAX_TIME_WAIT 10

namespace eprosima { namespace micrortps {
    class Agent;
} }

class AgentT
{
public:
    AgentT();

    void launch();
    void stop();
    bool client_disconnected(const std::array<uint8_t, 4> client_key);
    bool client_connected(const std::array<uint8_t, 4> client_key);
    bool object_created(const std::array<uint8_t, 4> client_key, const std::array<uint8_t, 2> object_id);
    bool object_deleted(const std::array<uint8_t, 4> client_key, const std::array<uint8_t, 2> object_id);

private:
    std::thread thread_;
    eprosima::micrortps::Agent* agent_;
    uint16_t port_;
};
