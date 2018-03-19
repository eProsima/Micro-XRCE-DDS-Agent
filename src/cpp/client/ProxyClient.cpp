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

namespace eprosima {
namespace micrortps {

ProxyClient::ProxyClient(dds::xrce::CLIENT_Representation client, const dds::xrce::MessageHeader& header)
    : representation_(std::move(client)),
      client_key(header.client_key()),
      session_id(header.session_id()),
      streams_manager_()
{
}

ProxyClient::~ProxyClient()
{
}

ProxyClient::ProxyClient(ProxyClient&& x) noexcept
    : representation_(std::move(x.representation_)),
      objects_(std::move(x.objects_)),
      client_key(x.client_key),
      session_id(x.session_id),
      streams_manager_(std::move(x.streams_manager_))
{
}

ProxyClient& ProxyClient::operator=(ProxyClient&& x) noexcept
{
    representation_  = std::move(x.representation_);
    objects_         = std::move(x.objects_);
    client_key       = std::move(x.client_key);
    session_id       = std::move(x.session_id);
    streams_manager_ = std::move(x.streams_manager_);
    return *this;
}

bool ProxyClient::create(const dds::xrce::ObjectId& id, const dds::xrce::ObjectVariant& representation)
{
    /* Take ownership of the mutex. */
    std::lock_guard<std::mutex> lock(objects_mutex_);

    switch (representation._d())
    {
        case dds::xrce::OBJK_PUBLISHER:
        {
            bool result = false;

            /* Check whether participant exists */
            auto participant_it = objects_.find(representation.publisher().participant_id());
            if (participant_it != objects_.end())
            {
                /* Check whether object exists */
                auto object_it = objects_.find(id);
                if (object_it == objects_.end())
                {
                    std::unique_ptr<Publisher> publisher(new Publisher(id, *participant_it->second.get()));
                    result = objects_.insert(std::make_pair(id, std::move(publisher))).second;
                }
                else
                {
                    /* TODO (Julian): take into account replace and reuse flags in case of existence */
                }
            }
            return result;
            break;
        }
        case dds::xrce::OBJK_SUBSCRIBER:
        {
            bool result = false;

            /* Check whether participant exists */
            auto participant_it = objects_.find(representation.subscriber().participant_id());
            if (participant_it != objects_.end())
            {
                /* Check whether object exists */
                auto object_it = objects_.find(id);
                if (object_it == objects_.end())
                {
                    std::unique_ptr<Subscriber> subscriber(new Subscriber(id, *participant_it->second.get()));
                    result = objects_.insert(std::make_pair(id, std::move(subscriber))).second;
                }
                else
                {
                    /* TODO (Julian): take into account replace and reuse flags in case of existence */
                }
            }
            return result;
            break;
        }
        case dds::xrce::OBJK_PARTICIPANT:
        {
            auto participant    = std::unique_ptr<XRCEParticipant>(new XRCEParticipant(id));
            bool insertion_done = false;
            if (participant->init())
            {
                insertion_done = objects_.insert(std::make_pair(id, std::move(participant))).second;
            }
            return insertion_done;
            break;
        }
        case dds::xrce::OBJK_DATAWRITER:
        {
            auto publisher_it   = objects_.find(representation.data_writer().publisher_id());
            auto data_writer_it = objects_.find(id);
            bool insertion_done = false;
            if ((publisher_it != objects_.end()) && (data_writer_it == objects_.end()))
            {
                Publisher* publisher = dynamic_cast<Publisher*>(publisher_it->second.get());
                XRCEObject* data_writer = nullptr;
                switch (representation.data_writer().representation()._d())
                {
                    case dds::xrce::REPRESENTATION_AS_XML_STRING:
                    {
                        XRCEParticipant& participant = dynamic_cast<XRCEParticipant&>(publisher->get_participant());
                        data_writer =
                                participant.create_writer(id, representation.data_writer().representation().string_representation());
                        break;
                    }
                    case dds::xrce::REPRESENTATION_IN_BINARY:
                        break;
                    default:
                        return insertion_done;
                        break;
                }
                if (data_writer != nullptr)
                {
                    auto publisher = dynamic_cast<Publisher*>(publisher_it->second.get());
                    if (publisher != nullptr)
                    {
                        if (!(insertion_done = objects_.insert(std::make_pair(id, std::move(std::unique_ptr<XRCEObject>(data_writer)))).second))
                        {
                            delete data_writer;
                        }
                    }
                }
            }
            return insertion_done;
            break;
        }
        case dds::xrce::OBJK_DATAREADER:
        {
            auto subscriber_it  = objects_.find(representation.data_reader().subscriber_id());
            auto data_reader_it = objects_.find(id);
            bool insertion_done = false;
            if ((subscriber_it != objects_.end()) && (data_reader_it == objects_.end()))
            {
                Subscriber* subscriber = dynamic_cast<Subscriber*>(subscriber_it->second.get());
                XRCEObject* data_reader = nullptr;
                switch (representation.data_reader().representation()._d())
                {
                    case dds::xrce::REPRESENTATION_AS_XML_STRING:
                    {
                        XRCEParticipant& participant = dynamic_cast<XRCEParticipant&>(subscriber->get_participant());
                        data_reader =
                                participant.create_reader(id, representation.data_reader().representation().string_representation(), this);
                        break;
                    }
                    case dds::xrce::REPRESENTATION_IN_BINARY:
                    /* TODO (Julian). */
                        break;
                    default:
                        return insertion_done;
                        break;
                }
                if (data_reader != nullptr)
                {
                    auto subscriber = dynamic_cast<Subscriber*>(subscriber_it->second.get());
                    if (subscriber != nullptr)
                    {
                        insertion_done = objects_.insert(std::make_pair(id, std::move(std::unique_ptr<XRCEObject>(data_reader)))).second;
                    }
                }
            }
            return insertion_done;
            break;
        }
        case dds::xrce::OBJK_TOPIC:
        {
            auto participant_it = objects_.find(representation.topic().participant_id());
            auto topic_it       = objects_.find(id);
            bool insertion_done = false;
            if ((participant_it != objects_.end()) && (topic_it == objects_.end()))
            {
                std::unique_ptr<XRCEObject> topic;
                switch (representation.topic().representation()._d())
                {
                    case dds::xrce::REPRESENTATION_AS_XML_STRING:
                    {
                        auto participant = dynamic_cast<XRCEParticipant*>(participant_it->second.get());
                        if (participant != nullptr)
                        {
                            topic.reset(participant->create_topic(
                                id, representation.topic().representation().xml_string_representation()));
                        }
                        break;
                    }
                    case dds::xrce::REPRESENTATION_BY_REFERENCE:
                    /* TODO (Julian). */
                        break;
                    case dds::xrce::REPRESENTATION_IN_BINARY:
                    /* TODO (Julian). */
                        break;
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
        case dds::xrce::OBJK_APPLICATION:
            /* TODO (Julian). */
            break;
        case dds::xrce::OBJK_QOSPROFILE:
            /* TODO (Julian). */
            break;
        case dds::xrce::OBJK_TYPE:
            /* TODO (Julian). */
            break;
        default:
            return false;
    }
    return false;
}

dds::xrce::ResultStatus ProxyClient::create(const dds::xrce::CreationMode& creation_mode,
                                            const dds::xrce::CREATE_Payload& create_payload)
{
    dds::xrce::ResultStatus result_status;

    std::unique_lock<std::mutex> lock(objects_mutex_);
    auto object_it = objects_.find(create_payload.object_id());
    if (object_it == objects_.end())
    {
        lock.unlock();
        if (create(create_payload.object_id(), create_payload.object_representation()))
        {
            result_status.status(dds::xrce::STATUS_OK);
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
        }
    }
    else
    {
        lock.unlock();
        if (!creation_mode.reuse()) // reuse = false
        {
            if (!creation_mode.replace()) // replace = false
            {
                result_status.status(dds::xrce::STATUS_ERR_ALREADY_EXISTS);
            }
            else // replace = true
            {
                delete_object(create_payload.object_id());
                if (create(create_payload.object_id(), create_payload.object_representation()))
                {
                    result_status.status(dds::xrce::STATUS_OK);
                }
                else
                {
                    // TODO(Borja): Change bool create with something handling different errors.
                    result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
                }
            }
        }
        else // reuse = true
        {
            if (!creation_mode.replace()) // replace = false
            {
                // TODO (Borja): compare representations.
            }
            else // replace = true
            {
                // TODO (Borja): compare representations.
            }
        }
    }
    return result_status;
}

dds::xrce::ResultStatus ProxyClient::update(const dds::xrce::ObjectId& /*object_id*/,
                                            const dds::xrce::ObjectVariant& /*representation*/)
{
    // TODO (Borja):
    return dds::xrce::ResultStatus{};
}

dds::xrce::ObjectInfo ProxyClient::get_info(const dds::xrce::ObjectId& /*object_id*/)
{
    // TODO (Borja):
    return dds::xrce::ObjectInfo{};
}

dds::xrce::ResultStatus ProxyClient::delete_object(const dds::xrce::DELETE_Payload& delete_payload)
{
    dds::xrce::ResultStatus result_status;
    // TODO (Borja): check permissions.
    if (delete_object(delete_payload.object_id()))
    {
        result_status.status(dds::xrce::STATUS_OK);
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }
    return result_status;
}

bool ProxyClient::delete_object(const dds::xrce::ObjectId& id)
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

XRCEObject* ProxyClient::get_object(const dds::xrce::ObjectId& object_id)
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

dds::xrce::ResultStatus ProxyClient::write(const dds::xrce::ObjectId& object_id,
                                           dds::xrce::WRITE_DATA_Payload_Data& data_payload)
{
    dds::xrce::ResultStatus result_status;
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(object_id);
    if (object_it == objects_.end())
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        auto writer = dynamic_cast<DataWriter*>(object_it->second.get());
        if (writer != nullptr && writer->write(data_payload))
        {
            result_status.status(dds::xrce::STATUS_OK);
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
        }
    }
    return result_status;
}

dds::xrce::ResultStatus ProxyClient::read(const dds::xrce::ObjectId& object_id,
                                          const dds::xrce::READ_DATA_Payload& data_payload)
{
    dds::xrce::ResultStatus result_status;
    std::lock_guard<std::mutex> lockGuard(objects_mutex_);
    auto object_it = objects_.find(object_id);
    if (object_it == objects_.end())
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }
    else
    {
        auto reader = dynamic_cast<DataReader*>(object_it->second.get());
        if (reader != nullptr && reader->read(data_payload) == 0)
        {
            result_status.status(dds::xrce::STATUS_OK);
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
        }
    }
    return result_status;
}

void ProxyClient::on_read_data(const dds::xrce::ObjectId& object_id,
                               const dds::xrce::RequestId& req_id,
                               const std::vector<unsigned char>& buffer)
{
    dds::xrce::MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
//    message_header.stream_id(stream_id);

    dds::xrce::DATA_Payload_Data payload;
    payload.request_id(req_id);
    payload.object_id(object_id);
    payload.data().serialized_data(buffer);

    root()->add_reply(message_header, payload);
}

} // namespace micrortps
} // namespace eprosima
