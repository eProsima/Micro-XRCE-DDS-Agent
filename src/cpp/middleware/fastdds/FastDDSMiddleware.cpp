// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/middleware/fastdds/FastDDSMiddleware.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include "../../xmlobjects/xmlobjects.h"

#include <uxr/agent/middleware/utils/Callbacks.hpp>

namespace eprosima {
namespace uxr {

using namespace fastrtps::xmlparser;

FastDDSMiddleware::FastDDSMiddleware()
    : participants_()
    , topics_()
    , publishers_()
    , subscribers_()
    , datawriters_()
    , datareaders_()
    , requesters_()
    , repliers_()
    , callback_factory_(callback_factory_.getInstance())
{
}

FastDDSMiddleware::FastDDSMiddleware(bool intraprocess_enabled)
    : Middleware(intraprocess_enabled)
    , participants_()
    , topics_()
    , publishers_()
    , subscribers_()
    , datawriters_()
    , datareaders_()
    , requesters_()
    , repliers_()
    , callback_factory_(callback_factory_.getInstance())
{
    agent_domain_id_ = get_domain_id_from_env();
    if (agent_domain_id_)
    {
        UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("Micro XRCE-DDS Agent DOMAIN_ID read from env"),
                "domain_id: {}", agent_domain_id_);
    }

}

/**********************************************************************************************************************
 * Create functions.
 **********************************************************************************************************************/
bool FastDDSMiddleware::create_participant_by_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref)
{
    if(domain_id == UXR_CLIENT_DOMAIN_ID_TO_OVERRIDE_WITH_ENV){
        domain_id = agent_domain_id_;
        UXR_AGENT_LOG_WARN(
                UXR_DECORATE_YELLOW("Overriding Micro XRCE-DDS Client DOMAIN_ID"),
                "domain_id: {}", domain_id
        );
    }

    bool rv = false;
    fastrtps::ParticipantAttributes attrs;
    auto participant_domain_id = domain_id;
    if(domain_id == UXR_CLIENT_DOMAIN_ID_TO_USE_FROM_REF && XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(ref, attrs))
    {
        participant_domain_id = static_cast<int16_t>(attrs.domainId);
    }
    std::shared_ptr<FastDDSParticipant> participant(new FastDDSParticipant(participant_domain_id));
    if (participant->create_by_ref(ref))
    {
        auto emplace_res = participants_.emplace(participant_id, std::move(participant));
        rv = emplace_res.second;
        if (rv)
        {
            callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                middleware::CallbackKind::CREATE_PARTICIPANT,
                **(emplace_res.first->second));
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_participant_by_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml)
{
    if(domain_id == UXR_CLIENT_DOMAIN_ID_TO_OVERRIDE_WITH_ENV){
        domain_id = agent_domain_id_;
        UXR_AGENT_LOG_WARN(
                UXR_DECORATE_YELLOW("Overriding Micro XRCE-DDS Client DOMAIN_ID"),
                "domain_id: {}", domain_id
        );
    }

    bool rv = false;
    std::shared_ptr<FastDDSParticipant> participant(new FastDDSParticipant(domain_id));
    if (participant->create_by_xml(xml))
    {
        auto emplace_res = participants_.emplace(participant_id, std::move(participant));
        rv = emplace_res.second;
        if (rv)
        {
            callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                middleware::CallbackKind::CREATE_PARTICIPANT,
                **(emplace_res.first->second));
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_participant_by_bin(
        uint16_t participant_id,
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce)
{
    auto participant_domain_id = static_cast<int16_t>(participant_xrce.domain_id());
    if(participant_domain_id == UXR_CLIENT_DOMAIN_ID_TO_OVERRIDE_WITH_ENV){
        participant_domain_id = agent_domain_id_;
        UXR_AGENT_LOG_WARN(
                UXR_DECORATE_YELLOW("Overriding Micro XRCE-DDS Client DOMAIN_ID"),
                "domain_id: {}", participant_domain_id
        );
    }

    bool rv = false;
    std::shared_ptr<FastDDSParticipant> participant(new FastDDSParticipant(participant_domain_id));
    if (participant->create_by_bin(participant_xrce))
    {
        auto emplace_res = participants_.emplace(participant_id, std::move(participant));
        rv = emplace_res.second;
        if (rv)
        {
            callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                middleware::CallbackKind::CREATE_PARTICIPANT,
                **(emplace_res.first->second));
        }
    }
    return rv;
}

static
std::shared_ptr<FastDDSTopic> create_topic(
        std::shared_ptr<FastDDSParticipant>& participant,
        const fastrtps::TopicAttributes& attrs)
{
    std::shared_ptr<FastDDSTopic> topic = participant->find_local_topic(attrs.getTopicName().c_str());
    if (topic)
    {
        if (0 != std::strcmp(attrs.getTopicDataType().c_str(), topic->get_type()->get_type_support()->getName()))
        {
            topic.reset();
        }
    }
    else
    {
        const char * type_name = attrs.getTopicDataType().c_str();
        std::shared_ptr<FastDDSType> type = participant->find_local_type(type_name);
        if (!type)
        {
            fastdds::dds::TypeSupport type_support(new TopicPubSubType{false});
            type_support->setName(type_name);
            type_support->m_isGetKeyDefined = (attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
            type = std::make_shared<FastDDSType>(type_support, participant);
            if (!participant->register_local_type(type))
            {
                type.reset();
            }
        }

        if (type)
        {
            topic = std::make_shared<FastDDSTopic>(participant);
            topic->create_by_name_type(attrs.getTopicName().c_str(), type);
            if (!participant->register_local_topic(topic))
            {
                topic.reset();
            }
        }
    }
    return topic;
}

bool FastDDSMiddleware::create_topic_by_ref(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    fastrtps::TopicAttributes attrs;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillTopicAttributes(ref, attrs))
    {
        auto it_participant = participants_.find(participant_id);
        if (participants_.end() != it_participant)
        {
            std::shared_ptr<FastDDSTopic> topic = create_topic(it_participant->second, attrs);
            rv = topic && topics_.emplace(topic_id, std::move(topic)).second;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_topic_by_xml(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    fastrtps::TopicAttributes attrs;
    if (xmlobjects::parse_topic(xml.data(), xml.size(), attrs))
    {
        auto it_participant = participants_.find(participant_id);
        if (participants_.end() != it_participant)
        {
            std::shared_ptr<FastDDSTopic> topic = create_topic(it_participant->second, attrs);
            rv = topic && topics_.emplace(topic_id, std::move(topic)).second;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_topic_by_bin(
        uint16_t topic_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        fastrtps::TopicAttributes attrs(
            topic_xrce.topic_name().c_str(),
            topic_xrce.type_name().c_str()
        );
        std::shared_ptr<FastDDSTopic> topic = create_topic(it_participant->second, attrs);
        rv = topic && topics_.emplace(topic_id, std::move(topic)).second;
    }
    return rv;
}

bool FastDDSMiddleware::create_publisher_by_xml(
        uint16_t publisher_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSPublisher> publisher(new FastDDSPublisher(it_participant->second));
        if (publisher->create_by_xml(xml))
        {
            publishers_.emplace(publisher_id, std::move(publisher));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_publisher_by_bin(
        uint16_t publisher_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Publisher_Binary& publisher_xrce)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSPublisher> publisher(new FastDDSPublisher(it_participant->second));
        if (publisher->create_by_bin(publisher_xrce))
        {
            publishers_.emplace(publisher_id, std::move(publisher));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_subscriber_by_xml(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSSubscriber> subscriber(new FastDDSSubscriber(it_participant->second));
        if (subscriber->create_by_xml(xml))
        {
            subscribers_.emplace(subscriber_id, std::move(subscriber));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_subscriber_by_bin(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Subscriber_Binary& subscriber_xrce)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSSubscriber> subscriber(new FastDDSSubscriber(it_participant->second));
        if (subscriber->create_by_bin(subscriber_xrce))
        {
            subscribers_.emplace(subscriber_id, std::move(subscriber));
            rv = true;
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datawriter_by_ref(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        std::shared_ptr<FastDDSDataWriter> datawriter(new FastDDSDataWriter(it_publisher->second));
        if (datawriter->create_by_ref(ref))
        {
            auto emplace_res = datawriters_.emplace(datawriter_id, std::move(datawriter));
            rv = emplace_res.second;
            if (rv)
            {
                callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                    middleware::CallbackKind::CREATE_DATAWRITER,
                    **it_publisher->second->get_participant(),
                    emplace_res.first->second->ptr());
            }
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datawriter_by_xml(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        std::shared_ptr<FastDDSDataWriter> datawriter(new FastDDSDataWriter(it_publisher->second));
        if (datawriter->create_by_xml(xml))
        {
            auto emplace_res = datawriters_.emplace(datawriter_id, std::move(datawriter));
            rv = emplace_res.second;
            if (rv)
            {
                callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                    middleware::CallbackKind::CREATE_DATAWRITER,
                    **it_publisher->second->get_participant(),
                    emplace_res.first->second->ptr());
            }
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datawriter_by_bin(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        std::shared_ptr<FastDDSDataWriter> datawriter(new FastDDSDataWriter(it_publisher->second));
        auto it_topics = topics_.find(conversion::objectid_to_raw(datawriter_xrce.topic_id()));
        if (topics_.end() != it_topics)
        {
            if (datawriter->create_by_bin(datawriter_xrce, it_topics->second))
            {
                auto emplace_res = datawriters_.emplace(datawriter_id, std::move(datawriter));
                rv = emplace_res.second;
                if (rv)
                {
                    callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                        middleware::CallbackKind::CREATE_DATAWRITER,
                        **it_publisher->second->get_participant(),
                        emplace_res.first->second->ptr());
                }
            }
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datareader_by_ref(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        std::shared_ptr<FastDDSDataReader> datareader(new FastDDSDataReader(it_subscriber->second));
        if (datareader->create_by_ref(ref))
        {
            auto emplace_res = datareaders_.emplace(datareader_id, std::move(datareader));
            rv = emplace_res.second;
            if (rv)
            {
                callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                    middleware::CallbackKind::CREATE_DATAREADER,
                    **it_subscriber->second->get_participant(),
                    emplace_res.first->second->ptr());
            }
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datareader_by_xml(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        std::shared_ptr<FastDDSDataReader> datareader(new FastDDSDataReader(it_subscriber->second));
        if (datareader->create_by_xml(xml))
        {
            auto emplace_res = datareaders_.emplace(datareader_id, std::move(datareader));
            rv = emplace_res.second;
            if (rv)
            {
                callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                    middleware::CallbackKind::CREATE_DATAREADER,
                    **it_subscriber->second->get_participant(),
                    emplace_res.first->second->ptr());
            }
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_datareader_by_bin(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        std::shared_ptr<FastDDSDataReader> datareader(new FastDDSDataReader(it_subscriber->second));
        auto it_topics = topics_.find(conversion::objectid_to_raw(datareader_xrce.topic_id()));
        if (topics_.end() != it_topics)
        {
            if (datareader->create_by_bin(datareader_xrce, it_topics->second))
            {
                auto emplace_res = datareaders_.emplace(datareader_id, std::move(datareader));
                rv = emplace_res.second;
                if (rv)
                {
                    callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                        middleware::CallbackKind::CREATE_DATAREADER,
                        **it_subscriber->second->get_participant(),
                        emplace_res.first->second->ptr());
                }
            }
        }
    }
    return rv;
}

std::shared_ptr<FastDDSRequester> FastDDSMiddleware::create_requester(
        std::shared_ptr<FastDDSParticipant>& participant,
        const fastrtps::RequesterAttributes& attrs)
{
    std::shared_ptr<FastDDSRequester> requester{};
    std::shared_ptr<FastDDSTopic> request_topic = create_topic(participant, attrs.publisher.topic);
    std::shared_ptr<FastDDSTopic> reply_topic = create_topic(participant, attrs.subscriber.topic);
    if (request_topic && reply_topic)
    {
        requester =
            std::make_shared<FastDDSRequester>(participant, request_topic, reply_topic);
        if (!requester->create_by_attributes(attrs))
        {
            requester.reset();
        }
    }
    return requester;
}

bool FastDDSMiddleware::create_requester_by_ref(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSParticipant>& participant = it_participant->second;
        fastrtps::RequesterAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillRequesterAttributes(ref, attrs))
        {
            std::shared_ptr<FastDDSRequester> requester = create_requester(participant, attrs);
            if (nullptr == requester)
            {
                return false;
            }
            auto emplace_res = requesters_.emplace(requester_id, std::move(requester));
            rv = emplace_res.second;
            if (rv)
            {
                callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                    middleware::CallbackKind::CREATE_REQUESTER,
                    participant->get_ptr(),
                    emplace_res.first->second->get_request_datawriter(),
                    emplace_res.first->second->get_reply_datareader());
            }
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_requester_by_xml(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSParticipant>& participant = it_participant->second;
        fastrtps::RequesterAttributes attrs;
        if (xmlobjects::parse_requester(xml.data(), xml.size(), attrs))
        {
            std::shared_ptr<FastDDSRequester> requester = create_requester(participant, attrs);
            if (nullptr == requester)
            {
                return false;
            }
            auto emplace_res = requesters_.emplace(requester_id, std::move(requester));
            rv = emplace_res.second;
            if (rv)
            {
                callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                    middleware::CallbackKind::CREATE_REQUESTER,
                    participant->get_ptr(),
                    emplace_res.first->second->get_request_datawriter(),
                    emplace_res.first->second->get_reply_datareader());
            }
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_requester_by_bin(
        uint16_t requester_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Requester_Binary& requester_xrce)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSParticipant>& participant = it_participant->second;
        fastrtps::RequesterAttributes attrs;

        attrs.service_name = requester_xrce.service_name();
        attrs.reply_topic_name = requester_xrce.reply_topic_name();
        attrs.request_topic_name = requester_xrce.request_topic_name();
        attrs.reply_type = requester_xrce.reply_type();
        attrs.request_type = requester_xrce.request_type();

        attrs.publisher.topic.topicName = requester_xrce.request_topic_name();
        attrs.publisher.topic.topicDataType = requester_xrce.request_type();

        attrs.subscriber.topic.topicName = requester_xrce.reply_topic_name();
        attrs.subscriber.topic.topicDataType = requester_xrce.reply_type();

        attrs.publisher.historyMemoryPolicy = fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        attrs.subscriber.historyMemoryPolicy = fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        attrs.subscriber.qos.m_reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;

        std::shared_ptr<FastDDSRequester> requester = create_requester(participant, attrs);
        if (nullptr == requester)
        {
            return false;
        }
        auto emplace_res = requesters_.emplace(requester_id, std::move(requester));
        rv = emplace_res.second;
        if (rv)
        {
            callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                middleware::CallbackKind::CREATE_REQUESTER,
                participant->get_ptr(),
                emplace_res.first->second->get_request_datawriter(),
                emplace_res.first->second->get_reply_datareader());
        }
    }
    return rv;
}

std::shared_ptr<FastDDSReplier> FastDDSMiddleware::create_replier(
        std::shared_ptr<FastDDSParticipant>& participant,
        const fastrtps::ReplierAttributes& attrs)
{
    std::shared_ptr<FastDDSReplier> replier{};
    std::shared_ptr<FastDDSTopic> request_topic = create_topic(participant, attrs.subscriber.topic);
    std::shared_ptr<FastDDSTopic> reply_topic = create_topic(participant, attrs.publisher.topic);
    if (request_topic && reply_topic)
    {
        replier =
            std::make_shared<FastDDSReplier>(participant, request_topic, reply_topic);
        if (!replier->create_by_attributes(attrs))
        {
            replier.reset();
        }
    }
    return replier;
}

bool FastDDSMiddleware::create_replier_by_ref(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSParticipant>& participant = it_participant->second;
        fastrtps::ReplierAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillReplierAttributes(ref, attrs))
        {
            std::shared_ptr<FastDDSReplier> replier = create_replier(participant, attrs);
            if (nullptr == replier)
            {
                return false;
            }
            auto emplace_res = repliers_.emplace(replier_id, std::move(replier));
            rv = emplace_res.second;
            if (rv)
            {
                callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                    middleware::CallbackKind::CREATE_REPLIER,
                    participant->get_ptr(),
                    emplace_res.first->second->get_reply_datawriter(),
                    emplace_res.first->second->get_request_datareader());
            }
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_replier_by_xml(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSParticipant>& participant = it_participant->second;
        fastrtps::ReplierAttributes attrs;
        if (xmlobjects::parse_replier(xml.data(), xml.size(), attrs))
        {
            std::shared_ptr<FastDDSReplier> replier = create_replier(participant, attrs);
            if (nullptr == replier)
            {
                return false;
            }
            auto emplace_res = repliers_.emplace(replier_id, std::move(replier));
            rv = emplace_res.second;
            if (rv)
            {
                callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                    middleware::CallbackKind::CREATE_REPLIER,
                    participant->get_ptr(),
                    emplace_res.first->second->get_reply_datawriter(),
                    emplace_res.first->second->get_request_datareader());
            }
        }
    }
    return rv;
}

bool FastDDSMiddleware::create_replier_by_bin(
        uint16_t replier_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Replier_Binary& replier_xrce)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastDDSParticipant>& participant = it_participant->second;
        fastrtps::ReplierAttributes attrs;

        attrs.service_name = replier_xrce.service_name();
        attrs.reply_topic_name = replier_xrce.reply_topic_name();
        attrs.request_topic_name = replier_xrce.request_topic_name();
        attrs.reply_type = replier_xrce.reply_type();
        attrs.request_type = replier_xrce.request_type();

        attrs.subscriber.topic.topicName = replier_xrce.request_topic_name();
        attrs.subscriber.topic.topicDataType = replier_xrce.request_type();

        attrs.publisher.topic.topicName = replier_xrce.reply_topic_name();
        attrs.publisher.topic.topicDataType = replier_xrce.reply_type();

        attrs.publisher.historyMemoryPolicy = fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        attrs.subscriber.historyMemoryPolicy = fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        attrs.subscriber.qos.m_reliability.kind = fastdds::dds::RELIABLE_RELIABILITY_QOS;

        std::shared_ptr<FastDDSReplier> replier = create_replier(participant, attrs);
        if (nullptr == replier)
        {
            return false;
        }
        auto emplace_res = repliers_.emplace(replier_id, std::move(replier));
        rv = emplace_res.second;
        if (rv)
        {
            callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
                middleware::CallbackKind::CREATE_REPLIER,
                participant->get_ptr(),
                emplace_res.first->second->get_reply_datawriter(),
                emplace_res.first->second->get_request_datareader());
        }
    }
    return rv;
}

/**********************************************************************************************************************
 * Delete functions.
 **********************************************************************************************************************/
bool FastDDSMiddleware::delete_participant(
        uint16_t participant_id)
{
    auto it = participants_.find(participant_id);
    if (it == participants_.end())
    {
        return false;
    }
    else
    {
        auto participant = it->second;
        callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
            middleware::CallbackKind::DELETE_PARTICIPANT,
            participant->get_ptr());

        participants_.erase(participant_id);
        return true;
    }
}

bool FastDDSMiddleware::delete_topic(
        uint16_t topic_id)
{
    return (0 != topics_.erase(topic_id));
}

bool FastDDSMiddleware::delete_publisher(
        uint16_t publisher_id)
{
    return (0 != publishers_.erase(publisher_id));
}

bool FastDDSMiddleware::delete_subscriber(
        uint16_t subscriber_id)
{
    return (0 != subscribers_.erase(subscriber_id));
}

bool FastDDSMiddleware::delete_datawriter(
        uint16_t datawriter_id)
{
    auto it = datawriters_.find(datawriter_id);
    if (it == datawriters_.end())
    {
        return false;
    }
    else
    {
        auto datawriter = it->second;
        callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
            middleware::CallbackKind::DELETE_DATAWRITER,
            datawriter->participant(),
            datawriter->ptr());

        datawriters_.erase(datawriter_id);
        return true;
    }
}

bool FastDDSMiddleware::delete_datareader(
        uint16_t datareader_id)
{
    auto it = datareaders_.find(datareader_id);
    if (it == datareaders_.end())
    {
        return false;
    }
    else
    {
        auto datareader = it->second;
        callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
            middleware::CallbackKind::DELETE_DATAREADER,
            datareader->participant(),
            datareader->ptr());

        datareaders_.erase(datareader_id);
        return true;
    }
}

bool FastDDSMiddleware::delete_requester(
        uint16_t requester_id)
{
    auto it = requesters_.find(requester_id);
    if (it == requesters_.end())
    {
        return false;
    }
    else
    {
        auto requester = it->second;
        callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
            middleware::CallbackKind::DELETE_REQUESTER,
            requester->get_participant(),
            requester->get_request_datawriter(),
            requester->get_reply_datareader());

        requesters_.erase(requester_id);
        return true;
    }
}

bool FastDDSMiddleware::delete_replier(
        uint16_t replier_id)
{
    auto it = repliers_.find(replier_id);
    if (it == repliers_.end())
    {
        return false;
    }
    else
    {
        auto replier = it->second;
        callback_factory_.execute_callbacks(Middleware::Kind::FASTDDS,
            middleware::CallbackKind::DELETE_REPLIER,
            replier->get_participant(),
            replier->get_reply_datawriter(),
            replier->get_request_datareader());

        repliers_.erase(replier_id);
        return true;
    }
}

/**********************************************************************************************************************
 * Write/Read functions.
 **********************************************************************************************************************/
bool FastDDSMiddleware::write_data(
        uint16_t datawriter_id,
        const std::vector<uint8_t>& data)
{
   bool rv = false;
   auto it = datawriters_.find(datawriter_id);
   if (datawriters_.end() != it)
   {
       rv = it->second->write(data);
   }
   return rv;
}

bool FastDDSMiddleware::write_request(
        uint16_t requester_id,
        uint32_t sequence_number,
        const std::vector<uint8_t>& data)
{
   bool rv = false;
   auto it = requesters_.find(requester_id);
   if (requesters_.end() != it)
   {
       rv = it->second->write(sequence_number, data);
   }
   return rv;
}

bool FastDDSMiddleware::write_reply(
        uint16_t replier_id,
        const std::vector<uint8_t>& data)
{
   bool rv = false;
   auto it = repliers_.find(replier_id);
   if (repliers_.end() != it)
   {
       rv = it->second->write(data);
   }
   return rv;
}

bool FastDDSMiddleware::read_data(
        uint16_t datareader_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
   bool rv = false;
   auto it = datareaders_.find(datareader_id);
   if (datareaders_.end() != it)
   {
       fastdds::dds::SampleInfo sample_info;
       rv = it->second->read(data, timeout, sample_info);

       if (intraprocess_enabled_)
       {
            for (auto dw = datawriters_.begin(); dw != datawriters_.end(); dw++)
            {
                if (dw->second->guid() == sample_info.sample_identity.writer_guid())
                {
                    rv = false;
                    break;
                }
            }
       }
   }
   return rv;
}

bool FastDDSMiddleware::read_request(
        uint16_t replier_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
   bool rv = false;
   auto it = repliers_.find(replier_id);
   if (repliers_.end() != it)
   {
        fastdds::dds::SampleInfo sample_info;
        rv = it->second->read(data, timeout, sample_info);

        if (intraprocess_enabled_)
        {
            for (auto rq = requesters_.begin(); rq != requesters_.end(); rq++)
            {
                if (rq->second->guid_datawriter() == sample_info.sample_identity.writer_guid())
                {
                    rv = false;
                    break;
                }
            }
        }

   }
   return rv;
}

bool FastDDSMiddleware::read_reply(
        uint16_t requester_id,
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
   bool rv = false;
   auto it = requesters_.find(requester_id);
   if (requesters_.end() != it)
   {
       fastdds::dds::SampleInfo sample_info;
       rv = it->second->read(sequence_number, data, timeout, sample_info);

       if (intraprocess_enabled_)
        {
            for (auto rp = repliers_.begin(); rp != repliers_.end(); rp++)
            {
                if (rp->second->guid_datawriter() == sample_info.sample_identity.writer_guid())
                {
                    rv = false;
                    break;
                }
            }
        }
   }
   return rv;
}

/**********************************************************************************************************************
 * Matched functions.
 **********************************************************************************************************************/
bool FastDDSMiddleware::matched_participant_from_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        fastrtps::ParticipantAttributes attrs;
        auto participant_domain_id = domain_id;
        if(domain_id == UXR_CLIENT_DOMAIN_ID_TO_USE_FROM_REF && XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(ref, attrs))
        {
            participant_domain_id = static_cast<int16_t>(attrs.domainId);
        }
        rv = (participant_domain_id== it->second->domain_id()) && (it->second->match_from_ref(ref));
    }
    return rv;
}

bool FastDDSMiddleware::matched_participant_from_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        rv = (domain_id == it->second->domain_id()) && (it->second->match_from_xml(xml));
    }
    return rv;
}

bool FastDDSMiddleware::matched_participant_from_bin(
        uint16_t participant_id,
        int16_t domain_id,
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce ) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        rv = (domain_id == it->second->domain_id()) && (it->second->match_from_bin(participant_xrce));
    }
    return rv;
}

bool FastDDSMiddleware::matched_topic_from_ref(
        uint16_t topic_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        fastrtps::TopicAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillTopicAttributes(ref, attrs))
        {
            rv = it->second->match(attrs);
        }
    }
    return rv;
}

bool FastDDSMiddleware::matched_topic_from_xml(
        uint16_t topic_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        fastrtps::TopicAttributes attrs;
        if (xmlobjects::parse_topic(xml.data(), xml.size(), attrs))
        {
            rv = it->second->match(attrs);
        }
    }
    return rv;
}

bool FastDDSMiddleware::matched_topic_from_bin(
        uint16_t topic_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        rv = it->second->match_from_bin(topic_xrce);
    }
    return rv;
}

bool FastDDSMiddleware::matched_datawriter_from_ref(
        uint16_t datawriter_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        fastrtps::PublisherAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(ref, attrs))
        {
            rv = it->second->match(attrs);
        }
    }
    return rv;
}

bool FastDDSMiddleware::matched_datawriter_from_xml(
        uint16_t datawriter_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        fastrtps::PublisherAttributes attrs;
        if (xmlobjects::parse_publisher(xml.data(), xml.size(), attrs))
        {
            rv = it->second->match(attrs);
        }
    }
    return rv;
}

bool FastDDSMiddleware::matched_datawriter_from_bin(
        uint16_t datawriter_id,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) const
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        rv = it->second->match_from_bin(datawriter_xrce);
    }
    return rv;
}

bool FastDDSMiddleware::matched_datareader_from_ref(
        uint16_t datareader_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        rv = it->second->match_from_ref(ref);
    }
    return rv;
}

bool FastDDSMiddleware::matched_datareader_from_xml(
        uint16_t datareader_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        rv = it->second->match_from_xml(xml);
    }
    return rv;
}

bool FastDDSMiddleware::matched_datareader_from_bin(
        uint16_t datareader_id,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce) const
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        rv = it->second->match_from_bin(datareader_xrce);
    }
    return rv;
}

bool FastDDSMiddleware::matched_requester_from_ref(
        uint16_t requester_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = requesters_.find(requester_id);
    if (requesters_.end() != it)
    {
        rv = it->second->match_from_ref(ref);
    }
    return rv;
}

bool FastDDSMiddleware::matched_requester_from_xml(
        uint16_t requester_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = requesters_.find(requester_id);
    if (requesters_.end() != it)
    {
        rv = it->second->match_from_xml(xml);
    }
    return rv;
}

bool FastDDSMiddleware::matched_requester_from_bin(
        uint16_t requester_id,
        const dds::xrce::OBJK_Requester_Binary& requester_xrce) const
{
    bool rv = false;
    auto it = requesters_.find(requester_id);
    if (requesters_.end() != it)
    {
        rv = it->second->match_from_bin(requester_xrce);
    }
    return rv;
}

bool FastDDSMiddleware::matched_replier_from_ref(
        uint16_t requester_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = repliers_.find(requester_id);
    if (repliers_.end() != it)
    {
        rv = it->second->match_from_ref(ref);
    }
    return rv;
}

bool FastDDSMiddleware::matched_replier_from_xml(
        uint16_t requester_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = repliers_.find(requester_id);
    if (repliers_.end() != it)
    {
        rv = it->second->match_from_xml(xml);
    }
    return rv;
}

bool FastDDSMiddleware::matched_replier_from_bin(
        uint16_t requester_id,
        const dds::xrce::OBJK_Replier_Binary& replier_xrce) const
{
    bool rv = false;
    auto it = repliers_.find(requester_id);
    if (repliers_.end() != it)
    {
        rv = it->second->match_from_bin(replier_xrce);
    }
    return rv;
}

int16_t FastDDSMiddleware::get_domain_id_from_env(){
    int16_t agent_domain_id = 0;
    const char * agent_domain_id_env = std::getenv( "XRCE_DOMAIN_ID_OVERRIDE" );
    if (nullptr != agent_domain_id_env)
    {
        agent_domain_id = static_cast<int16_t>(std::atoi(agent_domain_id_env));
    }
    return agent_domain_id;
}

} // namespace uxr
} // namespace eprosima