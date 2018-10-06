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

#include <uxr/agent/client/ProxyClient.hpp>
#include <uxr/agent/publisher/Publisher.hpp>
#include <uxr/agent/subscriber/Subscriber.hpp>
#include <uxr/agent/datareader/DataReader.hpp>
#include <uxr/agent/datawriter/DataWriter.hpp>
#include <uxr/agent/topic/Topic.hpp>
#ifdef VERBOSE_OUTPUT
#include <uxr/agent/libdev/MessageDebugger.h>
#include <uxr/agent/libdev/MessageOutput.h>
#endif

namespace eprosima {
namespace uxr {

ProxyClient::ProxyClient(const dds::xrce::CLIENT_Representation& representation)
    : representation_(representation),
      objects_(),
      session_()
{
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
        if (!creation_mode.reuse())
        {
            if (!creation_mode.replace())
            {
                result.status(dds::xrce::STATUS_ERR_ALREADY_EXISTS);
            }
            else
            {
                lock.unlock();
                delete_object(object_id);
                lock.lock();
                if (!create_object(object_id, object_representation))
                {
                    result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
                }
            }
        }
        else
        {
            if (!creation_mode.replace())
            {
                if (object_matched(it->second, object_representation))
                {
                    result.status(dds::xrce::STATUS_OK_MATCHED);
                }
                else
                {
                    result.status(dds::xrce::STATUS_ERR_MISMATCH);
                }
            }
            else
            {
                if (object_matched(it->second, object_representation))
                {
                    result.status(dds::xrce::STATUS_OK_MATCHED);
                }
                else
                {
                    lock.unlock();
                    delete_object(object_id);
                    lock.lock();
                    if (!create_object(object_id, object_representation))
                    {
                        result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
                    }
                }
            }
        }
    }

    return result;
}

dds::xrce::ResultStatus ProxyClient::delete_object(const dds::xrce::ObjectId& object_id)
{
    dds::xrce::ResultStatus result;
    result.status(dds::xrce::STATUS_OK);
    result.implementation_status(0x00);

    std::lock_guard<std::mutex> lock(mtx_);
    auto it = objects_.find(object_id);
    if (it != objects_.end())
    {
        it->second->release(objects_);
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
    std::lock_guard<std::mutex> lock(mtx_);
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

bool ProxyClient::create_object(const dds::xrce::ObjectId& object_id, const dds::xrce::ObjectVariant& representation)
{
    bool rv = false;
    switch (representation._d())
    {
        case dds::xrce::OBJK_PARTICIPANT:
            rv = create_participant(object_id, representation.participant());
            break;
        case dds::xrce::OBJK_TOPIC:
            rv = create_topic(object_id, representation.topic());
            break;
        case dds::xrce::OBJK_PUBLISHER:
            rv = create_publisher(object_id, representation.publisher());
            break;
        case dds::xrce::OBJK_SUBSCRIBER:
            rv = create_subscriber(object_id, representation.subscriber());
            break;
        case dds::xrce::OBJK_DATAWRITER:
            rv = create_datawriter(object_id, representation.data_writer());
            break;
        case dds::xrce::OBJK_DATAREADER:
            rv = create_datareader(object_id, representation.data_reader());
            break;
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

#ifdef VERBOSE_OUTPUT
    if (rv)
    {
        std::cout << "<== ";
        debug::printl_create_submessage(representation_.client_key(), object_id, representation);
    }
#endif

    return rv;
}

bool ProxyClient::create_participant(const dds::xrce::ObjectId& object_id,
                                     const dds::xrce::OBJK_PARTICIPANT_Representation& representation)
{
    bool rv = false;
    if (dds::xrce::OBJK_PARTICIPANT == (object_id.at(1) & 0x0F))
    {
        switch (representation.representation()._d())
        {
            case dds::xrce::REPRESENTATION_BY_REFERENCE:
            {
                std::shared_ptr<Participant> participant(new Participant(object_id));
                if (participant->init_by_ref(representation.representation().object_reference()))
                {
                    rv = objects_.insert(std::make_pair(object_id, std::move(participant))).second;
                }
                break;
            }
            case dds::xrce::REPRESENTATION_AS_XML_STRING:
            {
                std::shared_ptr<Participant> participant(new Participant(object_id));
                if (participant->init_by_xml(representation.representation().xml_string_representation()))
                {
                    rv = objects_.insert(std::make_pair(object_id, std::move(participant))).second;
                }
                break;
            }
            default:
                break;
        }
    }
    return rv;
}

bool ProxyClient::create_topic(const dds::xrce::ObjectId& object_id,
                               const dds::xrce::OBJK_TOPIC_Representation& representation)
{
    bool rv = false;
    if (dds::xrce::OBJK_PARTICIPANT == (representation.participant_id()[1] & 0x0F) &&
        dds::xrce::OBJK_TOPIC == (object_id.at(1) & 0x0F))
    {
        auto it = objects_.find(representation.participant_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Participant> participant = std::dynamic_pointer_cast<Participant>(it->second);
            switch (representation.representation()._d())
            {
                case dds::xrce::REPRESENTATION_BY_REFERENCE:
                {
                    const std::string& ref_rep = representation.representation().object_reference();
                    std::shared_ptr<Topic> topic(new Topic(object_id, participant));
                    if (topic->init_by_ref(ref_rep))
                    {
                        rv = objects_.insert(std::make_pair(object_id, std::move(topic))).second;
                    }
                    break;
                }
                case dds::xrce::REPRESENTATION_AS_XML_STRING:
                {
                    const std::string& xml_rep = representation.representation().xml_string_representation();
                    std::shared_ptr<Topic> topic(new Topic(object_id, participant));
                    if (topic->init_by_xml(xml_rep))
                    {
                        rv = objects_.insert(std::make_pair(object_id, std::move(topic))).second;
                    }
                    break;
                }
                case dds::xrce::REPRESENTATION_IN_BINARY:
                    // TODO (julian).
                    break;
                default:
                    break;
            }
        }
    }
    return rv;
}

bool ProxyClient::create_publisher(const dds::xrce::ObjectId& object_id,
                                   const dds::xrce::OBJK_PUBLISHER_Representation& representation)
{
    /* Check whether participant exists. */
    bool rv = false;
    if (dds::xrce::OBJK_PARTICIPANT == (representation.participant_id()[1] & 0x0F) &&
        dds::xrce::OBJK_PUBLISHER == (object_id.at(1) & 0x0F))
    {
        auto it = objects_.find(representation.participant_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Participant> participant = std::dynamic_pointer_cast<Participant>(it->second);
            switch (representation.representation()._d())
            {
                case dds::xrce::REPRESENTATION_AS_XML_STRING:
                {
                    std::shared_ptr<Publisher> publisher(new Publisher(object_id, participant));
                    rv = objects_.insert(std::make_pair(object_id, std::move(publisher))).second;
                    break;
                }
                case dds::xrce::REPRESENTATION_IN_BINARY:
                    // TODO (julian).
                    break;
                default:
                    break;
            }
        }
    }
    return rv;
}

bool ProxyClient::create_subscriber(const dds::xrce::ObjectId& object_id,
                                    const dds::xrce::OBJK_SUBSCRIBER_Representation& representation)
{
    /* Check whether participant exists. */
    bool rv = false;
    if (dds::xrce::OBJK_PARTICIPANT == (representation.participant_id()[1] & 0x0F) &&
        dds::xrce::OBJK_SUBSCRIBER == (object_id.at(1) & 0x0F))
    {
        auto it = objects_.find(representation.participant_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Participant> participant = std::dynamic_pointer_cast<Participant>(it->second);
            switch (representation.representation()._d())
            {
                case dds::xrce::REPRESENTATION_AS_XML_STRING:
                {
                    std::shared_ptr<Subscriber> subscriber(new Subscriber(object_id, participant));
                    rv = objects_.insert(std::make_pair(object_id, std::move(subscriber))).second;
                    break;
                }
                case dds::xrce::REPRESENTATION_IN_BINARY:
                    // TODO (julian).
                    break;
                default:
                    break;
            }
        }
    }
    return rv;
}

bool ProxyClient::create_datawriter(const dds::xrce::ObjectId& object_id,
                                    const dds::xrce::DATAWRITER_Representation& representation)
{
    bool rv = false;
    if (dds::xrce::OBJK_PUBLISHER == (representation.publisher_id()[1] & 0x0F) &&
        dds::xrce::OBJK_DATAWRITER == (object_id.at(1) & 0x0F))
    {
        auto it = objects_.find(representation.publisher_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Publisher> publisher = std::dynamic_pointer_cast<Publisher>(it->second);
            switch (representation.representation()._d())
            {
                case dds::xrce::REPRESENTATION_BY_REFERENCE:
                {
                    const std::string& ref_rep = representation.representation().object_reference();
                    std::shared_ptr<DataWriter> datawriter(new DataWriter(object_id, publisher));
                    if (datawriter->init_by_ref(ref_rep, objects_))
                    {
                        rv = objects_.insert(std::make_pair(object_id, std::move(datawriter))).second;
                    }
                    break;
                }
                case dds::xrce::REPRESENTATION_AS_XML_STRING:
                {
                    const std::string& xml_rep = representation.representation().xml_string_representation();
                    std::shared_ptr<DataWriter> datawriter(new DataWriter(object_id, publisher));
                    if (datawriter->init_by_xml(xml_rep, objects_))
                    {
                        rv = objects_.insert(std::make_pair(object_id, std::move(datawriter))).second;
                    }
                    break;
                }
                case dds::xrce::REPRESENTATION_IN_BINARY:
                    // TODO (julian).
                    break;
                default:
                    break;
            }
        }
    }
    return rv;
}

bool ProxyClient::create_datareader(const dds::xrce::ObjectId& object_id,
                                    const dds::xrce::DATAREADER_Representation& representation)
{
    bool rv = false;
    if (dds::xrce::OBJK_SUBSCRIBER == (representation.subscriber_id()[1] & 0x0F) &&
        dds::xrce::OBJK_DATAREADER == (object_id.at(1) & 0x0F))
    {
        auto it = objects_.find(representation.subscriber_id());
        if (it != objects_.end())
        {
            std::shared_ptr<Subscriber> subscriber = std::dynamic_pointer_cast<Subscriber>(it->second);
            switch (representation.representation()._d())
            {
                case dds::xrce::REPRESENTATION_BY_REFERENCE:
                {
                    const std::string& ref_rep = representation.representation().object_reference();
                    std::shared_ptr<DataReader> datareader(new DataReader(object_id, subscriber));
                    if (datareader->init_by_ref(ref_rep, objects_))
                    {
                        rv = objects_.insert(std::make_pair(object_id, std::move(datareader))).second;
                    }
                    break;
                }
                case dds::xrce::REPRESENTATION_AS_XML_STRING:
                {
                    const std::string& xml_rep = representation.representation().xml_string_representation();
                    std::shared_ptr<DataReader> datareader(new DataReader(object_id, subscriber));
                    if (datareader->init_by_xml(xml_rep, objects_))
                    {
                        rv = objects_.insert(std::make_pair(object_id, std::move(datareader))).second;
                    }
                    break;
                }
                case dds::xrce::REPRESENTATION_IN_BINARY:
                    // TODO (julian).
                    break;
                default:
                    break;
            }
        }
    }
    return rv;
}

bool ProxyClient::object_matched(std::shared_ptr<XRCEObject>& old_object,
                                 const dds::xrce::ObjectVariant& new_object_rep) const
{
    bool rv = false;

    /* Check ObjectKind. */
    if ((old_object->get_id().at(1) & 0x0F) == new_object_rep._d())
    {
        switch (new_object_rep._d())
        {
            case dds::xrce::OBJK_PARTICIPANT:
            {
                std::shared_ptr<Participant> participant = std::dynamic_pointer_cast<Participant>(old_object);
                rv = participant->matched(new_object_rep.participant());
                break;
            }
            case dds::xrce::OBJK_TOPIC:
            {
                std::shared_ptr<Topic> topic = std::dynamic_pointer_cast<Topic>(old_object);
                rv = topic->matched(new_object_rep.topic());
                break;
            }
            case dds::xrce::OBJK_PUBLISHER:
            {
                std::shared_ptr<Publisher> publisher = std::dynamic_pointer_cast<Publisher>(old_object);
                rv = publisher->matched(new_object_rep.publisher());
                break;
            }
            case dds::xrce::OBJK_SUBSCRIBER:
            {
                std::shared_ptr<Subscriber> subscriber = std::dynamic_pointer_cast<Subscriber>(old_object);
                rv = subscriber->matched(new_object_rep.subscriber());
                break;
            }
            case dds::xrce::OBJK_DATAWRITER:
            {
                std::shared_ptr<DataWriter> datawriter = std::dynamic_pointer_cast<DataWriter>(old_object);
                rv = datawriter->matched(new_object_rep.data_writer());
                break;
            }
            case dds::xrce::OBJK_DATAREADER:
            {
                std::shared_ptr<DataReader> datareader = std::dynamic_pointer_cast<DataReader>(old_object);
                rv = datareader->matched(new_object_rep.data_reader());
                break;
            }
            default:
                break;
        }
    }

    return rv;
}


} // namespace uxr
} // namespace eprosima
