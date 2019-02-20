#include <uxr/agent/p2p_agent/P2PAgentCommunication.hpp>
#include <uxr/agent/p2p_agent/DiscoveryClient.hpp>

#include <functional>

using namespace eprosima::uxr;
using namespace std::placeholders;

P2PAgentCommunication::P2PAgentCommunication(Mode mode)
    : mode_(mode)
    , period_(UXR_DEFAULT_DISCOVERY_PERIOD)
    , discovery_client_(std::bind(&::P2PAgentCommunication::on_discover_agent, this, _1, _2))
{}

P2PAgentCommunication::~P2PAgentCommunication() = default;

void P2PAgentCommunication::run()
{
    // Load client manager stuffs if needed
    discovery_thread_ = std::thread(&P2PAgentCommunication::run_discovery, this);
}

void P2PAgentCommunication::on_discover_agent(const std::string& ip, int port)
{
    if (!client_manager_.has_client_listening_from(ip, port))
    {
        client_manager_.create_client(ip, port);
    }
}

void P2PAgentCommunication::run_discovery()
{
    while(true)
    {
        if (Mode::SUBSCRIBE == (Mode::SUBSCRIBE & mode_))
        {
            discovery_client_.discover(period_);
        }
    }
}

