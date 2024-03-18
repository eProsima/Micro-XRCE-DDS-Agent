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
 * FastListener
 **********************************************************************************************************************/
void FastListener::onParticipantDiscovery(
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

void FastListener::onPublicationMatched(
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

void FastListener::onSubscriptionMatched(
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

/**********************************************************************************************************************
 * FastParticipant
 **********************************************************************************************************************/
FastParticipant::FastParticipant(
        fastrtps::Participant* impl)
    : impl_{impl}
    , type_register_{}
    , topic_register_{}
{}

FastParticipant::~FastParticipant()
{
    fastrtps::Domain::removeParticipant(impl_);
}

bool FastParticipant::match(
        const fastrtps::ParticipantAttributes& attrs) const
{
    return attrs == impl_->getAttributes();
}

bool FastParticipant::register_type(
        const std::shared_ptr<FastType>& type)
{
    return fastrtps::Domain::registerType(impl_, type.get())
        && type_register_.emplace(type->getName(), type).second;
}

bool FastParticipant::unregister_type(
        const std::string& type_name)
{
    return (1 == type_register_.erase(type_name))
        && fastrtps::Domain::unregisterType(impl_, type_name.c_str());
}

std::shared_ptr<FastType> FastParticipant::find_type(
        const std::string& type_name) const
{
    std::shared_ptr<FastType> type;
    auto it = type_register_.find(type_name);
    if (it != type_register_.end())
    {
        type = it->second.lock();
    }
    return type;
}

bool FastParticipant::register_topic(
        const std::shared_ptr<FastTopic>& topic)
{
    return topic_register_.emplace(topic->get_name(), topic).second;
}

bool FastParticipant::unregister_topic(
        const std::string& topic_name)
{
    return (1 == topic_register_.erase(topic_name));
}

std::shared_ptr<FastTopic> FastParticipant::find_topic(
        const std::string& topic_name) const
{
    std::shared_ptr<FastTopic> topic;
    auto it = topic_register_.find(topic_name);
    if (it != topic_register_.end())
    {
        topic = it->second.lock();
    }
    return topic;
}

int16_t FastParticipant::domain_id() const
{
    return (int16_t) impl_->getAttributes().domainId;
}

const fastrtps::rtps::GUID_t& FastParticipant::get_guid() const
{
  return impl_->getGuid();
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
    participant_->unregister_type(getName());
}

FastTopic::FastTopic(
        const std::string& name,
        const std::shared_ptr<FastType>& type,
        const std::shared_ptr<FastParticipant>& participant)
    : name_{name}
    , type_{type}
    , participant_(participant)
{}

FastTopic::~FastTopic()
{
    participant_->unregister_topic(name_);
}

bool FastTopic::match(const fastrtps::TopicAttributes& attrs) const
{
    return (attrs.getTopicName() == name_)
        && (0 == std::strcmp(type_->getName(), attrs.getTopicDataType().c_str()))
        && (type_->m_isGetKeyDefined == (attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY));
}

/**********************************************************************************************************************
 * FastDataWriter
 **********************************************************************************************************************/
FastDataWriter::FastDataWriter(
        fastrtps::Publisher* impl,
        const std::shared_ptr<FastTopic>& topic,
        const std::shared_ptr<FastPublisher>& publisher)
    : impl_{impl}
    , topic_{topic}
    , publisher_{publisher}
{}

FastDataWriter::~FastDataWriter()
{
    fastrtps::Domain::removePublisher(impl_);
}

bool FastDataWriter::match(
        const fastrtps::PublisherAttributes& attrs) const
{
    return (attrs == impl_->getAttributes());
}

bool FastDataWriter::write(
        const std::vector<uint8_t>& data)
{
    return impl_->write(&const_cast<std::vector<uint8_t>&>(data));
}

bool FastDataWriter::write(
        const std::vector<uint8_t>& data,
        fastrtps::rtps::WriteParams& wparams)
{
    return impl_->write(&const_cast<std::vector<uint8_t>&>(data), wparams);
}

const fastrtps::rtps::GUID_t& FastDataWriter::get_guid() const
{
    return impl_->getGuid();
}

const fastrtps::Participant* FastDataWriter::get_participant() const
{
    return publisher_->get_participant()->get_ptr();
}

const fastrtps::Publisher* FastDataWriter::get_ptr() const
{
    return impl_;
}

/**********************************************************************************************************************
 * FastDataReader
 **********************************************************************************************************************/
FastDataReader::FastDataReader(
        fastrtps::Subscriber* impl,
        const std::shared_ptr<FastTopic>& topic,
        const std::shared_ptr<FastSubscriber>& subscriber)
    : impl_{impl}
    , topic_{topic}
    , subscriber_{subscriber}
{}

FastDataReader::~FastDataReader()
{
    fastrtps::Domain::removeSubscriber(impl_);
}

bool FastDataReader::match(
        const fastrtps::SubscriberAttributes& attrs) const
{
    return (attrs == impl_->getAttributes());
}

bool FastDataReader::read(
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    eprosima::fastrtps::Duration_t tm =
        {int32_t(timeout.count() / 1000), uint32_t(timeout.count() * 1000000)};
    if (impl_->wait_for_unread_samples(tm))
    {
        fastrtps::SampleInfo_t info;
        rv = impl_->takeNextData(&data, &info);
    }
    return rv;
}

bool FastDataReader::read(
        std::vector<uint8_t>& data,
        fastrtps::SampleInfo_t& info,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    eprosima::fastrtps::Duration_t tm =
        {int32_t(timeout.count() / 1000), uint32_t(timeout.count() * 1000000)};
    if (impl_->wait_for_unread_samples(tm))
    {
        rv = impl_->takeNextData(&data, &info);
    }
    return rv;
}

const fastrtps::rtps::GUID_t& FastDataReader::get_guid() const
{
    return impl_->getGuid();
}

const fastrtps::Participant* FastDataReader::get_participant() const
{
    return subscriber_->get_participant()->get_ptr();
}

const fastrtps::Subscriber* FastDataReader::get_ptr() const
{
    return impl_;
}

/**********************************************************************************************************************
 * FastRequester
 **********************************************************************************************************************/
FastRequester::FastRequester(
        const std::shared_ptr<FastDataWriter>& datawriter,
        const std::shared_ptr<FastDataReader>& datareader)
    : datawriter_{datawriter}
    , datareader_{datareader}
    , publisher_id_{}
    , sequence_to_sequence_{}
{
        std::copy(
            std::begin(datawriter_->get_guid().guidPrefix.value),
            std::end(datawriter_->get_guid().guidPrefix.value),
            publisher_id_.guidPrefix().begin());
        std::copy(
            std::begin(datawriter_->get_guid().entityId.value),
            std::begin(datawriter_->get_guid().entityId.value) + 3,
            publisher_id_.entityId().entityKey().begin());
        publisher_id_.entityId().entityKind() = datawriter_->get_guid().entityId.value[3];
}

bool FastRequester::match(
    const fastrtps::RequesterAttributes& attrs) const
{
    return datawriter_->match(attrs.publisher)
        && datareader_->match(attrs.subscriber);
}

bool FastRequester::write(
        uint32_t sequence_number,
        const std::vector<uint8_t>& data)
{
    bool rv = true;
    try
    {
        fastrtps::rtps::WriteParams wparams;
        rv = datawriter_->write(data, wparams);
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
    bool rv = false;
    fastrtps::SampleInfo_t info;
    datareader_->read(data, info, timeout);

    if (rv)
    {
        try
        {
            if (info.related_sample_identity.writer_guid() == datawriter_->get_guid())
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

const fastrtps::Participant* FastRequester::get_participant() const
{
    const fastrtps::Participant* participant = datawriter_->get_participant();
    if (participant != datareader_->get_participant())
    {
        participant = nullptr;
    }

    return participant;
}

const fastrtps::Publisher* FastRequester::get_request_datawriter() const
{
    return datawriter_->get_ptr();
}

const fastrtps::Subscriber* FastRequester::get_reply_datareader() const
{
    return datareader_->get_ptr();
}

/**********************************************************************************************************************
 * FastReplier
 **********************************************************************************************************************/
FastReplier::FastReplier(
        const std::shared_ptr<FastDataWriter>& datawriter,
        const std::shared_ptr<FastDataReader>& datareader)
    : datawriter_{datawriter}
    , datareader_{datareader}
{}

bool FastReplier::match(
        const fastrtps::ReplierAttributes& attrs) const
{
    return datawriter_->match(attrs.publisher)
        && datareader_->match(attrs.subscriber);
}

inline
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

inline
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
    fastcdr::Cdr deserializer(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::CdrVersion::XCDRv1);

    dds::SampleIdentity sample_identity;
    sample_identity.deserialize(deserializer);

    fastrtps::rtps::WriteParams wparams;
    transport_sample_identity(sample_identity, wparams.related_sample_identity());

    std::vector<uint8_t> output_data(data.size() - deserializer.get_serialized_data_length());
    deserializer.deserialize_array(output_data.data(), output_data.size());

    return datawriter_->write(output_data, wparams);
}

bool FastReplier::read(
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    std::vector<uint8_t> temp_data;
    bool rv = false;
    fastrtps::SampleInfo_t info;
    datareader_->read(temp_data, info, timeout);

    if (rv)
    {
        dds::SampleIdentity sample_identity;
        transform_sample_identity(info.sample_identity, sample_identity);

        data.clear();
        data.resize(sample_identity.getCdrSerializedSize() + temp_data.size());

        fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(data.data()), data.size()};
        fastcdr::Cdr serializer(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::CdrVersion::XCDRv1);

        try
        {
            sample_identity.serialize(serializer);
            serializer.serialize_array(temp_data.data(), temp_data.size());
        }
        catch(const std::exception&)
        {
            rv = false;
        }
    }

    return rv;
}

const fastrtps::Participant* FastReplier::get_participant() const
{
    const fastrtps::Participant* participant = datawriter_->get_participant();
    if (participant != datareader_->get_participant())
    {
        participant = nullptr;
    }

    return participant;
}

const fastrtps::Subscriber* FastReplier::get_request_datareader() const
{
    return datareader_->get_ptr();
}

const fastrtps::Publisher* FastReplier::get_reply_datawriter() const
{
    return datawriter_->get_ptr();
}

} // namespace uxr
} // namespace eprosima
