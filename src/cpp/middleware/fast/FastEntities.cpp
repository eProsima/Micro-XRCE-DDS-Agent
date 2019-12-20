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
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>
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
        const fastrtps::TopicAttributes& attrs,
        std::shared_ptr<FastType>& type,
        uint16_t topic_id)
{
    // TODO (#5057): allow more than one topic.
    bool rv = false;
    auto it = type_register_.find(attrs.getTopicDataType().c_str());
    if (type_register_.end() == it)
    {
        type = std::make_shared<FastType>(shared_from_this());
        type->setName(attrs.getTopicDataType().c_str());
        type->m_isGetKeyDefined = (attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
        rv = fastrtps::Domain::registerType(ptr_, type.get());
        if (rv)
        {
            topics_.emplace(type->getName(), topic_id);
            type_register_.emplace(type->getName(), type);
        }
    }
    else
    {
        type = it->second.lock();
        rv = true;
    }

    return rv;
}

bool FastParticipant::unregister_topic(
        const std::string& topic_name)
{
    bool rv = false;
    if ((0 != type_register_.erase(topic_name)) && (0 != topics_.erase(topic_name)))
    {
        fastrtps::Domain::unregisterType(ptr_, topic_name.c_str());
        rv = true;
    }
    return rv;
}

bool FastParticipant::find_topic(
        const std::string& topic_name,
        uint16_t& topic_id)
{
    bool rv = false;
    auto it = topics_.find(topic_name);
    if (topics_.end() != it)
    {
        topic_id = it->second;
        rv = true;
    }
    return rv;
}

/**********************************************************************************************************************
 * FastTopic
 **********************************************************************************************************************/
FastType::FastType(
        const std::shared_ptr<FastParticipant>& participant)
    : TopicPubSubType{false}
    , participant_{participant}
{}

FastType::~FastType()
{
    participant_->unregister_topic(getName());
}

FastTopic::FastTopic(
        const std::shared_ptr<FastParticipant>& participant)
    : participant_(participant)
    , type_{nullptr}
{}

bool FastTopic::create_by_attributes(
        const fastrtps::TopicAttributes& attrs,
        uint16_t topic_id)
{
    bool rv = false;
    if (participant_->register_topic(attrs, type_, topic_id))
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
        rv = (0 == std::strcmp(type_->getName(), new_attributes.getTopicDataType().c_str())) &&
             (type_->m_isGetKeyDefined == (new_attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY));
    }
    return rv;
}

bool FastTopic::match_from_xml(const std::string& xml) const
{
    bool rv = false;
    fastrtps::TopicAttributes new_attributes;
    if (xmlobjects::parse_topic(xml.data(), xml.size(), new_attributes))
    {
        rv = (0 == std::strcmp(type_->getName(), new_attributes.getTopicDataType().c_str())) &&
             (type_->m_isGetKeyDefined == (new_attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY));
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
    , request_topic_{nullptr}
    , reply_topic_{nullptr}
    , publisher_ptr_{nullptr}
    , subscriber_ptr_{nullptr}
    , publisher_id_{}
    , sequence_to_sequence_{}
{
}

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
    const fastrtps::TopicAttributes& request_topic_attrs = attrs.publisher.topic;
    if (!participant_->register_topic(request_topic_attrs, request_topic_))
    {
        return false;
    }

    const fastrtps::TopicAttributes& reply_topic_attrs = attrs.subscriber.topic;
    if (!participant_->register_topic(reply_topic_attrs, reply_topic_))
    {
        return false;
    }

    bool rv = false;
    fastrtps::Participant* participant_ptr = participant_.get()->get_ptr();
    publisher_ptr_ = fastrtps::Domain::createPublisher(participant_ptr, attrs.publisher, this);
    subscriber_ptr_ = fastrtps::Domain::createSubscriber(participant_ptr, attrs.subscriber, this);
    rv = (nullptr != publisher_ptr_) && (nullptr != subscriber_ptr_);

    if (rv)
    {
        std::copy(
            std::begin(publisher_ptr_->getGuid().guidPrefix.value),
            std::end(publisher_ptr_->getGuid().guidPrefix.value),
            publisher_id_.guidPrefix().begin());
        std::copy(
            std::begin(publisher_ptr_->getGuid().entityId.value),
            std::begin(publisher_ptr_->getGuid().entityId.value) + 3,
            publisher_id_.entityId().entityKey().begin());
        publisher_id_.entityId().entityKind() = publisher_ptr_->getGuid().entityId.value[3];
    }

    return rv;
}

bool FastRequester::match_from_ref(const std::string& ref) const
{
    bool rv = false;
    fastrtps::RequesterAttributes new_attributes;
    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillRequesterAttributes(ref, new_attributes))
    {
        rv = match(new_attributes);
    }
    return rv;
}

bool FastRequester::match_from_xml(const std::string& xml) const
{
    bool rv = false;
    fastrtps::RequesterAttributes new_attributes;
    if (xmlobjects::parse_requester(xml.data(), xml.size(), new_attributes))
    {
        rv = match(new_attributes);
    }
    return rv;
}

bool FastRequester::write(
        uint32_t sequence_number,
        const std::vector<uint8_t>& data)
{
    bool rv = true;

    try
    {
        fastrtps::rtps::WriteParams wparams;
        rv = publisher_ptr_->write(&const_cast<std::vector<uint8_t>&>(data), wparams);
        if (rv)
        {
            int64_t sequence = (int64_t)wparams.sample_identity().sequence_number().high << 32;
            sequence += wparams.sample_identity().sequence_number().low;
            sequence_to_sequence_.emplace(sequence, sequence_number);
        }
    }
    catch(const std::exception&)
    {
        rv = false;
    }

    return rv;
}

bool FastRequester::read(
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    auto now = std::chrono::steady_clock::now();
    bool rv = false;
    fastrtps::SampleInfo_t info;
    if (unread_count_ != 0)
    {
        rv = subscriber_ptr_->takeNextData(&data, &info);
        unread_count_ = subscriber_ptr_->getUnreadCount();
    }
    else
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (cv_.wait_until(lock, now + timeout, [&](){ return unread_count_ != 0; }))
        {
            lock.unlock();
            rv = subscriber_ptr_->takeNextData(&data, &info);
            unread_count_ = subscriber_ptr_->getUnreadCount();
        }
    }

    if (rv)
    {
        try
        {
            if (info.related_sample_identity.writer_guid() == publisher_ptr_->getGuid())
            {
                int64_t sequence = (int64_t)info.related_sample_identity.sequence_number().high << 32;
                sequence += info.related_sample_identity.sequence_number().low;
                auto it = sequence_to_sequence_.find(sequence);
                if (it != sequence_to_sequence_.end())
                {
                    sequence_number = it->second;
                    sequence_to_sequence_.erase(it);
                }
                else
                {
                    rv = false;
                }
            }
            else
            {
                rv = false;
            }
        }
        catch(const std::exception&)
        {
            rv = false;
        }
    }

    return rv;
}

void FastRequester::onPublicationMatched(
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

void FastRequester::onSubscriptionMatched(
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

void FastRequester::onNewDataMessage(
        fastrtps::Subscriber*)
{
    unread_count_ = subscriber_ptr_->getUnreadCount();
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.notify_one();
}

bool FastRequester::match(const fastrtps::RequesterAttributes& attrs) const
{
    bool rv = false;
    if ((0 == std::strcmp(request_topic_->getName(), attrs.publisher.topic.getTopicDataType().c_str())) &&
         (  request_topic_->m_isGetKeyDefined ==
            (attrs.publisher.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY)) &&
        (0 == std::strcmp(reply_topic_->getName(), attrs.subscriber.topic.getTopicDataType().c_str())) &&
         (  reply_topic_->m_isGetKeyDefined ==
            (attrs.subscriber.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY)))
    {
        rv &= (attrs.publisher == publisher_ptr_->getAttributes() &&
               attrs.subscriber == subscriber_ptr_->getAttributes());
    }
    return rv;
}

/**********************************************************************************************************************
 * FastReplier
 **********************************************************************************************************************/
FastReplier::FastReplier(
        const std::shared_ptr<FastParticipant>& participant)
    : participant_{participant}
    , request_topic_{nullptr}
    , reply_topic_{nullptr}
    , publisher_ptr_{nullptr}
    , subscriber_ptr_{nullptr}
    , unread_count_{0}
{}

FastReplier::~FastReplier()
{
    fastrtps::Domain::removePublisher(publisher_ptr_);
    fastrtps::Domain::removeSubscriber(subscriber_ptr_);
}

bool FastReplier::create_by_ref(
        const std::string& ref)
{
    bool rv = false;
    fastrtps::ReplierAttributes replier_attrs;

    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillReplierAttributes(ref, replier_attrs))
    {
        rv = create_by_attributes(replier_attrs);
    }

    return rv;
}

bool FastReplier::create_by_attributes(
        const fastrtps::ReplierAttributes& attrs)
{

    const fastrtps::TopicAttributes& request_topic_attrs = attrs.subscriber.topic;
    if (!participant_->register_topic(request_topic_attrs, request_topic_))
    {
        return false;
    }

    const fastrtps::TopicAttributes& reply_topic_attrs = attrs.publisher.topic;
    if (!participant_->register_topic(reply_topic_attrs, reply_topic_))
    {
        return false;
    }

    bool rv = false;
    fastrtps::Participant* participant_ptr = participant_.get()->get_ptr();
    publisher_ptr_ = fastrtps::Domain::createPublisher(participant_ptr, attrs.publisher, this);
    subscriber_ptr_ = fastrtps::Domain::createSubscriber(participant_ptr, attrs.subscriber, this);
    rv = (nullptr != publisher_ptr_) && (nullptr != subscriber_ptr_);

    return rv;
}

bool FastReplier::match_from_ref(const std::string& ref) const
{
    bool rv = false;
    fastrtps::ReplierAttributes new_attributes;
    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillReplierAttributes(ref, new_attributes))
    {
        rv = match(new_attributes);
    }
    return rv;
}

bool FastReplier::match_from_xml(const std::string& xml) const
{
    bool rv = false;
    fastrtps::ReplierAttributes new_attributes;
    if (xmlobjects::parse_replier(xml.data(), xml.size(), new_attributes))
    {
        rv = match(new_attributes);
    }
    return rv;
}

void transform_sample_identity(
        const fastrtps::rtps::SampleIdentity& fast_identity,
        dds::SampleIdentity& dds_identity)
{
    std::copy(
        std::begin(fast_identity.writer_guid().guidPrefix.value),
        std::end(fast_identity.writer_guid().guidPrefix.value),
        dds_identity.writer_guid().guidPrefix().begin());
    std::copy(
        std::begin(fast_identity.writer_guid().entityId.value),
        std::begin(fast_identity.writer_guid().entityId.value) + 3,
        dds_identity.writer_guid().entityId().entityKey().begin());
    dds_identity.writer_guid().entityId().entityKind() = fast_identity.writer_guid().entityId.value[3];

    dds_identity.sequence_number().high() = fast_identity.sequence_number().high;
    dds_identity.sequence_number().low() = fast_identity.sequence_number().low;
}

void transport_sample_identity(
        const dds::SampleIdentity& dds_identity,
        fastrtps::rtps::SampleIdentity& fast_identity)
{
    std::copy(
        dds_identity.writer_guid().guidPrefix().begin(),
        dds_identity.writer_guid().guidPrefix().end(),
        std::begin(fast_identity.writer_guid().guidPrefix.value));
    std::copy(
        dds_identity.writer_guid().entityId().entityKey().begin(),
        dds_identity.writer_guid().entityId().entityKey().end(),
        std::begin(fast_identity.writer_guid().entityId.value));
    fast_identity.writer_guid().entityId.value[3] = dds_identity.writer_guid().entityId().entityKind();

    fast_identity.sequence_number().high = dds_identity.sequence_number().high();
    fast_identity.sequence_number().low = dds_identity.sequence_number().low();
}

bool FastReplier::write(
        const std::vector<uint8_t>& data)
{
    fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(const_cast<uint8_t*>(data.data())), data.size()};
    fastcdr::Cdr deserializer(fastbuffer);

    dds::SampleIdentity sample_identity;
    sample_identity.deserialize(deserializer);

    fastrtps::rtps::WriteParams wparams;
    transport_sample_identity(sample_identity, wparams.related_sample_identity());

    std::vector<uint8_t> output_data(data.size() - deserializer.getSerializedDataLength());
    deserializer.deserializeArray(output_data.data(), output_data.size());

    return publisher_ptr_->write(&const_cast<std::vector<uint8_t>&>(output_data), wparams);
}

bool FastReplier::read(
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    std::vector<uint8_t> temp_data;
    auto now = std::chrono::steady_clock::now();
    bool rv = false;
    fastrtps::SampleInfo_t info;

    if (unread_count_ != 0)
    {
        rv = subscriber_ptr_->takeNextData(&temp_data, &info);
        unread_count_ = subscriber_ptr_->getUnreadCount();
    }
    else
    {
        std::unique_lock<std::mutex> lock(mtx_);
        if (cv_.wait_until(lock, now + timeout, [&](){ return unread_count_ != 0; }))
        {
            lock.unlock();
            rv = subscriber_ptr_->takeNextData(&temp_data, &info);
            unread_count_ = subscriber_ptr_->getUnreadCount();
        }
    }

    if (rv)
    {
        dds::SampleIdentity sample_identity;
        transform_sample_identity(info.sample_identity, sample_identity);

        data.clear();
        data.resize(sample_identity.getCdrSerializedSize() + temp_data.size());

        fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(data.data()), data.size()};
        fastcdr::Cdr serializer(fastbuffer);

        try
        {
            sample_identity.serialize(serializer);
            serializer.serializeArray(temp_data.data(), temp_data.size());
        }
        catch(const std::exception&)
        {
            rv = false;
        }
    }

    return rv;
}

void FastReplier::onPublicationMatched(
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

void FastReplier::onSubscriptionMatched(
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

void FastReplier::onNewDataMessage(
        fastrtps::Subscriber*)
{
    unread_count_ = subscriber_ptr_->getUnreadCount();
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.notify_one();
}

bool FastReplier::match(const fastrtps::ReplierAttributes& attrs) const
{
    bool rv = false;
    if ((0 == std::strcmp(reply_topic_->getName(), attrs.publisher.topic.getTopicDataType().c_str())) &&
         (  reply_topic_->m_isGetKeyDefined ==
            (attrs.publisher.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY)) &&
        (0 == std::strcmp(request_topic_->getName(), attrs.subscriber.topic.getTopicDataType().c_str())) &&
         (  request_topic_->m_isGetKeyDefined ==
            (attrs.subscriber.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY)))
    {
        rv &= (attrs.publisher == publisher_ptr_->getAttributes() &&
               attrs.subscriber == subscriber_ptr_->getAttributes());
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
