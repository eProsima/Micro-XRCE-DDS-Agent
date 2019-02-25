#include <uxr/agent/p2p_agent/InternalClientManager.hpp>
#include <uxr/agent/p2p_agent/InternalClient.hpp>

using namespace eprosima::uxr;

InternalClientManager::InternalClientManager() = default;
InternalClientManager::~InternalClientManager() = default;


void InternalClientManager::create_client(const std::string& ip, int port)
{
    InternalClient client(ip, port);
    client.connect();
    //client.add_topic(/* All Topic manager topics */ );
    internal_clients_.emplace(InternalClientKey(ip, port), client);
}

bool InternalClientManager::has_client_listening_from(const std::string& ip, int port)
{
    return internal_clients_.find(InternalClientKey(ip, port)) != internal_clients_.end();
}

void InternalClientManager::topic_register_event_change(/* event_type and topic */)
{
    // Must be check if it exists?
}

