#include "agent/Root.h"

using namespace eprosima::micrortps;

Agent::Agent() {}

Agent& root()
{
    static Agent xrce_agent;
    return xrce_agent;
}

Status Agent::create_client(const ClientKey& client_key,  const ObjectVariant& client_representation)
{
    Status status;
    status.result().request_id();
    status.result().status(STATUS_LAST_OP_CREATE);
    status.object_id(); // TODO que object id?

    if ((client_representation.discriminator() == OBJK_CLIENT) && 
        (client_representation.client().xrce_cookie() == std::array<uint8_t, 4>XRCE_COOKIE))
    {
        if (client_representation.client().xrce_version()[0] <= XRCE_VERSION_MAJOR)
        {
            // TODO The Agent shall check the ClientKey to ensure it is authorized to connect to the Agent
            // If this check fails the operation shall fail and returnValue is set to {STATUS_LAST_OP_CREATE,STATUS_ERR_DENIED}.
            clients_[client_key] = ProxyClient{client_representation.client()};
            status.result().implementation_status(STATUS_OK);
        }
        else{
            status.result().implementation_status(STATUS_ERR_INCOMPATIBLE);
        }
    }
    else
    {        
        status.result().implementation_status(STATUS_ERR_INVALID_DATA);
    }
    return status;
}

Status Agent::delete_client(const ClientKey& client_key)
{
    Status status;
    status.result().request_id();
    status.result().status(STATUS_LAST_OP_DELETE);
    if (0 == clients_.erase(client_key))
    {
        status.result().implementation_status(STATUS_ERR_INVALID_DATA);
        // TODO en el documento se menciona STATUS_ERR_INVALID pero no existe.
    }
    else
    {
        status.result().implementation_status(STATUS_OK);
    }
    return status;
}