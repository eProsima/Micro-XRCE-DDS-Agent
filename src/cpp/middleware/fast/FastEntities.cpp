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
            "GUIDPrefix: {}, EntityId: {}, Status: MATCHED",
            info.info.m_guid.guidPrefix,
            info.info.m_guid.entityId);
    }
    else
    {
        UXR_AGENT_LOG_TRACE(
            "GUIDPrefix: {}, EntityId: {}, Status: UNMATCHED",
            info.info.m_guid.guidPrefix,
            info.info.m_guid.entityId);
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
    setName(attrs.getTopicDataType().data());
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
        rv = (0 == std::strcmp(getName(), new_attributes.getTopicDataType().data())) &&
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
        rv = (0 == std::strcmp(getName(), new_attributes.getTopicDataType().data())) &&
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
        rv = participant_->find_topic(ptr_->getAttributes().topic.getTopicDataType(), topic_id);
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
        rv = participant_->find_topic(ptr_->getAttributes().topic.getTopicDataType(), topic_id);
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
            "GUIDPrefix: {}, EntityId: {}, Status: MATCHED",
            info.remoteEndpointGuid.guidPrefix,
            info.remoteEndpointGuid.entityId);
    }
    else
    {
        UXR_AGENT_LOG_TRACE(
            "GUIDPrefix: {}, EntityId: {}, Status: MATCHED",
            info.remoteEndpointGuid.guidPrefix,
            info.remoteEndpointGuid.entityId);
    }
}

/**********************************************************************************************************************
 * FastDataReader
 **********************************************************************************************************************/
FastDataReader::FastDataReader(const std::shared_ptr<FastParticipant>& participant)
    : participant_(participant)
    , ptr_(nullptr)
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
        rv = participant_->find_topic(ptr_->getAttributes().topic.getTopicDataType(), topic_id);
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
        rv = participant_->find_topic(ptr_->getAttributes().topic.getTopicDataType(), topic_id);
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
    std::unique_lock<std::mutex> lock(mtx_);
    if (ptr_->getUnreadCount() != 0)
    {
        lock.unlock();
        fastrtps::SampleInfo_t info;
        rv = ptr_->takeNextData(&data, &info);
    }
    else
    {
        if (cv_.wait_until(lock, now + timeout, [&](){ return  ptr_->getUnreadCount() != 0; }))
        {
            lock.unlock();
            fastrtps::SampleInfo_t info;
            rv = ptr_->takeNextData(&data, &info);
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
            "GUIDPrefix: {}, EntityId: {}, Status: MATCHED",
            info.remoteEndpointGuid.guidPrefix,
            info.remoteEndpointGuid.entityId);
    }
    else
    {
        UXR_AGENT_LOG_TRACE(
            "GUIDPrefix: {}, EntityId: {}, Status: UNMATCHED",
            info.remoteEndpointGuid.guidPrefix,
            info.remoteEndpointGuid.entityId);
    }
}

void FastDataReader::onNewDataMessage(fastrtps::Subscriber *)
{
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.notify_one();
}

} // namespace uxr
} // namespace eprosima
