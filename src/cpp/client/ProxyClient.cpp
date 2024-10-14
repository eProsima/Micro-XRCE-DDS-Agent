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
#include <uxr/agent/requester/Requester.hpp>
#include <uxr/agent/replier/Replier.hpp>
#include <uxr/agent/topic/Topic.hpp>
#include <uxr/agent/logger/Logger.hpp>

#ifdef UAGENT_FAST_PROFILE
#include <uxr/agent/middleware/fastdds/FastDDSMiddleware.hpp>
#endif

#ifdef UAGENT_CED_PROFILE
#include <uxr/agent/middleware/ced/CedMiddleware.hpp>
#endif

namespace eprosima {
namespace uxr {

ProxyClient::ProxyClient(
        const dds::xrce::CLIENT_Representation& representation,
        Middleware::Kind middleware_kind,
        std::unordered_map<std::string, std::string>&& properties)
    : representation_(representation)
    , objects_()
    , session_(SessionInfo{representation.client_key(), representation.session_id(), representation.mtu()})
    , state_{State::alive}
    , timestamp_{std::chrono::steady_clock::now()}
    , properties_(std::move(properties))
    , client_dead_time_(CLIENT_DEAD_TIME)
    , hard_liveliness_check_(false)
{
    switch (middleware_kind)
    {
        case Middleware::Kind::NONE:
        {
            UXR_AGENT_LOG_CRITICAL(
                UXR_DECORATE_RED("no Middleware selected"),
                UXR_CLIENT_KEY_PATTERN,
                conversion::clientkey_to_raw(representation.client_key()));
        }
#ifdef UAGENT_FAST_PROFILE
        case Middleware::Kind::FASTDDS:
        {
            bool intraprocess_enabled =
                properties_.find("uxr_sm") != properties_.end() &&
                properties_["uxr_sm"] == "1";
            middleware_.reset(new FastDDSMiddleware(intraprocess_enabled));
            break;
        }
#endif
#ifdef UAGENT_CED_PROFILE
        case Middleware::Kind::CED:
        {
            middleware_.reset(new CedMiddleware(conversion::clientkey_to_raw(representation.client_key())));
            break;
        }
#endif
    }
    hard_liveliness_check_ = properties_.find("uxr_hl") != properties_.end();
    if (hard_liveliness_check_) {
        client_dead_time_ = std::chrono::milliseconds(std::stoi(properties_["uxr_hl"]));
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("session hard timeout enabled"),
            "client_key: 0x{:08X}, timeout: {} ms",
            conversion::clientkey_to_raw(representation.client_key()),
            std::stoi(properties_["uxr_hl"]));
    }
}

dds::xrce::ResultStatus ProxyClient::create_object(
        const dds::xrce::CreationMode& creation_mode,
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
        create_object(object_id, object_representation, result);
    }
    else
    {
        if (!creation_mode.reuse())
        {
            if (!creation_mode.replace())
            {
                result.status(dds::xrce::STATUS_ERR_ALREADY_EXISTS);
                UXR_AGENT_LOG_DEBUG(
                    UXR_DECORATE_RED("already exists"),
                    UXR_CREATE_OBJECT_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id));
            }
            else
            {
                delete_object_unlock(object_id);
                create_object(object_id, object_representation, result);
            }
        }
        else
        {
            if (!creation_mode.replace())
            {
                if (it->second->matched(object_representation))
                {
                    result.status(dds::xrce::STATUS_OK_MATCHED);
                    UXR_AGENT_LOG_DEBUG(
                        UXR_DECORATE_GREEN("objects matched"),
                        UXR_CREATE_OBJECT_PATTERN,
                        conversion::clientkey_to_raw(representation_.client_key()),
                        conversion::objectid_to_raw(object_id));
                }
                else
                {
                    result.status(dds::xrce::STATUS_ERR_MISMATCH);
                    UXR_AGENT_LOG_DEBUG(
                        UXR_DECORATE_RED("objects mismatch"),
                        UXR_CREATE_OBJECT_PATTERN,
                        conversion::clientkey_to_raw(representation_.client_key()),
                        conversion::objectid_to_raw(object_id));
                }
            }
            else
            {
                if (it->second->matched(object_representation))
                {
                    result.status(dds::xrce::STATUS_OK_MATCHED);
                    UXR_AGENT_LOG_DEBUG(
                        UXR_DECORATE_GREEN("objects matched"),
                        UXR_CREATE_OBJECT_PATTERN,
                        conversion::clientkey_to_raw(representation_.client_key()),
                        conversion::objectid_to_raw(object_id));
                }
                else
                {
                    delete_object_unlock(object_id);
                    create_object(object_id, object_representation, result);
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
    if (!delete_object_unlock(object_id))
    {
        result.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
    }
    return result;
}

dds::xrce::ResultStatus ProxyClient::update(
        const dds::xrce::ObjectId& /*object_id*/,
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

std::shared_ptr<XRCEObject> ProxyClient::get_object(const dds::xrce::ObjectId& object_id)
{
    std::shared_ptr<XRCEObject> object;
    std::lock_guard<std::mutex> lock(mtx_);
    auto object_it = objects_.find(object_id);
    if (object_it != objects_.end())
    {
        object = object_it->second;
    }
    return object;
}

void ProxyClient::release()
{
    objects_.clear();
}

Session& ProxyClient::session()
{
    return session_;
}

bool ProxyClient::create_object(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::ObjectVariant& representation,
        dds::xrce::ResultStatus& result_status)
{
    bool rv = false;
    switch (representation._d())
    {
        case dds::xrce::OBJK_PARTICIPANT:
            rv = create_participant(object_id, representation.participant(), result_status);
            break;
        case dds::xrce::OBJK_TOPIC:
            rv = create_topic(object_id, representation.topic(), result_status);
            break;
        case dds::xrce::OBJK_PUBLISHER:
            rv = create_publisher(object_id, representation.publisher(), result_status);
            break;
        case dds::xrce::OBJK_SUBSCRIBER:
            rv = create_subscriber(object_id, representation.subscriber(), result_status);
            break;
        case dds::xrce::OBJK_DATAWRITER:
            rv = create_datawriter(object_id, representation.data_writer(), result_status);
            break;
        case dds::xrce::OBJK_DATAREADER:
            rv = create_datareader(object_id, representation.data_reader(), result_status);
            break;
        case dds::xrce::OBJK_REQUESTER:
            rv = create_requester(object_id, representation.requester(), result_status);
            break;
        case dds::xrce::OBJK_REPLIER:
            rv = create_replier(object_id, representation.replier(), result_status);
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
    return rv;
}

bool ProxyClient::create_participant(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::OBJK_PARTICIPANT_Representation& representation,
        dds::xrce::ResultStatus& result_status)
{
    bool rv = false;

    if (std::unique_ptr<Participant> participant = Participant::create(object_id, shared_from_this(), representation))
    {
        if (objects_.emplace(object_id, std::move(participant)).second)
        {
            rv = true;
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("participant created"),
                UXR_CREATE_PARTICIPANT_PATTERN,
                conversion::clientkey_to_raw(representation_.client_key()),
                conversion::objectid_to_raw(object_id));
        }
        else
        {
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_RED("resources error"),
                UXR_CREATE_PARTICIPANT_PATTERN,
                conversion::clientkey_to_raw(representation_.client_key()),
                conversion::objectid_to_raw(object_id));
            result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
        }
    }
    else
    {
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_RED("DDS error"),
            UXR_CREATE_PARTICIPANT_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
        result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
    }

    return rv;
}

bool ProxyClient::create_topic(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::OBJK_TOPIC_Representation& representation,
        dds::xrce::ResultStatus& result_status)
{
    bool rv = false;
    dds::xrce::ObjectId participant_id;
    participant_id[0] = representation.participant_id()[0];
    participant_id[1] = (representation.participant_id()[1] & 0xF0) | dds::xrce::OBJK_PARTICIPANT;

    auto it = objects_.find(participant_id);
    if (it != objects_.end())
    {
        if (std::unique_ptr<Topic> topic = Topic::create(object_id, conversion::objectid_to_raw(participant_id), shared_from_this(), representation))
        {
            if (objects_.emplace(object_id, std::move(topic)).second)
            {
                rv = true;
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("topic created"),
                    UXR_CREATE_TOPIC_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id),
                    conversion::objectid_to_raw(participant_id));
            }
            else
            {
                result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
                UXR_AGENT_LOG_DEBUG(
                    UXR_DECORATE_RED("resources error"),
                    UXR_CREATE_TOPIC_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id));
            }
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_RED("DDS error"),
                UXR_CREATE_TOPIC_PATTERN,
                conversion::clientkey_to_raw(representation_.client_key()),
                conversion::objectid_to_raw(object_id));
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_RED("unknown participant"),
            UXR_CREATE_TOPIC_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
    }

    return rv;
}

bool ProxyClient::create_publisher(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::OBJK_PUBLISHER_Representation& representation,
        dds::xrce::ResultStatus& result_status)
{
    bool rv = false;
    dds::xrce::ObjectId participant_id;
    participant_id[0] = representation.participant_id()[0];
    participant_id[1] = (representation.participant_id()[1] & 0xF0) | dds::xrce::OBJK_PARTICIPANT;

    auto it = objects_.find(participant_id);
    if (it != objects_.end())
    {
        if (std::unique_ptr<Publisher> publisher = Publisher::create(object_id, conversion::objectid_to_raw(participant_id), shared_from_this(), representation))
        {
            if (objects_.emplace(object_id, std::move(publisher)).second)
            {
                rv = true;
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("publisher created"),
                    UXR_CREATE_PUBLISHER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id),
                    conversion::objectid_to_raw(participant_id));
            }
            else
            {
                result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
                UXR_AGENT_LOG_DEBUG(
                    UXR_DECORATE_RED("resources error"),
                    UXR_CREATE_PUBLISHER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id));
            }
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_RED("DDS error"),
                UXR_CREATE_PUBLISHER_PATTERN,
                conversion::clientkey_to_raw(representation_.client_key()),
                conversion::objectid_to_raw(object_id));
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_RED("unknown participant"),
            UXR_CREATE_PUBLISHER_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
    }

    return rv;
}

bool ProxyClient::create_subscriber(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::OBJK_SUBSCRIBER_Representation& representation,
        dds::xrce::ResultStatus& result_status)
{
    bool rv = false;
    dds::xrce::ObjectId participant_id;
    participant_id[0] = representation.participant_id()[0];
    participant_id[1] = (representation.participant_id()[1] & 0xF0) | dds::xrce::OBJK_PARTICIPANT;

    auto it = objects_.find(participant_id);
    if (it != objects_.end())
    {
        if (std::unique_ptr<Subscriber> subscriber = Subscriber::create(object_id, conversion::objectid_to_raw(participant_id), shared_from_this(), representation))
        {
            if (objects_.emplace(object_id, std::move(subscriber)).second)
            {
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("subscriber created"),
                    UXR_CREATE_SUBSCRIBER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id),
                    conversion::objectid_to_raw(participant_id));
                rv = true;
            }
            else
            {
                result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
                UXR_AGENT_LOG_DEBUG(
                    UXR_DECORATE_RED("resources error"),
                    UXR_CREATE_SUBSCRIBER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id));
            }
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_RED("DDS error"),
                UXR_CREATE_SUBSCRIBER_PATTERN,
                conversion::clientkey_to_raw(representation_.client_key()),
                conversion::objectid_to_raw(object_id));
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_RED("unknown participant"),
            UXR_CREATE_SUBSCRIBER_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
    }

    return rv;
}

bool ProxyClient::create_datawriter(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::DATAWRITER_Representation& representation,
        dds::xrce::ResultStatus& result_status)
{
    bool rv = false;
    dds::xrce::ObjectId publisher_id;
    publisher_id[0] = representation.publisher_id()[0];
    publisher_id[1] = (representation.publisher_id()[1] & 0xF0) | dds::xrce::OBJK_PUBLISHER;

    auto it = objects_.find(publisher_id);
    if (it != objects_.end())
    {
        if (std::unique_ptr<DataWriter> datawriter = DataWriter::create(object_id, conversion::objectid_to_raw(publisher_id), shared_from_this(), representation))
        {
            if (objects_.emplace(object_id, std::move(datawriter)).second)
            {
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("datawriter created"),
                    UXR_CREATE_DATAWRITER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id),
                    conversion::objectid_to_raw(publisher_id));
                rv = true;
            }
            else
            {
                result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
                UXR_AGENT_LOG_DEBUG(
                    UXR_DECORATE_RED("resources error"),
                    UXR_CREATE_DATAWRITER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id));
            }
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_RED("DDS error"),
                UXR_CREATE_DATAWRITER_PATTERN,
                conversion::clientkey_to_raw(representation_.client_key()),
                conversion::objectid_to_raw(object_id));
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_RED("unknown participant"),
            UXR_CREATE_DATAWRITER_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
    }

    return rv;
}

bool ProxyClient::create_datareader(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::DATAREADER_Representation& representation,
        dds::xrce::ResultStatus& result_status)
{
    bool rv = false;
    dds::xrce::ObjectId subscriber_id;
    subscriber_id[0] = representation.subscriber_id()[0];
    subscriber_id[1] = (representation.subscriber_id()[1] & 0xF0) | dds::xrce::OBJK_SUBSCRIBER;

    auto it = objects_.find(subscriber_id);
    if (it != objects_.end())
    {
        if (std::unique_ptr<DataReader> datareader = DataReader::create(object_id, conversion::objectid_to_raw(subscriber_id), shared_from_this(), representation))
        {
            if (objects_.emplace(object_id, std::move(datareader)).second)
            {
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("datareader created"),
                    UXR_CREATE_DATAREADER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id),
                    conversion::objectid_to_raw(subscriber_id));
                rv = true;
            }
            else
            {
                result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
                UXR_AGENT_LOG_DEBUG(
                    UXR_DECORATE_RED("resources error"),
                    UXR_CREATE_DATAREADER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id));
            }
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_RED("DDS error"),
                UXR_CREATE_DATAREADER_PATTERN,
                conversion::clientkey_to_raw(representation_.client_key()),
                conversion::objectid_to_raw(object_id));
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_RED("unknown participant"),
            UXR_CREATE_DATAREADER_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
    }

    return rv;
}

bool ProxyClient::create_requester(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::REQUESTER_Representation& representation,
        dds::xrce::ResultStatus& result_status)
{
    bool rv = false;
    dds::xrce::ObjectId participant_id;
    participant_id[0] = representation.participant_id()[0];
    participant_id[1] = (representation.participant_id()[1] & 0xF0) | dds::xrce::OBJK_PARTICIPANT;

    auto it = objects_.find(participant_id);
    if (it != objects_.end())
    {
        if (std::unique_ptr<Requester> requester = Requester::create(object_id, conversion::objectid_to_raw(participant_id), shared_from_this(), representation))
        {
            if (objects_.emplace(object_id, std::move(requester)).second)
            {
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("requester created"),
                    UXR_CREATE_REQUESTER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id),
                    conversion::objectid_to_raw(participant_id));
                rv = true;
            }
            else
            {
                result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
                UXR_AGENT_LOG_DEBUG(
                    UXR_DECORATE_RED("resources error"),
                    UXR_CREATE_REQUESTER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id));
            }
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_RED("DDS error"),
                UXR_CREATE_REQUESTER_PATTERN,
                conversion::clientkey_to_raw(representation_.client_key()),
                conversion::objectid_to_raw(object_id));
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_RED("unknown participant"),
            UXR_CREATE_SUBSCRIBER_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
    }

    return rv;
}

bool ProxyClient::create_replier(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::REPLIER_Representation& representation,
        dds::xrce::ResultStatus& result_status)
{
    bool rv = false;
    dds::xrce::ObjectId participant_id;
    participant_id[0] = representation.participant_id()[0];
    participant_id[1] = (representation.participant_id()[1] & 0xF0) | dds::xrce::OBJK_PARTICIPANT;

    auto it = objects_.find(participant_id);
    if (it != objects_.end())
    {
        if (std::unique_ptr<Replier> requester = Replier::create(object_id, conversion::objectid_to_raw(participant_id), shared_from_this(), representation))
        {
            if (objects_.emplace(object_id, std::move(requester)).second)
            {
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("replier created"),
                    UXR_CREATE_REQUESTER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id),
                    conversion::objectid_to_raw(participant_id));
                rv = true;
            }
            else
            {
                result_status.status(dds::xrce::STATUS_ERR_RESOURCES);
                UXR_AGENT_LOG_DEBUG(
                    UXR_DECORATE_RED("resources error"),
                    UXR_CREATE_REQUESTER_PATTERN,
                    conversion::clientkey_to_raw(representation_.client_key()),
                    conversion::objectid_to_raw(object_id));
            }
        }
        else
        {
            result_status.status(dds::xrce::STATUS_ERR_DDS_ERROR);
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_RED("DDS error"),
                UXR_CREATE_REQUESTER_PATTERN,
                conversion::clientkey_to_raw(representation_.client_key()),
                conversion::objectid_to_raw(object_id));
        }
    }
    else
    {
        result_status.status(dds::xrce::STATUS_ERR_UNKNOWN_REFERENCE);
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_RED("unknown participant"),
            UXR_CREATE_SUBSCRIBER_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
    }

    return rv;
}

bool ProxyClient::delete_object_unlock(
        const dds::xrce::ObjectId& object_id)
{
    bool rv = false;
    auto it = objects_.find(object_id);
    if (it != objects_.end())
    {
        objects_.erase(object_id);
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_GREEN("object deleted"),
            UXR_CREATE_OBJECT_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_RED("unknown referece"),
            UXR_CREATE_OBJECT_PATTERN,
            conversion::clientkey_to_raw(representation_.client_key()),
            conversion::objectid_to_raw(object_id));
    }
    return rv;
}

ProxyClient::State ProxyClient::get_state()
{
    std::lock_guard<std::mutex> lock(state_mtx_);
    if (State::alive == state_)
    {
        using namespace std::chrono;
        state_ = (duration_cast<milliseconds>(steady_clock::now() - timestamp_) < client_dead_time_)
            ? State::alive
            : State::dead;
    }
    return state_;
}

void ProxyClient::update_state(const ProxyClient::State state)
{
    std::lock_guard<std::mutex> lock(state_mtx_);
    state_ = state;
    if (State::alive == state_) {
        timestamp_ = std::chrono::steady_clock::now();
        hard_liveliness_check_tries_ = 0;
    }
}

} // namespace uxr
} // namespace eprosima
