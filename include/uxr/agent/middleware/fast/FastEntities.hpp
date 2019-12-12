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
#include <fastrtps/attributes/RequesterAttributes.h> // TODO (julianbermudez): remove and user forward declaration.
#include <fastrtps/attributes/ReplierAttributes.h>
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

} // namespace fastrtps
} // namespace eprosima

namespace eprosima {
namespace uxr {

class FastTopic;

/**********************************************************************************************************************
 * FastParticipant
 **********************************************************************************************************************/
class FastParticipant : public fastrtps::ParticipantListener
{
public:
    FastParticipant(int16_t domain_id)
        : domain_id_(domain_id)
        , ptr_(nullptr)
        , topics_register_{}
    {}

    ~FastParticipant() override;

    bool create_by_ref(const std::string& ref);

    bool create_by_attributes(const fastrtps::ParticipantAttributes& attrs);

    bool match_from_ref(const std::string& ref) const;

    bool match_from_xml(const std::string& xml) const;

    bool remove();

    void onParticipantDiscovery(
            fastrtps::Participant*,
            fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    fastrtps::Participant* get_ptr() const { return ptr_; }

    bool register_topic(
            FastTopic* const topic,
            uint16_t topic_id);

    bool unregister_topic(const FastTopic* const topic);

    bool find_topic(
            const std::string& topic_name,
            uint16_t& topic_id);

    int16_t domain_id() { return domain_id_; }

private:
    int16_t domain_id_;
    fastrtps::Participant* ptr_;
    std::unordered_map<std::string, uint16_t> topics_register_;
};

/**********************************************************************************************************************
 * FastTopic
 **********************************************************************************************************************/
class FastTopic : public TopicPubSubType
{
public:
    FastTopic(const std::shared_ptr<FastParticipant>& participant);

    ~FastTopic();

    bool create_by_attributes(
            const fastrtps::TopicAttributes& attrs,
            uint16_t topic_id);

    bool match_from_ref(const std::string& ref) const;

    bool match_from_xml(const std::string& xml) const;

private:
    std::shared_ptr<FastParticipant> participant_;
};

/**********************************************************************************************************************
 * FastPublisher
 **********************************************************************************************************************/
class FastPublisher
{
public:
    FastPublisher(uint16_t participant_id) : participant_id_(participant_id) {}
    ~FastPublisher() = default;

    uint16_t get_participant_id() { return participant_id_; }

private:
    uint16_t participant_id_;
};

/**********************************************************************************************************************
 * FastSubscriber
 **********************************************************************************************************************/
class FastSubscriber
{
public:
    FastSubscriber(uint16_t participant_id) : participant_id_(participant_id) {}
    ~FastSubscriber() = default;

    uint16_t get_participant_id() { return participant_id_; }

private:
    uint16_t participant_id_;
};

/**********************************************************************************************************************
 * FastDataWriter
 **********************************************************************************************************************/
class FastDataWriter : public fastrtps::PublisherListener
{
public:
    FastDataWriter(const std::shared_ptr<FastParticipant>& participant);

    ~FastDataWriter() override;

    bool create_by_ref(
            const std::string& ref,
            uint16_t& topic_id);

    bool create_by_attributes(
            const fastrtps::PublisherAttributes& attrs,
            uint16_t& topic_id);

    bool match_from_ref(const std::string& ref) const;

    bool match_from_xml(const std::string& xml) const;

    bool write(const std::vector<uint8_t>& data);

    void onPublicationMatched(
            fastrtps::Publisher*,
            fastrtps::rtps::MatchingInfo& info) override;

    const fastrtps::Publisher* get_ptr() const { return ptr_; }

private:
    std::shared_ptr<FastParticipant> participant_;
    fastrtps::Publisher* ptr_;
};

/**********************************************************************************************************************
 * FastDataReader
 **********************************************************************************************************************/
class FastDataReader : public fastrtps::SubscriberListener
{
public:
    FastDataReader(const std::shared_ptr<FastParticipant>& participant);

    ~FastDataReader() override;

    bool create_by_ref(
            const std::string& ref,
            uint16_t& topic_id);

    bool create_by_attributes(
            const fastrtps::SubscriberAttributes& attrs,
            uint16_t& topic_id);

    bool match_from_ref(const std::string& ref) const;

    bool match_from_xml(const std::string& xml) const;

    bool read(
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout);

    void onSubscriptionMatched(
            fastrtps::Subscriber* sub,
            fastrtps::rtps::MatchingInfo& info) override;

    void onNewDataMessage(fastrtps::Subscriber*) override;

    const fastrtps::Subscriber* get_ptr() const { return ptr_; }

private:
    std::shared_ptr<FastParticipant> participant_;
    fastrtps::Subscriber* ptr_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<uint64_t> unread_count_;
};


/**********************************************************************************************************************
 * FastRequester
 **********************************************************************************************************************/
class FastRequester : public fastrtps::SubscriberListener, public fastrtps::PublisherListener
{
public:
    FastRequester(
            const std::shared_ptr<FastParticipant>& participant);

    ~FastRequester() override;

    bool create_by_ref(
            const std::string& ref);

    bool create_by_attributes(
            const fastrtps::RequesterAttributes& attrs);

    bool match_from_ref(
            const std::string& ref) const;

    bool match_from_xml(
            const std::string& xml) const;

    bool write(
            uint32_t sequence_number,
            const std::vector<uint8_t>& data);

    bool read(
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout);

    void onPublicationMatched(
            fastrtps::Publisher*,
            fastrtps::rtps::MatchingInfo& info) override;

    void onSubscriptionMatched(
            fastrtps::Subscriber* sub,
            fastrtps::rtps::MatchingInfo& info) override;

    void onNewDataMessage(
            fastrtps::Subscriber*) override;

private:
    std::shared_ptr<FastParticipant> participant_;
    TopicPubSubType request_topic_;
    TopicPubSubType reply_topic_;
    fastrtps::Publisher* publisher_ptr_;
    fastrtps::Subscriber* subscriber_ptr_;
};

/**********************************************************************************************************************
 * FastReplier
 **********************************************************************************************************************/
class FastReplier : public fastrtps::SubscriberListener, public fastrtps::PublisherListener
{
public:
    FastReplier(
            const std::shared_ptr<FastParticipant>& participant);

    ~FastReplier() override;

    bool create_by_ref(
            const std::string& ref);

    bool create_by_attributes(
            const fastrtps::ReplierAttributes& attrs);

    bool match_from_ref(
            const std::string& ref) const;

    bool match_from_xml(
            const std::string& xml) const;

    bool write(
            const std::vector<uint8_t>& data);

    bool read(
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout);

    void onPublicationMatched(
            fastrtps::Publisher*,
            fastrtps::rtps::MatchingInfo& info) override;

    void onSubscriptionMatched(
            fastrtps::Subscriber* sub,
            fastrtps::rtps::MatchingInfo& info) override;

    void onNewDataMessage(
            fastrtps::Subscriber*) override;

private:
    std::shared_ptr<FastParticipant> participant_;
    TopicPubSubType request_topic_;
    TopicPubSubType reply_topic_;
    fastrtps::Publisher* publisher_ptr_;
    fastrtps::Subscriber* subscriber_ptr_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<uint64_t> unread_count_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_FAST_FAST_ENTITIES_HPP_
