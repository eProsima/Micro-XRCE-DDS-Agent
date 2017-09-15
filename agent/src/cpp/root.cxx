#include "agent/root.h"

using namespace eprosima::micrortps;

Agent::Agent() {}

Agent& root()
{
    static Agent xrce_agent;
    return xrce_agent;
}

ResultStatus Agent::create_client(const OBJK_CLIENT_Representation& client_representation)
{
    clients_.emplace_back(ProxyClient{});
}

void Agent::delete_client()
{
    clients_.pop_back();
}