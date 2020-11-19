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

#ifndef UXR_AGENT_MIDDLEWARE_FAST_FAST_ENTITIES_HPP_
#define UXR_AGENT_MIDDLEWARE_FAST_FAST_ENTITIES_HPP_

#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <uxr/agent/types/TopicPubSubType.hpp>
#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/types/XRCETypes.hpp>

#include <unordered_map>
#include <condition_variable>
#include <atomic>

namespace eprosima {
namespace fastrtps {

class ParticipantAttributes;
class PublisherAttributes;
class SubscriberAttributes;
class RequesterAttributes;
class ReplierAttributes;
class SampleInfo_t;

} // namespace fastrtps
} // namespace eprosima

namespace eprosima {
namespace uxr {

class FastType;
class FastTopic;

/**********************************************************************************************************************
 * Listeners
 **********************************************************************************************************************/
class FastListener : public fastrtps::ParticipantListener
                   , public fastrtps::PublisherListener
                   , public fastrtps::SubscriberListener
{
public:
    void onParticipantDiscovery(
        fastrtps::Participant*,
        fastrtps::rtps::ParticipantDiscoveryInfo&& info) final;

    void onPublicationMatched(
        fastrtps::Publisher*,
        fastrtps::rtps::MatchingInfo& info) final;

    void onSubscriptionMatched(
        fastrtps::Subscriber*,
        fastrtps::rtps::MatchingInfo& info) final;
};

/**********************************************************************************************************************
 * FastParticipant
 **********************************************************************************************************************/
class FastParticipant
{
public:
    FastParticipant(fastrtps::Participant* impl);

    ~FastParticipant();

    bool match(
            const fastrtps::ParticipantAttributes& attrs) const;

    fastrtps::Participant* get_ptr() const { return impl_; }

    bool register_type(
            const std::shared_ptr<FastType>& type);

    bool unregister_type(
            const std::string& type_name);

    std::shared_ptr<FastType> find_type(
            const std::string& type_name) const;

    bool register_topic(
            const std::shared_ptr<FastTopic>& topic);

    bool unregister_topic(
            const std::string& topic_name);

    std::shared_ptr<FastTopic> find_topic(
            const std::string& topic_name) const;

    int16_t domain_id() const;

    const fastrtps::rtps::GUID_t& get_guid() const;

private:
    fastrtps::Participant* impl_;
    std::unordered_map<std::string, std::weak_ptr<FastType>> type_register_;
    std::unordered_map<std::string, std::weak_ptr<FastTopic>> topic_register_;
};

/**********************************************************************************************************************
 * FastTopic
 **********************************************************************************************************************/
class FastType : public TopicPubSubType
{
public:
    FastType(
            const std::shared_ptr<FastParticipant>& participant);

    ~FastType();

private:
    std::shared_ptr<FastParticipant> participant_;
};

class FastTopic
{
public:
    FastTopic(
            const std::string& name,
            const std::shared_ptr<FastType>& type,
            const std::shared_ptr<FastParticipant>& participant);

    ~FastTopic();

    const std::string& get_name() const { return name_; }

    const std::shared_ptr<FastType>& get_type() const { return type_; }

    bool match(
            const fastrtps::TopicAttributes& attrs) const;

private:
    std::string name_;
    std::shared_ptr<FastType> type_;
    std::shared_ptr<FastParticipant> participant_;
};

/**********************************************************************************************************************
 * FastPublisher
 **********************************************************************************************************************/
class FastPublisher
{
public:
    FastPublisher(
            const std::shared_ptr<FastParticipant>& participant)
        : participant_(participant)
    {}

    ~FastPublisher() = default;

    const std::shared_ptr<FastParticipant>& get_participant() const { return participant_; }

private:
        std::shared_ptr<FastParticipant> participant_;
};

/**********************************************************************************************************************
 * FastSubscriber
 **********************************************************************************************************************/
class FastSubscriber
{
public:
    FastSubscriber(
            const std::shared_ptr<FastParticipant>& participant)
        : participant_(participant)
    {}

    ~FastSubscriber() = default;

    const std::shared_ptr<FastParticipant>& get_participant() const { return participant_; }

private:
        std::shared_ptr<FastParticipant> participant_;
};

/**********************************************************************************************************************
 * FastDataWriter
 **********************************************************************************************************************/
class FastDataWriter
{
public:
    FastDataWriter(
            fastrtps::Publisher* impl_,
            const std::shared_ptr<FastTopic>& topic,
            const std::shared_ptr<FastPublisher>& publisher);

    ~FastDataWriter();

    bool match(
            const fastrtps::PublisherAttributes& attrs) const;

    bool write(
            const std::vector<uint8_t>& data);

    bool write(
            const std::vector<uint8_t>& data,
            fastrtps::rtps::WriteParams& wparams);

    const fastrtps::rtps::GUID_t& get_guid() const;

    const fastrtps::Participant* get_participant() const;

    const fastrtps::Publisher* get_ptr() const;

private:
    fastrtps::Publisher* impl_;
    std::shared_ptr<FastTopic> topic_;
    std::shared_ptr<FastPublisher> publisher_;
};

/**********************************************************************************************************************
 * FastDataReader
 **********************************************************************************************************************/
class FastDataReader
{
public:
    FastDataReader(
            fastrtps::Subscriber* impl_,
            const std::shared_ptr<FastTopic>& topic,
            const std::shared_ptr<FastSubscriber>& Subscriber);

    ~FastDataReader();

    bool match(
            const fastrtps::SubscriberAttributes& attrs) const;

    bool read(
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout);

    bool read(
            std::vector<uint8_t>& data,
            fastrtps::SampleInfo_t& info,
            std::chrono::milliseconds timeout);

    const fastrtps::rtps::GUID_t& get_guid() const;

    const fastrtps::Participant* get_participant() const;

    const fastrtps::Subscriber* get_ptr() const;

private:
    fastrtps::Subscriber* impl_;
    std::shared_ptr<FastTopic> topic_;
    std::shared_ptr<FastSubscriber> subscriber_;
};


/**********************************************************************************************************************
 * FastRequester
 **********************************************************************************************************************/
class FastRequester
{
public:
    FastRequester(
            const std::shared_ptr<FastDataWriter>& datawriter,
            const std::shared_ptr<FastDataReader>& datareader);

    ~FastRequester() = default;

    bool match(
            const fastrtps::RequesterAttributes& attrs) const;

    bool write(
            uint32_t sequence_number,
            const std::vector<uint8_t>& data);

    bool read(
            uint32_t& sequence_number,
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout);

    const fastrtps::Participant* get_participant() const;

    const fastrtps::Publisher* get_request_datawriter() const;

    const fastrtps::Subscriber* get_reply_datareader() const;

private:
    std::shared_ptr<FastDataWriter> datawriter_;
    std::shared_ptr<FastDataReader> datareader_;
    dds::GUID_t publisher_id_;
    std::map<int64_t, uint32_t> sequence_to_sequence_;
};

/**********************************************************************************************************************
 * FastReplier
 **********************************************************************************************************************/
class FastReplier
{
public:
    FastReplier(
            const std::shared_ptr<FastDataWriter>& datawriter,
            const std::shared_ptr<FastDataReader>& datareader);

    ~FastReplier() = default;

    bool match(
            const fastrtps::ReplierAttributes& attrs) const;

    bool write(
            const std::vector<uint8_t>& data);

    bool read(
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout);

    const fastrtps::Participant* get_participant() const;

    const fastrtps::Subscriber* get_request_datareader() const;

    const fastrtps::Publisher* get_reply_datawriter() const;

private:
    std::shared_ptr<FastDataWriter> datawriter_;
    std::shared_ptr<FastDataReader> datareader_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_FAST_FAST_ENTITIES_HPP_
