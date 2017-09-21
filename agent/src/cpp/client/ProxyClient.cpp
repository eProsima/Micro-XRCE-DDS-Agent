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

#include <utility>

#include "agent/client/ProxyClient.h"

#include "agent/ObjectVariant.h"
#include "agent/Root.h"
#include "agent/datareader/DataReader.h"
#include "agent/datawriter/DataWriter.h"

using namespace eprosima::micrortps;

ProxyClient::ProxyClient(OBJK_CLIENT_Representation  client) : representation_(std::move(client)), sequence_count_(0)
{ }

ProxyClient::~ProxyClient()
{
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    for (auto& xrce_object : objects_)
    {
        delete xrce_object.second;
    }
}

ProxyClient::ProxyClient(const ProxyClient &x)
:
    representation_( x.representation_),
    objects_(x.objects_),
    requests_info_(x.requests_info_),
    sequence_count_(x.sequence_count_.load())
{
}

ProxyClient::ProxyClient(ProxyClient &&x)
:
    representation_(std::move(x.representation_)),
    objects_(std::move(x.objects_)),
    requests_info_(std::move(x.requests_info_)),
    sequence_count_(x.sequence_count_.load())
{
}
ProxyClient& ProxyClient::operator=(const ProxyClient &x)
{
    representation_ =  x.representation_;
    objects_ = x.objects_;
    requests_info_ = x.requests_info_;
    sequence_count_ = x.sequence_count_.load();
    return *this;
}
ProxyClient& ProxyClient::operator=(ProxyClient &&x)
{
    representation_ = std::move(x.representation_);
    objects_ = std::move(x.objects_);
    requests_info_ = std::move(x.requests_info_);
    sequence_count_ = x.sequence_count_.load();
    return *this;
}

bool ProxyClient::create(const InternalObjectId& internal_object_id, const ObjectVariant& representation)
{
    switch(representation.discriminator())
    {
        case OBJK_PUBLISHER:
            //return objects_.insert(std::make_pair(internal_id, new DataWriter()));
            return false;
        break;
        case OBJK_SUBSCRIBER:
        {
            std::lock_guard<std::mutex> lockGuard(objects_mutex_);
            return objects_.insert(std::make_pair(internal_object_id, new DataReader(this))).second;
            break;
        }
        case OBJK_PARTICIPANT:
            return true;
        break;
        case OBJK_CLIENT:
        case OBJK_APPLICATION:
        case OBJK_QOSPROFILE:
        case OBJK_TYPE:
        case OBJK_TOPIC:
        case OBJK_DATAWRITER:
        case OBJK_DATAREADER:
        default:
            return false;
        break;
    }
}


Status ProxyClient::create(const CreationMode& creation_mode, const CREATE_PAYLOAD& create_payload)
{
    Status status;
    status.result().request_id(create_payload.request_id());
    status.result().status(STATUS_LAST_OP_CREATE);
    status.object_id(create_payload.object_id());

    auto internal_id = generate_object_id(create_payload.object_id(), 0x00);

    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(internal_id);
    if(object_it == objects_.end()) 
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
                delete_object(internal_id);
                create(internal_id, create_payload.object_representation());
            }
        }
        else // reuse = true
        {
            if (!creation_mode.replace()) // replace = false
            {
                // TODO(borja): Compara representaciones
            }
            else // replace = true
            {
               // TODO(borja): compara representaciones
            }
        }
    }
    return status;
}

Status ProxyClient::update(const ObjectId&  /*object_id*/, const ObjectVariant&  /*representation*/)
{
    // TODO(borja): 
}

Info ProxyClient::get_info(const ObjectId&  /*object_id*/)
{
    // TODO(borja): 
}

Status ProxyClient::delete_object(const DELETE_PAYLOAD& delete_payload)
{
    Status status;
    status.object_id(delete_payload.object_id());
    status.result().request_id(delete_payload.request_id());
    status.result().status(STATUS_LAST_OP_DELETE);
    // TODO(borja): comprobar permisos
    if (delete_object(generate_object_id(delete_payload.object_id(), 0x00)))
    {
        status.result().implementation_status(STATUS_OK);
    }
    else
    {
        status.result().implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
        // TODO(borja): en el documento se menciona STATUS_ERR_INVALID pero no existe.
    }
    return status;
}

bool ProxyClient::delete_object(const InternalObjectId& internal_object_id)
{
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto find_it = objects_.find(internal_object_id);
    if (find_it != objects_.end())
    {
        delete find_it->second;
        objects_.erase(find_it);
        return true;
    }   
    return false;    
}

Status ProxyClient::write(const ObjectId& object_id, const WRITE_DATA_PAYLOAD& data_payload)
{
    Status status;
    status.result().request_id(data_payload.request_id());
    status.result().status(STATUS_LAST_OP_WRITE);
    auto internal_id = generate_object_id(object_id, 0x00);
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(internal_id);
    if(object_it == objects_.end()) 
    {
        status.result().implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        auto* writer = dynamic_cast<DataWriter*>(object_it->second);
        writer->write(nullptr);
    }
    return status;
}

Status ProxyClient::read(const ObjectId& object_id, const READ_DATA_PAYLOAD&  data_payload)
{
    Status status;
    status.result().request_id();
    status.result().status(STATUS_LAST_OP_READ);
    auto internal_id = generate_object_id(object_id, 0x00);
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(internal_id);
    if(object_it == objects_.end()) 
    {
        status.result().implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        auto* reader = dynamic_cast<DataReader*>(object_it->second);
        if (!reader->read(data_payload))
        {
            status.result().status(STATUS_OK);
        }
    }
    return status;
}

ProxyClient::InternalObjectId ProxyClient::generate_object_id(const ObjectId& id, uint8_t suffix) const
{
    InternalObjectId internal_id{};
    std::copy(id.begin(), id.end(), internal_id.begin());
    internal_id[id.size()] = suffix;
    return internal_id;
}

void ProxyClient::store_request_info(const ObjectId& object_id, const MessageHeader& message_header)
{
    requests_info_[generate_object_id(object_id, 0x00)] = message_header;
}

void ProxyClient::remove_request_info(const ObjectId& object_id)
{
    requests_info_.erase(generate_object_id(object_id, 0x00));
}

const MessageHeader *const ProxyClient::get_request_info(const ObjectId& object_id) const
{
    try
    {
        return &requests_info_.at(generate_object_id(object_id, 0x00));
    } catch (const std::out_of_range&)
    {
        std::cerr << "Client " << object_id << "not found" << std::endl;
        return nullptr;
    }
}

uint16_t ProxyClient::sequence()
{
    return sequence_count_++;
}