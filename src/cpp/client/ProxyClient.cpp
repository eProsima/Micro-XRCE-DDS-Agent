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

#include <agent/client/ProxyClient.h>

#include <agent/Root.h>
#include <agent/datareader/DataReader.h>
#include <agent/datawriter/DataWriter.h>
#include <agent/participant/Participant.h>
#include <agent/publisher/Publisher.h>
#include <agent/subscriber/Subscriber.h>
#include <agent/topic/Topic.hpp>

using eprosima::micrortps::Info;
using eprosima::micrortps::ProxyClient;
using eprosima::micrortps::ResultStatus;
using eprosima::micrortps::XRCEObject;

ProxyClient::ProxyClient(OBJK_CLIENT_Representation client, const MessageHeader& header)
    : representation_(std::move(client)), client_key(header.client_key()), session_id(header.session_id()),
      stream_id(header.stream_id())
{
}

ProxyClient::~ProxyClient()
{
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
    client_key      = std::move(x.client_key);
    session_id      = std::move(x.session_id);
    stream_id       = std::move(x.stream_id);
    return *this;
}

bool ProxyClient::create(const ObjectId& id, const ObjectVariant& representation)
{
    switch (representation._d())
    {
        case OBJECTKIND::PUBLISHER:
        {
            std::lock_guard<std::mutex> lock(objects_mutex_);
            auto object_it      = objects_.find(id);
            bool insertion_done = false;
            if (object_it == objects_.end())
            {
                insertion_done =
                    objects_.insert(std::make_pair(id, std::move(std::unique_ptr<Publisher>(new Publisher(id)))))
                        .second;
            }
            return insertion_done;
            break;
        }
        case OBJECTKIND::SUBSCRIBER:
        {
            std::lock_guard<std::mutex> lock(objects_mutex_);
            auto object_it      = objects_.find(id);
            bool insertion_done = false;
            if (object_it == objects_.end())
            {
                insertion_done =
                    objects_.insert(std::make_pair(id, std::move(std::unique_ptr<Subscriber>(new Subscriber(id)))))
                        .second;
            }
            return insertion_done;
            break;
        }
        case OBJECTKIND::PARTICIPANT:
        {
            std::lock_guard<std::mutex> lockGuard(objects_mutex_);
            auto participant    = std::unique_ptr<XRCEParticipant>(new XRCEParticipant(id));
            bool insertion_done = false;
            if (participant->init())
            {
                insertion_done = objects_.insert(std::make_pair(id, std::move(participant))).second;
            }
            return insertion_done;
            break;
        }
        case OBJECTKIND::DATAWRITER:
        {
            std::lock_guard<std::mutex> lock(objects_mutex_);

            auto participant_it = objects_.find(representation.data_writer().participant_id());
            auto publisher_it   = objects_.find(representation.data_writer().publisher_id());
            auto data_writer_it = objects_.find(id);
            bool insertion_done = false;
            if ((participant_it != objects_.end()) && (publisher_it != objects_.end()) &&
                (data_writer_it == objects_.end()))
            {
                XRCEObject* data_w = nullptr;
                switch (representation.data_writer().representation()._d())
                {
                    case REPRESENTATION_AS_XML_STRING:
                    {
                        auto participant = dynamic_cast<XRCEParticipant*>(participant_it->second.get());
                        if (participant != nullptr)
                        {
                            data_w = participant->create_writer(
                                id, representation.data_writer().representation().xml_string_representation());
                        }
                        break;
                    }
                    case REPRESENTATION_BY_REFERENCE:
                    case REPRESENTATION_IN_BINARY:
                    default:
                        return insertion_done;
                        break;
                }
                if (data_w != nullptr)
                {
                    auto publisher = dynamic_cast<Publisher*>(publisher_it->second.get());
                    if (publisher != nullptr)
                    {
                        publisher->add_writer(data_w);
                        if (!(insertion_done = objects_.insert(std::make_pair(id, std::move(std::unique_ptr<XRCEObject>(data_w)))).second))
                        {
                            delete data_w;
                        }
                    }
                }
            }
            return insertion_done;
            break;
        }
        case OBJECTKIND::DATAREADER:
        {
            std::lock_guard<std::mutex> lock(objects_mutex_);
            auto participant_it = objects_.find(representation.data_reader().participant_id());
            auto subscriber_it  = objects_.find(representation.data_reader().subscriber_id());
            auto data_reader_it = objects_.find(id);
            bool insertion_done = false;
            if ((participant_it != objects_.end()) && (subscriber_it != objects_.end()) &&
                (data_reader_it == objects_.end()))
            {
                XRCEObject* data_r = nullptr;
                switch (representation.data_reader().representation()._d())
                {
                    case REPRESENTATION_AS_XML_STRING:
                    {
                        auto participant = dynamic_cast<XRCEParticipant*>(participant_it->second.get());
                        if (participant != nullptr)
                        {
                            data_r = participant->create_reader(
                                id, representation.data_reader().representation().xml_string_representation(), this);
                        }
                        break;
                    }
                    case REPRESENTATION_BY_REFERENCE:
                    case REPRESENTATION_IN_BINARY:
                    default:
                        return insertion_done;
                        break;
                }
                if (data_r != nullptr)
                {
                    auto subscriber = dynamic_cast<Subscriber*>(subscriber_it->second.get());
                    if (subscriber != nullptr)
                    {
                        subscriber->add_reader(data_r);
                        insertion_done = objects_.insert(std::make_pair(id, std::move(std::unique_ptr<XRCEObject>(data_r)))).second;
                    }
                }
            }
            return insertion_done;
            break;
        }
        case OBJECTKIND::TOPIC:
        {
            std::lock_guard<std::mutex> lock(objects_mutex_);

            auto participant_it = objects_.find(representation.topic().participant_id());
            auto topic_it       = objects_.find(id);
            bool insertion_done = false;
            if ((participant_it != objects_.end()) && (topic_it == objects_.end()))
            {
                std::unique_ptr<XRCEObject> topic;
                switch (representation.topic().representation()._d())
                {
                    case REPRESENTATION_AS_XML_STRING:
                    {
                        auto participant = dynamic_cast<XRCEParticipant*>(participant_it->second.get());
                        if (participant != nullptr)
                        {
                            topic.reset(participant->create_topic(
                                id, representation.topic().representation().xml_string_representation()));
                        }
                        break;
                    }
                    case REPRESENTATION_BY_REFERENCE:
                    case REPRESENTATION_IN_BINARY:
                    default:
                        return insertion_done;
                        break;
                }
                if (topic)
                {
                    insertion_done = objects_.insert(std::make_pair(id, std::move(topic))).second;
                }
            }
            return insertion_done;
            break;
        }
        case OBJECTKIND::APPLICATION:
        case OBJECTKIND::QOSPROFILE:
        case OBJECTKIND::TYPE:
        default:
            return false;
            break;
    }
    return false;
}

ResultStatus ProxyClient::create(const CreationMode& creation_mode, const CREATE_Payload& create_payload)
{
    ResultStatus status;
    status.status(STATUS_LAST_OP_CREATE);
    status.request_id(create_payload.request_id());

    std::unique_lock<std::mutex> lock(objects_mutex_);
    auto object_it = objects_.find(create_payload.object_id());
    if (object_it == objects_.end())
    {
        lock.unlock();
        if (create(create_payload.object_id(), create_payload.object_representation()))
        {
            status.implementation_status(STATUS_OK);
        }
        else
        {
            status.implementation_status(STATUS_ERR_DDS_ERROR);
        }
    }
    else
    {
        lock.unlock();
        if (!creation_mode.reuse()) // reuse = false
        {
            if (!creation_mode.replace()) // replace = false
            {
                status.implementation_status(STATUS_ERR_ALREADY_EXISTS);
            }
            else // replace = true
            {
                delete_object(create_payload.object_id());
                if (create(create_payload.object_id(), create_payload.object_representation()))
                {
                    status.implementation_status(STATUS_OK);
                }
                else
                {
                    // TODO(borja): Change bool create with something handling different errors.
                    status.implementation_status(STATUS_ERR_DDS_ERROR);
                }
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
    status.request_id(delete_payload.request_id());
    // TODO(borja): comprobar permisos
    if (delete_object(delete_payload.object_id()))
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

bool ProxyClient::delete_object(const ObjectId& id)
{
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto find_it = objects_.find(id);
    if (find_it != objects_.end())
    {
        objects_.erase(find_it);
        return true;
    }
    return false;
}

XRCEObject* ProxyClient::get_object(const ObjectId& object_id)
{
    XRCEObject* object = nullptr;
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(object_id);
    if (object_it != objects_.end())
    {
        object = object_it->second.get();
    }
    return object;
}

ResultStatus ProxyClient::write(const ObjectId& object_id, const WRITE_DATA_Payload& data_payload)
{
    ResultStatus status;
    status.status(STATUS_LAST_OP_WRITE);
    status.request_id(data_payload.request_id());
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(object_id);
    if (object_it == objects_.end())
    {
        status.implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        auto writer = dynamic_cast<DataWriter*>(object_it->second.get());
        if (writer != nullptr && writer->write(data_payload))
        {
            status.implementation_status(STATUS_OK);
        }
        else
        {
            status.implementation_status(STATUS_ERR_DDS_ERROR);
        }
    }
    return status;
}

ResultStatus ProxyClient::read(const ObjectId& object_id, const READ_DATA_Payload& data_payload)
{
    ResultStatus status;
    status.status(STATUS_LAST_OP_READ);
    status.request_id(data_payload.request_id());
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(object_id);
    if (object_it == objects_.end())
    {
        status.implementation_status(STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        auto reader = dynamic_cast<DataReader*>(object_it->second.get());
        if (reader != nullptr && reader->read(data_payload) == 0)
        {
            status.implementation_status(STATUS_OK);
        }
        else
        {
            status.implementation_status(STATUS_ERR_DDS_ERROR);
        }
    }
    return status;
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
    payload.result().request_id(req_id);
    payload.object_id(object_id);
    payload.data().serialized_data(buffer);

    // TODO(borja) May cause issues. Tests created their own instance but read data will create a static one.
    root()->add_reply(message_header, payload);
}
