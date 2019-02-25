#include <uxr/agent/p2p_agent/DiscoveryClient.hpp>

#include <uxr/client/client.h>

using namespace eprosima::uxr;

void DiscoveryClient::on_agent_found(const uxrAgentAddress* address, int64_t /*timestamp*/, void* args)
{
    DiscoveryClient* discovery_client = static_cast<DiscoveryClient*>(args);
    discovery_client->agent_discovery_callback_(address->ip, address->port);
}

DiscoveryClient::DiscoveryClient(AgentDiscoveryCallback callback)
    : agent_discovery_callback_(callback)
{}

DiscoveryClient::~DiscoveryClient() = default;

void DiscoveryClient::discover(int timeout)
{
    std::vector<uxrAgentAddress> addresses(discovery_addresses_.size());
    for(size_t i = 0; i < addresses.size(); ++i)
    {
        addresses[i].ip = discovery_addresses_[i].first.c_str();
        addresses[i].port = discovery_addresses_[i].second;
    }

    uxr_discovery_agents(1, timeout, on_agent_found, this, addresses.data(), addresses.size());
}

void DiscoveryClient::add_discovery_address(const std::string& ip, int port)
{
    discovery_addresses_.emplace_back(std::make_pair(ip, port));
}

