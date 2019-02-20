#include <uxr/agent/p2p_agent/DiscoveryClient.hpp>

#include <uxr/client/client.h>

using namespace eprosima::uxr;

bool DiscoveryClient::on_agent_found(const uxrAgentAddress* address, int64_t timestamp, void* args)
{
    DiscoveryClient* discovery_client = static_cast<DiscoveryClient*>(args);
    discovery_client->agent_discovery_callback_(address->ip, address->port);
    return false;
}

DiscoveryClient::DiscoveryClient(AgentDiscoveryCallback callback)
    : agent_discovery_callback_(callback)
{}

DiscoveryClient::~DiscoveryClient() = default;

void DiscoveryClient::discover(int timeout)
{
    uxrAgentAddress* data = discovery_addresses_.data();
    size_t size = discovery_addresses_.size();
    uxr_discovery_agents_unicast(1, timeout, on_agent_found, this, &uxrAgentAddress(), data, size))
}

void DiscoveryClient::add_discovery_address(const std::string& ip, int port)
{
    discovery_addresses_.emplace_back(uxrAgentAddress{ip.c_str(), port});
}

