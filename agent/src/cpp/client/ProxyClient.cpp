#include "agent/client/ProxyClient.h"

#include "agent/ObjectVariant.h"

using namespace eprosima::micrortps;

ProxyClient::ProxyClient(const OBJK_CLIENT_Representation& client) : representation_(client)
{ }

ProxyClient::~ProxyClient()
{
    for (auto& xrce_object : _objects)
    {
        delete xrce_object.second;
    }
}

bool ProxyClient::create(const InternalObjectId& internal_id, const ObjectVariant& representation)
{
switch(representation.discriminator())
{
    case OBJK_PUBLISHER:
        _objects.insert(std::make_pair(internal_id, new TestObjectP));
    break;
    case OBJK_SUBSCRIBER:
        _objects.insert(std::make_pair(internal_id, new TestObjectS));
    break;
    case OBJK_CLIENT:
    case OBJK_APPLICATION:
    case OBJK_PARTICIPANT:
    case OBJK_QOSPROFILE:
    case OBJK_TYPE:
    case OBJK_TOPIC:
    case OBJK_DATAWRITER:
    case OBJK_DATAREADER:
    default:
    break;
}
return true;
}


Status ProxyClient::create(const CreationMode& creation_mode, const ObjectId& object_id, const ObjectVariant& representation)
{
    Status status;
    status.result().request_id();
    status.result().status(STATUS_LAST_OP_CREATE);

    InternalObjectId internalId;
    std::copy(object_id.begin(), object_id.end(), internalId.begin());
    // TODO internalId[3] = representation.discriminator();
    internalId[3] = 0x00;

    auto object_it = _objects.find(internalId);
    if(object_it == _objects.end()) 
    {
        if (create(internalId, representation))
        {
            status.result().implementation_status(STATUS_OK);
        }
    }
    else
    {
        if (!creation_mode.reuse()) // reuse = false
        {
            if (!creation_mode.replace()) // replace = false
            {
                status.result().implementation_status(STATUS_ERR_ALREADY_EXISTS);
            }
            else // replace = true
            {
                delete_object(object_id);
                create(internalId, representation);
            }
        }
        else // reuse = true
        {
            if (!creation_mode.replace()) // replace = false
            {
                // TODO Compara representaciones
            }
            else // replace = true
            {
               // TODO compara representaciones
            }
        }
    }
    return status;
}

Status ProxyClient::update(const ObjectId& object_id, const ObjectVariant& representation)
{
    // TODO
}

Info ProxyClient::get_info(const ObjectId& object_id)
{
    // TODO
}

Status ProxyClient::delete_object(const ObjectId& object_id)
{
    InternalObjectId internalId;
    std::copy(object_id.begin(), object_id.end(), internalId.begin());
    // TODO internalId[3] = representation.discriminator();
    internalId[3] = 0x00;

    Status status;
    status.result().request_id();
    status.result().status(STATUS_LAST_OP_DELETE);
    // TODO comprobar permisos
    if (0 == _objects.erase(internalId))
    {
        status.result().implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
        // TODO en el documento se menciona STATUS_ERR_INVALID pero no existe.
    }
    else
    {
        status.result().implementation_status(STATUS_OK);
    }
    return status;
}