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

ProxyClient::ProxyClient(dds::xrce::CLIENT_Representation client,
                         const dds::xrce::ClientKey& client_key,
                         const dds::xrce::SessionId& session_id,
                         uint32_t addr, uint16_t port)
    : representation_(std::move(client)),
      objects_(),
      client_key_(client_key),
      session_id_(session_id),
      addr_(addr),
      port_(port),
      streams_manager_()
{
}

ProxyClient::~ProxyClient()
{
}

ProxyClient::ProxyClient(ProxyClient&& x) noexcept
    : representation_(std::move(x.representation_)),
      objects_(std::move(x.objects_)),
      client_key_(x.client_key_),
      session_id_(x.session_id_),
      streams_manager_(std::move(x.streams_manager_))
{
}

ProxyClient& ProxyClient::operator=(ProxyClient&& x) noexcept
{
    representation_  = std::move(x.representation_);
    objects_         = std::move(x.objects_);
    client_key_      = std::move(x.client_key_);
    session_id_      = std::move(x.session_id_);
    streams_manager_ = std::move(x.streams_manager_);
    return *this;
}

bool ProxyClient::create(const dds::xrce::ObjectId& object_id, const dds::xrce::ObjectVariant& representation)
{
    bool result = false;
    switch (representation._d())
    {
        case dds::xrce::OBJK_PUBLISHER:
        {
            /* Check whether participant exists */
            auto participant_it = objects_.find(representation.publisher().participant_id());
            if (participant_it != objects_.end())
            {
                XRCEParticipant* participant = dynamic_cast<XRCEParticipant*>(participant_it->second.get());
                std::unique_ptr<Publisher> publisher(new Publisher(object_id, *participant));
                result = objects_.insert(std::make_pair(object_id, std::move(publisher))).second;
            }
            break;
        }
        case dds::xrce::OBJK_SUBSCRIBER:
        {
            /* Check whether participant exists */
            auto participant_it = objects_.find(representation.subscriber().participant_id());
            if (participant_it != objects_.end())
            {
                XRCEParticipant* participant = dynamic_cast<XRCEParticipant*>(participant_it->second.get());
                std::unique_ptr<Subscriber> subscriber(new Subscriber(object_id, *participant));
                result = objects_.insert(std::make_pair(object_id, std::move(subscriber))).second;
            }
            break;
        }
        case dds::xrce::OBJK_PARTICIPANT:
        {
            auto participant = std::unique_ptr<XRCEParticipant>(new XRCEParticipant(object_id));
            if (participant->init())
            {
                result = objects_.insert(std::make_pair(object_id, std::move(participant))).second;
            }
            break;
        }
        case dds::xrce::OBJK_DATAWRITER:
        {
            auto publisher_it   = objects_.find(representation.data_writer().publisher_id());
            if ((publisher_it != objects_.end()))
            {
                Publisher* publisher = dynamic_cast<Publisher*>(publisher_it->second.get());
                XRCEObject* data_writer = nullptr;
                switch (representation.data_writer().representation()._d())
                {
                    case dds::xrce::REPRESENTATION_AS_XML_STRING:
                    {
                        XRCEParticipant& participant = publisher->get_participant();
                        data_writer =
                                participant.create_writer(object_id, representation.data_writer().representation().string_representation());
                        break;
                    }
                    case dds::xrce::REPRESENTATION_IN_BINARY:
                        break;
                    default:
                        break;
                }
                if (data_writer != nullptr)
                {
                    if (publisher != nullptr)
                    {
                        result = objects_.insert(std::make_pair(object_id, std::move(std::unique_ptr<XRCEObject>(data_writer)))).second;
                        if (!result)
                        {
                            delete data_writer;
                        }
                    }
                }
            }
            break;
        }
        case dds::xrce::OBJK_DATAREADER:
        {
            auto subscriber_it  = objects_.find(representation.data_reader().subscriber_id());
            if ((subscriber_it != objects_.end()))
            {
                Subscriber* subscriber = dynamic_cast<Subscriber*>(subscriber_it->second.get());
                XRCEObject* data_reader = nullptr;
                switch (representation.data_reader().representation()._d())
                {
                    case dds::xrce::REPRESENTATION_AS_XML_STRING:
                    {
                        XRCEParticipant& participant = subscriber->get_participant();
                        data_reader =
                                participant.create_reader(object_id, representation.data_reader().representation().string_representation(), this);
                        break;
                    }
                    case dds::xrce::REPRESENTATION_IN_BINARY:
                    /* TODO (Julian). */
                        break;
                    default:
                        break;
                }
                if (data_reader != nullptr)
                {
                    if (subscriber != nullptr)
                    {
                        result = objects_.insert(std::make_pair(object_id, std::move(std::unique_ptr<XRCEObject>(data_reader)))).second;
                        if (!result)
                        {
                            delete data_reader;
                        }
                    }
                }
            }
            break;
        }
        case dds::xrce::OBJK_TOPIC:
        {
            auto participant_it = objects_.find(representation.topic().participant_id());
            if ((participant_it != objects_.end()))
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
                                object_id, representation.topic().representation().xml_string_representation()));
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
                        break;
                }
                if (topic)
                {
                    result = objects_.insert(std::make_pair(object_id, std::move(topic))).second;
                }
            }
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
            break;
    }
    return result;
}

dds::xrce::ResultStatus ProxyClient::create(const dds::xrce::CreationMode& creation_mode,
                                            const dds::xrce::ObjectPrefix& objectid_prefix,
                                            const dds::xrce::ObjectVariant& object_representation)
{
    dds::xrce::ResultStatus result;
    result.status(dds::xrce::STATUS_OK);
    result.implementation_status(0x00);

    /* Compute ObjectId. */
    dds::xrce::ObjectId object_id;
    object_id[0] = objectid_prefix[0];
    object_id[1] = (objectid_prefix[1] & 0xF0) | object_representation._d();

    /* Check whether object exists. */
    std::unique_lock<std::mutex> lock(objects_mutex_);
    auto it = objects_.find(object_id);
    bool exists = (it != objects_.end());

    /* Create object according with creation mode (see Table 7 XRCE). */
    if (!exists)
    {
        if (!create(object_id, object_representation))
        {
            result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        }
    }
    else
    {
        if (creation_mode.replace())
        {
            objects_.erase(object_id);
            if (!create(object_id, object_representation))
            {
                result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
            }
        }
        else
        {
            result.status(dds::xrce::STATUS_ERR_ALREADY_EXISTS);
        }
    }

    return result;
}

dds::xrce::ResultStatus ProxyClient::delete_object(const dds::xrce::ObjectId& object_id)
{
    dds::xrce::ResultStatus result;
    result.status(dds::xrce::STATUS_OK);
    result.implementation_status(0x00);

    std::unique_lock<std::mutex> lock(objects_mutex_);
    auto it = objects_.find(object_id);
    if (it != objects_.end())
    {
        objects_.erase(object_id);
    }
    else
    {
        result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }

    return result;
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

void ProxyClient::on_read_data(const dds::xrce::StreamId& stream_id,
                               const dds::xrce::ObjectId& object_id,
                               const dds::xrce::RequestId& request_id,
                               const std::vector<unsigned char>& buffer)
{
    /* Message header. */
    dds::xrce::MessageHeader message_header;
    message_header.client_key(client_key_);
    message_header.session_id(session_id_);
    message_header.stream_id(stream_id);
    uint16_t seq_num = streams_manager_.next_ouput_message(stream_id);
    message_header.sequence_nr(seq_num);

    /* Payload. */
    dds::xrce::DATA_Payload_Data payload;
    payload.request_id(request_id);
    payload.object_id(object_id);
    payload.data().serialized_data(buffer);

    /* Serialize message. */
    Message message{};
    XRCEFactory message_creator{message.get_buffer().data(), message.get_buffer().max_size()};
    message_creator.header(message_header);
    message_creator.data(payload);
    message.set_real_size(message_creator.get_total_size());
    message.set_addr(addr_);
    message.set_port(port_);

    /* Store message. */
    streams_manager_.store_output_message(stream_id, message.get_buffer().data(), message.get_real_size());

    /* Send message. */
    root().add_reply(message);
}

StreamsManager& ProxyClient::stream_manager()
{
    return streams_manager_;
}

} // namespace micrortps
} // namespace eprosima
