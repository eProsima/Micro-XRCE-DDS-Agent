// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/middleware/fast/FastMiddleware.hpp>

#include <fastrtps/Domain.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../../xmlobjects/xmlobjects.h"

namespace eprosima {
namespace uxr {

using namespace fastrtps::xmlparser;

/**********************************************************************************************************************
 * Create functions.
 **********************************************************************************************************************/
bool FastMiddleware::create_participant_by_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref)
{
    bool rv = false;
    fastrtps::ParticipantAttributes attrs;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(ref, attrs))
    {
        attrs.domainId = uint32_t(domain_id);
        fastrtps::Participant* impl = fastrtps::Domain::createParticipant(attrs, &listener_);
        if (nullptr != impl)
        {
            std::shared_ptr<FastParticipant> participant(new FastParticipant(impl));
            rv = participants_.emplace(participant_id, std::move(participant)).second;
        }
    }
    return rv;
}

bool FastMiddleware::create_participant_by_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml)
{
    (void) domain_id;
    bool rv = false;
    fastrtps::ParticipantAttributes attrs;
    if (xmlobjects::parse_participant(xml.data(), xml.size(), attrs))
    {
        attrs.domainId = uint32_t(domain_id);
        fastrtps::Participant* impl = fastrtps::Domain::createParticipant(attrs, &listener_);
        if (nullptr != impl)
        {
            std::shared_ptr<FastParticipant> participant(new FastParticipant(impl));
            rv = participants_.emplace(participant_id, std::move(participant)).second;
        }
    }
    return rv;
}

static
std::shared_ptr<FastTopic> create_topic(
        std::shared_ptr<FastParticipant>& participant,
        const fastrtps::TopicAttributes& attrs)
{
    std::shared_ptr<FastTopic> topic = participant->find_topic(attrs.getTopicName().c_str());
    if (topic)
    {
        if (0 != std::strcmp(attrs.getTopicDataType().c_str(), topic->get_type()->getName()))
        {
            topic.reset();
        }
    }
    else
    {
        const char * type_name = attrs.getTopicDataType().c_str();
        std::shared_ptr<FastType> type = participant->find_type(type_name);
        if (!type)
        {
            type = std::make_shared<FastType>(participant);
            type->setName(type_name);
            type->m_isGetKeyDefined = (attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
            if (!participant->register_type(type))
            {
                type.reset();
            }
        }

        if (type)
        {
            topic = std::make_shared<FastTopic>(attrs.getTopicName().c_str(), type, participant);
            if (!participant->register_topic(topic))
            {
                topic.reset();
            }
        }
    }
    return topic;
}

bool FastMiddleware::create_topic_by_ref(
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
            std::shared_ptr<FastTopic> topic = create_topic(it_participant->second, attrs);
            rv = topic && topics_.emplace(topic_id, std::move(topic)).second;
        }
    }
    return rv;
}

bool FastMiddleware::create_topic_by_xml(
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
            std::shared_ptr<FastTopic> topic = create_topic(it_participant->second, attrs);
            rv = topic && topics_.emplace(topic_id, std::move(topic)).second;
        }
    }
    return rv;
}

bool FastMiddleware::create_publisher_by_xml(
        uint16_t publisher_id,
        uint16_t participant_id,
        const std::string&)
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (it != participants_.end())
    {
        std::shared_ptr<FastPublisher> publisher = std::make_shared<FastPublisher>(it->second);
        rv = publishers_.emplace(publisher_id, std::move(publisher)).second;
    }
    return rv;
}

bool FastMiddleware::create_subscriber_by_xml(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const std::string&)
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (it != participants_.end())
    {
        std::shared_ptr<FastSubscriber> subscriber = std::make_shared<FastSubscriber>(it->second);
        rv = subscribers_.emplace(subscriber_id, std::move(subscriber)).second;
    }
    return rv;
}

inline
std::shared_ptr<FastDataWriter> create_datawriter(
        const fastrtps::PublisherAttributes& attrs,
        fastrtps::PublisherListener* listener,
        const std::shared_ptr<FastPublisher>& publisher)
{
    std::shared_ptr<FastDataWriter> datawriter;
    const std::shared_ptr<FastParticipant>& participant = publisher->get_participant();
    std::shared_ptr<FastTopic> topic =
        participant->find_topic(attrs.topic.getTopicName().to_string());
    if (topic)
    {
        fastrtps::Publisher* impl =
            fastrtps::Domain::createPublisher(participant->get_ptr(), attrs, listener);
        if (nullptr != impl)
        {
            datawriter = std::make_shared<FastDataWriter>(impl, topic, publisher);
        }
    }
    return datawriter;
}

bool FastMiddleware::create_datawriter_by_ref(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        fastrtps::PublisherAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(ref, attrs))
        {
            std::shared_ptr<FastDataWriter> datawriter =
                create_datawriter(attrs, &listener_, it_publisher->second);
            rv = datawriter && datawriters_.emplace(datawriter_id, std::move(datawriter)).second;
        }
    }
    return rv;
}

bool FastMiddleware::create_datawriter_by_xml(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        fastrtps::PublisherAttributes attrs;
        if (xmlobjects::parse_publisher(xml.data(), xml.size(), attrs))
        {
            std::shared_ptr<FastDataWriter> datawriter =
                create_datawriter(attrs, &listener_, it_publisher->second);
            rv = datawriter && datawriters_.emplace(datawriter_id, std::move(datawriter)).second;
        }
    }
    return rv;
}

inline
std::shared_ptr<FastDataReader> create_datareader(
        const fastrtps::SubscriberAttributes& attrs,
        fastrtps::SubscriberListener* listener,
        const std::shared_ptr<FastSubscriber>& subscriber)
{
    std::shared_ptr<FastDataReader> datareader;
    const std::shared_ptr<FastParticipant>& participant = subscriber->get_participant();
    std::shared_ptr<FastTopic> topic =
        participant->find_topic(attrs.topic.getTopicName().to_string());
    if (topic)
    {
        fastrtps::Subscriber* impl =
            fastrtps::Domain::createSubscriber(participant->get_ptr(), attrs, listener);
        if (nullptr != impl)
        {
            datareader = std::make_shared<FastDataReader>(impl, topic, subscriber);
        }
    }
    return datareader;
}

bool FastMiddleware::create_datareader_by_ref(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        fastrtps::SubscriberAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(ref, attrs))
        {
            std::shared_ptr<FastDataReader> datareader =
                create_datareader(attrs, &listener_, it_subscriber->second);
            rv = datareader && datareaders_.emplace(datareader_id, std::move(datareader)).second;
        }
    }
    return rv;
}

bool FastMiddleware::create_datareader_by_xml(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        fastrtps::SubscriberAttributes attrs;
        if (xmlobjects::parse_subscriber(xml.data(), xml.size(), attrs))
        {
            std::shared_ptr<FastDataReader> datareader =
                create_datareader(attrs, &listener_, it_subscriber->second);
            rv = datareader && datareaders_.emplace(datareader_id, std::move(datareader)).second;
        }
    }
    return rv;
}

static
std::shared_ptr<FastRequester> create_requester(
        std::shared_ptr<FastParticipant>& participant,
        const fastrtps::RequesterAttributes& attrs)
{
    std::shared_ptr<FastRequester> requester{};
    std::shared_ptr<FastTopic> request_topic = create_topic(participant, attrs.publisher.topic);
    std::shared_ptr<FastTopic> reply_topic = create_topic(participant, attrs.subscriber.topic);
    if (request_topic && reply_topic)
    {
        requester =
            std::make_shared<FastRequester>(participant, request_topic, reply_topic);
        if (!requester->create_by_attributes(attrs))
        {
            requester.reset();
        }
    }
    return requester;
}

bool FastMiddleware::create_requester_by_ref(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastParticipant>& participant = it_participant->second;
        fastrtps::RequesterAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillRequesterAttributes(ref, attrs))
        {
            std::shared_ptr<FastRequester> requester = create_requester(participant, attrs);
            rv = requester && requesters_.emplace(requester_id, std::move(requester)).second;
        }
    }
    return rv;
}

bool FastMiddleware::create_requester_by_xml(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastParticipant>& participant = it_participant->second;
        fastrtps::RequesterAttributes attrs;
        if (xmlobjects::parse_requester(xml.data(), xml.size(), attrs))
        {
            std::shared_ptr<FastRequester> requester = create_requester(participant, attrs);
            rv = requester && requesters_.emplace(requester_id, std::move(requester)).second;
        }
    }
    return rv;
}

static
std::shared_ptr<FastReplier> create_replier(
        std::shared_ptr<FastParticipant>& participant,
        const fastrtps::ReplierAttributes& attrs)
{
    std::shared_ptr<FastReplier> replier{};
    std::shared_ptr<FastTopic> request_topic = create_topic(participant, attrs.subscriber.topic);
    std::shared_ptr<FastTopic> reply_topic = create_topic(participant, attrs.publisher.topic);
    if (request_topic && reply_topic)
    {
        replier =
            std::make_shared<FastReplier>(participant, request_topic, reply_topic);
        if (!replier->create_by_attributes(attrs))
        {
            replier.reset();
        }
    }
    return replier;
}

bool FastMiddleware::create_replier_by_ref(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastParticipant>& participant = it_participant->second;
        fastrtps::ReplierAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillReplierAttributes(ref, attrs))
        {
            std::shared_ptr<FastReplier> replier = create_replier(participant, attrs);
            rv = replier && repliers_.emplace(replier_id, std::move(replier)).second;
        }
    }
    return rv;
}

bool FastMiddleware::create_replier_by_xml(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastParticipant>& participant = it_participant->second;
        fastrtps::ReplierAttributes attrs;
        if (xmlobjects::parse_replier(xml.data(), xml.size(), attrs))
        {
            std::shared_ptr<FastReplier> replier = create_replier(participant, attrs);
            rv = replier && repliers_.emplace(replier_id, std::move(replier)).second;
        }
    }
    return rv;
}

/**********************************************************************************************************************
 * Delete functions.
 **********************************************************************************************************************/
bool FastMiddleware::delete_participant(
        uint16_t participant_id)
{
    return (0 != participants_.erase(participant_id));
}

bool FastMiddleware::delete_topic(
        uint16_t topic_id)
{
    return (0 != topics_.erase(topic_id));
}

bool FastMiddleware::delete_publisher(
        uint16_t publisher_id)
{
    return (0 != publishers_.erase(publisher_id));
}

bool FastMiddleware::delete_subscriber(
        uint16_t subscriber_id)
{
    return (0 != subscribers_.erase(subscriber_id));
}

bool FastMiddleware::delete_datawriter(
        uint16_t datawriter_id)
{
    return (0 != datawriters_.erase(datawriter_id));
}

bool FastMiddleware::delete_datareader(
        uint16_t datareader_id)
{
    return (0 != datareaders_.erase(datareader_id));
}

bool FastMiddleware::delete_requester(
        uint16_t requester_id)
{
    return (0 != requesters_.erase(requester_id));
}

bool FastMiddleware::delete_replier(
        uint16_t replier_id)
{
    return (0 != repliers_.erase(replier_id));
}

/**********************************************************************************************************************
 * Write/Read functions.
 **********************************************************************************************************************/
bool FastMiddleware::write_data(
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

bool FastMiddleware::write_request(
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

bool FastMiddleware::write_reply(
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

bool FastMiddleware::read_data(
        uint16_t datareader_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        rv = it->second->read(data, timeout);
    }
    return rv;
}

bool FastMiddleware::read_request(
        uint16_t replier_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    auto it = repliers_.find(replier_id);
    if (repliers_.end() != it)
    {
        rv = it->second->read(data, timeout);
    }
    return rv;
}

bool FastMiddleware::read_reply(
        uint16_t requester_id,
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    auto it = requesters_.find(requester_id);
    if (requesters_.end() != it)
    {
        rv = it->second->read(sequence_number, data, timeout);
    }
    return rv;
}

/**********************************************************************************************************************
 * Matched functions.
 **********************************************************************************************************************/
bool FastMiddleware::matched_participant_from_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        fastrtps::ParticipantAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(ref, attrs))
        {
            attrs.domainId = uint32_t(domain_id);
            rv = it->second->match(attrs);
        }
    }
    return rv;
}

bool FastMiddleware::matched_participant_from_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        fastrtps::ParticipantAttributes attrs;
        if (xmlobjects::parse_participant(xml.data(), xml.size(), attrs))
        {
            attrs.domainId = uint32_t(domain_id);
            rv = it->second->match(attrs);
        }
    }
    return rv;
}

bool FastMiddleware::matched_topic_from_ref(
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

bool FastMiddleware::matched_topic_from_xml(
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

bool FastMiddleware::matched_datawriter_from_ref(
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

bool FastMiddleware::matched_datawriter_from_xml(
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

bool FastMiddleware::matched_datareader_from_ref(
        uint16_t datareader_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        fastrtps::SubscriberAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(ref, attrs))
        {
            rv = it->second->match(attrs);
        }
    }
    return rv;
}

bool FastMiddleware::matched_datareader_from_xml(
        uint16_t datareader_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        fastrtps::SubscriberAttributes attrs;
        if (xmlobjects::parse_subscriber(xml.data(), xml.size(), attrs))
        {
            rv = it->second->match(attrs);
        }
    }
    return rv;
}

bool FastMiddleware::matched_requester_from_ref(
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

bool FastMiddleware::matched_requester_from_xml(
        uint16_t requester_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = requesters_.find(requester_id);
    if (requesters_.end() != it)
    {
        rv = it->second->match_from_ref(xml);
    }
    return rv;
}

bool FastMiddleware::matched_replier_from_ref(
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

bool FastMiddleware::matched_replier_from_xml(
        uint16_t requester_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = repliers_.find(requester_id);
    if (repliers_.end() != it)
    {
        rv = it->second->match_from_ref(xml);
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
