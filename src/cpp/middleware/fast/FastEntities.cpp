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

#include <uxr/agent/middleware/fast/FastEntities.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../../xmlobjects/xmlobjects.h"

namespace eprosima {
namespace uxr {

/**********************************************************************************************************************
 * FastParticipant
 **********************************************************************************************************************/
FastParticipant::~FastParticipant()
{
    fastrtps::Domain::removeParticipant(ptr_);
}

bool FastParticipant::create_by_ref(const std::string& ref)
{
    ptr_ = fastrtps::Domain::createParticipant(ref, this);
    return (nullptr != ptr_);
}

bool FastParticipant::create_by_attributes(const ParticipantAttributes& attrs)
{
    ptr_ = fastrtps::Domain::createParticipant(attrs, this);
    return (nullptr != ptr_);
}

bool FastParticipant::match_from_ref(const std::string& ref) const
{
    bool rv = false;
    fastrtps::ParticipantAttributes new_attributes;
    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillParticipantAttributes(ref, new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

bool FastParticipant::match_from_xml(const std::string& xml) const
{
    bool rv = false;
    fastrtps::ParticipantAttributes new_attributes;
    if (xmlobjects::parse_participant(xml.data(), xml.size(), new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

void FastParticipant::onParticipantDiscovery(
        fastrtps::Participant*,
        fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        UXR_AGENT_LOG_TRACE(
            UXR_DECORATE_WHITE("matched"),
            "entity_id: {}, guid_prefix: {}",
            info.info.m_guid.entityId,
            info.info.m_guid.guidPrefix);
    }
    else
    {
        UXR_AGENT_LOG_TRACE(
            UXR_DECORATE_WHITE("unmatched"),
            "entity_id: {}, guid_prefix: {}",
            info.info.m_guid.entityId,
            info.info.m_guid.guidPrefix);
    }
}

bool FastParticipant::register_topic(
        FastTopic* const topic,
        uint16_t topic_id)
{
    // TODO (#5057): allow more than one topic.
    bool rv = false;
    auto it = topics_register_.find(topic->getName());
    if (topics_register_.end() == it)
    {
        fastrtps::Domain::registerType(ptr_, topic);
        topics_register_[topic->getName()] = topic_id;
        rv = true;
    }
    return rv;
}

bool FastParticipant::unregister_topic(const FastTopic* const topic)
{
    bool rv = false;
    if (0 != topics_register_.erase(topic->getName()))
    {
        fastrtps::Domain::unregisterType(ptr_, topic->getName());
        rv = true;
    }
    return rv;
}

bool FastParticipant::find_topic(
        const std::string& topic_name,
        uint16_t& topic_id)
{
    bool rv = false;
    auto it = topics_register_.find(topic_name);
    if (topics_register_.end() != it)
    {
        topic_id = it->second;
        rv = true;
    }
    return rv;
}

/**********************************************************************************************************************
 * FastTopic
 **********************************************************************************************************************/
FastTopic::FastTopic(const std::shared_ptr<FastParticipant>& participant)
    : TopicPubSubType{false}
    , participant_(participant)
{}

FastTopic::~FastTopic()
{
    participant_->unregister_topic(this);
}

bool FastTopic::create_by_attributes(
        const fastrtps::TopicAttributes& attrs,
        uint16_t topic_id)
{
    bool rv = false;
    setName(attrs.getTopicDataType().c_str());
    m_isGetKeyDefined = (attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
    if (participant_->register_topic(this, topic_id))
    {
        rv = true;
    }
    return rv;
}

bool FastTopic::match_from_ref(const std::string& ref) const
{
    bool rv = false;
    fastrtps::TopicAttributes new_attributes;
    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillTopicAttributes(ref, new_attributes))
    {
        rv = (0 == std::strcmp(getName(), new_attributes.getTopicDataType().c_str())) &&
             (m_isGetKeyDefined == (new_attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY));
    }
    return rv;
}

bool FastTopic::match_from_xml(const std::string& xml) const
{
    bool rv = false;
    fastrtps::TopicAttributes new_attributes;
    if (xmlobjects::parse_topic(xml.data(), xml.size(), new_attributes))
    {
        rv = (0 == std::strcmp(getName(), new_attributes.getTopicDataType().c_str())) &&
             (m_isGetKeyDefined == (new_attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY));
    }
    return rv;
}

/**********************************************************************************************************************
 * FastDataWriter
 **********************************************************************************************************************/
FastDataWriter::FastDataWriter(const std::shared_ptr<FastParticipant>& participant)
    : participant_(participant)
    , ptr_(nullptr)
{}

FastDataWriter::~FastDataWriter()
{
    fastrtps::Domain::removePublisher(ptr_);
}

bool FastDataWriter::create_by_ref(
        const std::string& ref,
        uint16_t& topic_id)
{
    bool rv = false;
    ptr_ = fastrtps::Domain::createPublisher(participant_->get_ptr(), ref, this);
    if (nullptr != ptr_)
    {
        rv = participant_->find_topic(ptr_->getAttributes().topic.getTopicDataType().c_str(), topic_id);
    }
    return rv;
}

bool FastDataWriter::create_by_attributes(
        const PublisherAttributes& attrs,
        uint16_t& topic_id)
{
    bool rv = false;
    ptr_ = fastrtps::Domain::createPublisher(participant_->get_ptr(), attrs, this);
    if (nullptr != ptr_)
    {
        rv = participant_->find_topic(ptr_->getAttributes().topic.getTopicDataType().c_str(), topic_id);
    }
    return rv;
}

bool FastDataWriter::match_from_ref(const std::string& ref) const
{
    bool rv = false;
    fastrtps::PublisherAttributes new_attributes;
    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillPublisherAttributes(ref, new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

bool FastDataWriter::match_from_xml(const std::string& xml) const
{
    bool rv = false;
    fastrtps::PublisherAttributes new_attributes;
    if (xmlobjects::parse_publisher(xml.data(), xml.size(), new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

bool FastDataWriter::write(const std::vector<uint8_t>& data)
{
    return ptr_->write(&const_cast<std::vector<uint8_t>&>(data));
}

void FastDataWriter::onPublicationMatched(
        fastrtps::Publisher*,
        fastrtps::rtps::MatchingInfo& info)
{
    if (info.status == fastrtps::rtps::MATCHED_MATCHING)
    {
        UXR_AGENT_LOG_TRACE(
            UXR_DECORATE_WHITE("matched"),
            "entity_id: {}, guid_prefix: {}",
            info.remoteEndpointGuid.entityId,
            info.remoteEndpointGuid.guidPrefix);
    }
    else
    {
        UXR_AGENT_LOG_TRACE(
            UXR_DECORATE_WHITE("unmatched"),
            "entity_id: {}, guid_prefix: {}",
            info.remoteEndpointGuid.entityId,
            info.remoteEndpointGuid.guidPrefix);
    }
}

/**********************************************************************************************************************
 * FastDataReader
 **********************************************************************************************************************/
FastDataReader::FastDataReader(const std::shared_ptr<FastParticipant>& participant)
    : participant_{participant}
    , ptr_{nullptr}
    , unread_count_{0}
{}

FastDataReader::~FastDataReader()
{
    fastrtps::Domain::removeSubscriber(ptr_);
}

bool FastDataReader::create_by_ref(
        const std::string& ref,
        uint16_t& topic_id)
{
    bool rv = false;
    ptr_ = fastrtps::Domain::createSubscriber(participant_->get_ptr(), ref, this);
    if (nullptr != ptr_)
    {
        rv = participant_->find_topic(ptr_->getAttributes().topic.getTopicDataType().c_str(), topic_id);
    }
    return rv;
}

bool FastDataReader::create_by_attributes(
        const SubscriberAttributes& attrs,
        uint16_t& topic_id)
{
    bool rv = false;
    ptr_ = fastrtps::Domain::createSubscriber(participant_->get_ptr(), attrs, this);
    if (nullptr != ptr_)
    {
        rv = participant_->find_topic(ptr_->getAttributes().topic.getTopicDataType().c_str(), topic_id);
    }
    return rv;
}

bool FastDataReader::match_from_ref(const std::string& ref) const
{
    bool rv = false;
    fastrtps::SubscriberAttributes new_attributes;
    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillSubscriberAttributes(ref, new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

bool FastDataReader::match_from_xml(const std::string& xml) const
{
    bool rv = false;
    fastrtps::SubscriberAttributes new_attributes;
    if (xmlobjects::parse_subscriber(xml.data(), xml.size(), new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

bool FastDataReader::read(
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    auto now = std::chrono::steady_clock::now();
    bool rv = false;
    if (unread_count_ != 0)
    {
        fastrtps::SampleInfo_t info;
        rv = ptr_->takeNextData(&data, &info);
        unread_count_ = ptr_->getUnreadCount();
    }
    else
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (cv_.wait_until(lock, now + timeout, [&](){ return unread_count_ != 0; }))
        {
            lock.unlock();
            fastrtps::SampleInfo_t info;
            rv = ptr_->takeNextData(&data, &info);
            unread_count_ = ptr_->getUnreadCount();
        }
    }
    return rv;
}

void FastDataReader::onSubscriptionMatched(
        fastrtps::Subscriber*,
        fastrtps::rtps::MatchingInfo& info)
{
    if (info.status == fastrtps::rtps::MATCHED_MATCHING)
    {
        UXR_AGENT_LOG_TRACE(
            UXR_DECORATE_WHITE("matched"),
            "entity_id: {}, guid_prefix: {}",
            info.remoteEndpointGuid.entityId,
            info.remoteEndpointGuid.guidPrefix);
    }
    else
    {
        UXR_AGENT_LOG_TRACE(
            UXR_DECORATE_WHITE("unmatched"),
            "entity_id: {}, guid_prefix: {}",
            info.remoteEndpointGuid.entityId,
            info.remoteEndpointGuid.guidPrefix);
    }
}

void FastDataReader::onNewDataMessage(fastrtps::Subscriber *)
{
    unread_count_ = ptr_->getUnreadCount();
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.notify_one();
}

/**********************************************************************************************************************
 * FastRequester
 **********************************************************************************************************************/
FastRequester::FastRequester(
        const std::shared_ptr<FastParticipant>& participant)
    : participant_{participant}
    , request_topic_{false}
    , reply_topic_{false}
    , publisher_ptr_{nullptr}
    , subscriber_ptr_{nullptr}
{}

FastRequester::~FastRequester()
{
    fastrtps::Domain::removePublisher(publisher_ptr_);
    fastrtps::Domain::removeSubscriber(subscriber_ptr_);
}

bool FastRequester::create_by_ref(
        const std::string& ref)
{
    bool rv = false;
    fastrtps::RequesterAttributes requester_attrs;

    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillRequesterAttributes(ref, requester_attrs))
    {
        rv = create_by_attributes(requester_attrs);
    }

    return rv;
}

bool FastRequester::create_by_attributes(
        const fastrtps::RequesterAttributes& attrs)
{
    bool rv = false;

    const fastrtps::TopicAttributes& request_topic_attrs = attrs.publisher.topic;
    request_topic_.setName(request_topic_attrs.getTopicDataType().c_str());
    request_topic_.m_isGetKeyDefined =
        (request_topic_attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);

    const fastrtps::TopicAttributes& reply_topic_attrs = attrs.subscriber.topic;
    reply_topic_.setName(reply_topic_attrs.getTopicDataType().c_str());
    reply_topic_.m_isGetKeyDefined =
        (reply_topic_attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);

    fastrtps::Participant* participant_ptr = participant_.get()->get_ptr();
    if (fastrtps::Domain::registerType(participant_ptr, &request_topic_) &&
        fastrtps::Domain::registerType(participant_ptr, &reply_topic_))
    {
        publisher_ptr_ = fastrtps::Domain::createPublisher(participant_ptr, attrs.publisher, this);
        subscriber_ptr_ = fastrtps::Domain::createSubscriber(participant_ptr, attrs.subscriber, this);
        rv = (nullptr != publisher_ptr_) && (nullptr != subscriber_ptr_);
    }

    return rv;
}

void FastRequester::onPublicationMatched(
        fastrtps::Publisher*,
        fastrtps::rtps::MatchingInfo&)
{
    // TODO
}

void FastRequester::onSubscriptionMatched(
        fastrtps::Subscriber*,
        fastrtps::rtps::MatchingInfo&)
{
    // TODO
}

void FastRequester::onNewDataMessage(
        fastrtps::Subscriber*)
{
    // TODO
}

} // namespace uxr
} // namespace eprosima
