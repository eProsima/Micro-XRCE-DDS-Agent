// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <micrortps/agent/client/ProxyClient.hpp>
#include <micrortps/agent/Root.hpp>

namespace eprosima {
namespace micrortps {

ProxyClient::ProxyClient(const dds::xrce::CLIENT_Representation& representation)
    : representation_(std::move(representation)),
      objects_(),
      session_()
{
}

bool ProxyClient::create_object(const dds::xrce::ObjectId& object_id, const dds::xrce::ObjectVariant& representation)
{
    bool result = false;
    switch (representation._d())
    {
        case dds::xrce::OBJK_PUBLISHER:
        {
            /* Check whether participant exists */
            auto participant_it = objects_.find(representation.publisher().participant_id());
            if (participant_it != objects_.end() &&
                ((representation.publisher().participant_id()[1] & 0x0F) == dds::xrce::OBJK_PARTICIPANT))
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
            if (participant_it != objects_.end() &&
                ((representation.subscriber().participant_id()[1] & 0x0F) == dds::xrce::OBJK_PARTICIPANT))
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
            if ((publisher_it != objects_.end()) &&
                ((representation.data_writer().publisher_id()[1] & 0x0F) == dds::xrce::OBJK_PUBLISHER))
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
            if ((subscriber_it != objects_.end()) &&
                ((representation.data_reader().subscriber_id()[1] & 0x0F) == dds::xrce::OBJK_SUBSCRIBER))
            {
                Subscriber* subscriber = dynamic_cast<Subscriber*>(subscriber_it->second.get());
                XRCEObject* data_reader = nullptr;
                switch (representation.data_reader().representation()._d())
                {
                    case dds::xrce::REPRESENTATION_AS_XML_STRING:
                    {
                        XRCEParticipant& participant = subscriber->get_participant();
                        data_reader =
                                participant.create_reader(object_id, representation.data_reader().representation().string_representation());
                        break;
                    }
                    case dds::xrce::REPRESENTATION_IN_BINARY:
                        // TODO (Julian).
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
            if ((participant_it != objects_.end()) &&
                ((representation.topic().participant_id()[1] & 0x0F) == dds::xrce::OBJK_PARTICIPANT))
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
    std::unique_lock<std::mutex> lock(mtx_);
    auto it = objects_.find(object_id);
    bool exists = (it != objects_.end());

    /* Create object according with creation mode (see Table 7 XRCE). */
    if (!exists)
    {
        if (!create_object(object_id, object_representation))
        {
            result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        }
    }
    else
    {
        if (creation_mode.replace())
        {
            objects_.erase(object_id);
            if (!create_object(object_id, object_representation))
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

    std::unique_lock<std::mutex> lock(mtx_);
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
    std::lock_guard<std::mutex> lockGuard(mtx_);
    auto object_it = objects_.find(object_id);
    if (object_it != objects_.end())
    {
        object = object_it->second.get();
    }
    return object;
}

Session& ProxyClient::session()
{
    return session_;
}

} // namespace micrortps
} // namespace eprosima
