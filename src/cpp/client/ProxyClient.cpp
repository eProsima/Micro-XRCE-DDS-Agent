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

#include "agent/Root.h"
#include "agent/datareader/DataReader.h"
#include "agent/datawriter/DataWriter.h"
#include "agent/participant/Participant.h"
#include "agent/publisher/Publisher.h"
#include "agent/subscriber/Subscriber.h"

// #include "libdev/MessageDebugger.h"

#include "agent/Root.h"

using eprosima::micrortps::Info;
using eprosima::micrortps::ProxyClient;
using eprosima::micrortps::ResultStatus;

ProxyClient::ProxyClient(OBJK_CLIENT_Representation client, const MessageHeader& header)
    : representation_(std::move(client)), client_key(header.client_key()), session_id(header.session_id()),
      stream_id(header.stream_id())
{
}

ProxyClient::~ProxyClient()
{
    for(auto& xrce_object : objects_)
    {
        delete xrce_object.second;
    }
}

ProxyClient::ProxyClient(ProxyClient&& x) noexcept
    : representation_(std::move(x.representation_)), objects_(std::move(x.objects_)), client_key(x.client_key),
      session_id(x.session_id), stream_id(x.stream_id)
{
}

ProxyClient& ProxyClient::operator=(ProxyClient&& x) noexcept
{
    representation_ = std::move(x.representation_);
    objects_        = std::move(x.objects_);
    return *this;
}

bool ProxyClient::create(const InternalObjectId& internal_object_id, const ObjectVariant& representation)
{
    switch(representation._d())
    {
        case OBJK_PUBLISHER:
        {
            std::unique_lock<std::mutex> lock(objects_mutex_);
            auto object_it      = objects_.find(internal_object_id);
            bool insertion_done = false;
            if(object_it == objects_.end())
            {
                insertion_done = objects_.insert(std::make_pair(internal_object_id, new Publisher())).second;
            }
            return insertion_done;
            break;
        }
        case OBJK_SUBSCRIBER:
        {
            std::unique_lock<std::mutex> lock(objects_mutex_);
            auto object_it      = objects_.find(internal_object_id);
            bool insertion_done = false;
            if(object_it == objects_.end())
            {
                insertion_done = objects_.insert(std::make_pair(internal_object_id, new Subscriber())).second;
            }
            return insertion_done;
            break;
        }
        case OBJK_PARTICIPANT:
        {
            std::lock_guard<std::mutex> lockGuard(objects_mutex_);
            return objects_.insert(std::make_pair(internal_object_id, new eprosima::micrortps::XRCEParticipant())).second;
            break;
        }
        case OBJK_DATAWRITER:
        {
            std::unique_lock<std::mutex> lock(objects_mutex_);

            auto participant_it =
                objects_.find(generate_object_id(representation.data_writer().participant_id(), 0x00));
            auto publisher_it   = objects_.find(generate_object_id(representation.data_writer().publisher_id(), 0x00));
            auto data_writer_it = objects_.find(internal_object_id);
            bool insertion_done = false;
            if((participant_it != objects_.end()) && (publisher_it != objects_.end()) &&
               (data_writer_it == objects_.end()))
            {
                auto data_w = dynamic_cast<XRCEParticipant*>(participant_it->second)->create_writer();
                dynamic_cast<Publisher*>(publisher_it->second)->add_writer(data_w);
                insertion_done = objects_.insert(std::make_pair(internal_object_id, data_w)).second;
            }
            return insertion_done;
            break;
        }
        case OBJK_DATAREADER:
        {
            std::unique_lock<std::mutex> lock(objects_mutex_);
            auto participant_it =
                objects_.find(generate_object_id(representation.data_reader().participant_id(), 0x00));
            auto subscriber_it  = objects_.find(generate_object_id(representation.data_reader().subscriber_id(), 0x00));
            auto data_writer_it = objects_.find(internal_object_id);
            bool insertion_done = false;
            if((participant_it != objects_.end()) && (subscriber_it != objects_.end()) &&
               (data_writer_it == objects_.end()))
            {
                auto data_r = dynamic_cast<XRCEParticipant*>(participant_it->second)->create_reader(this);
                dynamic_cast<Subscriber*>(subscriber_it->second)->add_reader(data_r);
                insertion_done = objects_.insert(std::make_pair(internal_object_id, data_r)).second;
            }
            return insertion_done;
            break;
        }
        case OBJK_APPLICATION:
        case OBJK_QOSPROFILE:
        case OBJK_TYPE:
        case OBJK_TOPIC:
        default:
            return false;
            break;
    }
}

ResultStatus ProxyClient::create(const CreationMode& creation_mode, const CREATE_Payload& create_payload)
{
    ResultStatus status;
    status.status(STATUS_LAST_OP_CREATE);
    auto internal_id = generate_object_id(create_payload.object_id(), 0x00);

    std::unique_lock<std::mutex> lock(objects_mutex_);
    auto object_it = objects_.find(internal_id);
    if(object_it == objects_.end())
    {
        lock.unlock();
        if(create(internal_id, create_payload.object_representation()))
        {
            status.implementation_status(STATUS_OK);
        }
    }
    else
    {
        lock.unlock();
        if(!creation_mode.reuse()) // reuse = false
        {
            if(!creation_mode.replace()) // replace = false
            {
                status.implementation_status(STATUS_ERR_ALREADY_EXISTS);
            }
            else // replace = true
            {
                delete_object(internal_id);
                create(internal_id, create_payload.object_representation());
            }
        }
        else // reuse = true
        {
            if(!creation_mode.replace()) // replace = false
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

ResultStatus ProxyClient::update(const ObjectId& /*object_id*/, const ObjectVariant& /*representation*/)
{
    // TODO(borja):
    return ResultStatus{};
}

Info ProxyClient::get_info(const ObjectId& /*object_id*/)
{
    // TODO(borja):
    return Info{};
}

ResultStatus ProxyClient::delete_object(const DELETE_RESOURCE_Payload& delete_payload)
{
    ResultStatus status;
    status.status(STATUS_LAST_OP_DELETE);
    // TODO(borja): comprobar permisos
    if(delete_object(generate_object_id(delete_payload.object_id(), 0x00)))
    {
        status.implementation_status(STATUS_OK);
    }
    else
    {
        status.implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
        // TODO(borja): en el documento se menciona STATUS_ERR_INVALID pero no existe.
    }
    return status;
}

bool ProxyClient::delete_object(const InternalObjectId& internal_object_id)
{
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto find_it = objects_.find(internal_object_id);
    if(find_it != objects_.end())
    {
        delete find_it->second;
        objects_.erase(find_it);
        return true;
    }
    return false;
}

ResultStatus ProxyClient::write(const ObjectId& object_id, const WRITE_DATA_Payload& data_payload)
{
    ResultStatus status;
    status.status(STATUS_LAST_OP_WRITE);
    auto internal_id = generate_object_id(object_id, 0x00);
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(internal_id);
    if(object_it == objects_.end())
    {
        status.implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        dynamic_cast<DataWriter*>(dynamic_cast<Publisher*>(object_it->second)->get_writer())->write(data_payload);
    }
    return status;
}

ResultStatus ProxyClient::read(const ObjectId& object_id, const READ_DATA_Payload& data_payload)
{
    ResultStatus status;
    status.status(STATUS_LAST_OP_READ);
    auto internal_id = generate_object_id(object_id, 0x00);
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(internal_id);
    if(object_it == objects_.end())
    {
        status.implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        if(!(dynamic_cast<DataReader*>(dynamic_cast<Subscriber*>(object_it->second)->get_reader())
                 ->read(data_payload) == 0))
        {
            status.implementation_status(STATUS_OK);
        }
    }
    return status;
}

ProxyClient::InternalObjectId ProxyClient::generate_object_id(const ObjectId& id, uint8_t suffix) const
{
    InternalObjectId internal_id{};
    std::copy(id.begin(), id.end(), internal_id.begin());
    try
    {
        internal_id.at(internal_id.size() - 1) = suffix;
    }
    catch(std::out_of_range const& exc)
    {
        std::cerr << "Impossible to generate internal object ID: " << exc.what() << std::endl;
    }
    return internal_id;
}

void ProxyClient::on_read_data(const ObjectId& object_id, const RequestId& req_id,
                               const std::vector<unsigned char>& buffer)
{
    // printf("on_read_data\n");

    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);

    DATA_Payload_Data payload;
    payload.request_id(req_id);
    payload.object_id(object_id);
    payload.data().serialized_data(buffer);

    root()->add_reply(message_header, payload);
}
