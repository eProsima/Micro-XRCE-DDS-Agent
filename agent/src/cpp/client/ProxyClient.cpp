// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "agent/client/ProxyClient.h"

#include "agent/ObjectVariant.h"
#include "agent/datawriter/DataWriter.h"
#include "agent/datareader/DataReader.h"

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
        //_objects.insert(std::make_pair(internal_id, new DataWriter()));
    break;
    case OBJK_SUBSCRIBER:
        _objects.insert(std::make_pair(internal_id, new DataReader(this)));
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


Status ProxyClient::create(const CreationMode& creation_mode, const CREATE_PAYLOAD& create_payload)
{
    Status status;
    status.result().request_id(create_payload.request_id());
    status.result().status(STATUS_LAST_OP_CREATE);

    auto internal_id = generate_object_id(create_payload.object_id(), 0x00);

    auto object_it = _objects.find(internal_id);
    if(object_it == _objects.end()) 
    {
        if (create(internal_id, create_payload.object_representation()))
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
                delete_object(create_payload.object_id());
                create(internal_id, create_payload.object_representation());
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

Status ProxyClient::delete_object(const DELETE_PAYLOAD& delete_payload)
{
    Status status;
    status.result().request_id(delete_payload.request_id());
    status.result().status(STATUS_LAST_OP_DELETE);
    // TODO comprobar permisos
    if (delete_object(delete_payload.object_id()))
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

bool ProxyClient::delete_object(const ObjectId& object_id)
{
    return (0 != _objects.erase(generate_object_id(object_id, 0x00)));
}

Status ProxyClient::write(const ObjectId& object_id, const WRITE_DATA_PAYLOAD& data_payload)
{
    Status status;
    status.result().request_id(data_payload.request_id());
    status.result().status(STATUS_LAST_OP_WRITE);
    auto internal_id = generate_object_id(object_id, 0x00);
    auto object_it = _objects.find(internal_id);
    if(object_it == _objects.end()) 
    {
        status.result().implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        DataWriter* writer = dynamic_cast<DataWriter*>(object_it->second);
        writer->write(nullptr);
    }
    return status;
}

Status ProxyClient::read(const ObjectId& object_id, const READ_DATA_PAYLOAD& data_payload)
{
    Status status;
    status.result().request_id();
    status.result().status(STATUS_LAST_OP_READ);
    auto internal_id = generate_object_id(object_id, 0x00);
    auto object_it = _objects.find(internal_id);
    if(object_it == _objects.end()) 
    {
        status.result().implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        DataReader* reader = dynamic_cast<DataReader*>(object_it->second);
        //reader->read(data_payload); // TODO const ref?
    }
}

ProxyClient::InternalObjectId ProxyClient::generate_object_id(const ObjectId& id, uint8_t suffix) const
{
    InternalObjectId internal_id;
    std::copy(id.begin(), id.end(), internal_id.begin());
    internal_id[id.size()] = suffix;
    return internal_id;
}