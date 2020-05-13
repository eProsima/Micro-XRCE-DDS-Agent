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
class FastDataWriter
{
public:
    FastDataWriter(
            fastrtps::Publisher* impl_,
            const std::shared_ptr<FastTopic>& topic,
            const std::shared_ptr<FastParticipant>& participant);

    ~FastDataWriter();

    bool match(
            const fastrtps::PublisherAttributes& attrs) const;

    bool write(
            const std::vector<uint8_t>& data);

    const fastrtps::Publisher* get_ptr() const { return impl_; }

private:
    fastrtps::Publisher* impl_;
    std::shared_ptr<FastTopic> topic_;
    std::shared_ptr<FastParticipant> participant_;
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
            const std::string& ref);

    bool create_by_attributes(
            const fastrtps::SubscriberAttributes& attrs);

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
    std::shared_ptr<FastTopic> topic_;
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
            const std::shared_ptr<FastParticipant>& participant,
            const std::shared_ptr<FastTopic>& request_topic,
            const std::shared_ptr<FastTopic>& reply_topic);

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
            uint32_t& sequence_number,
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
    bool match(const fastrtps::RequesterAttributes& attrs) const;

private:
    std::shared_ptr<FastParticipant> participant_;
    std::shared_ptr<FastTopic> request_topic_;
    std::shared_ptr<FastTopic> reply_topic_;
    fastrtps::Publisher* publisher_ptr_;
    fastrtps::Subscriber* subscriber_ptr_;
    dds::GUID_t publisher_id_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<uint64_t> unread_count_;
    std::map<int64_t, uint32_t> sequence_to_sequence_;
};

/**********************************************************************************************************************
 * FastReplier
 **********************************************************************************************************************/
class FastReplier : public fastrtps::SubscriberListener, public fastrtps::PublisherListener
{
public:
    FastReplier(
            const std::shared_ptr<FastParticipant>& participant,
            const std::shared_ptr<FastTopic>& request_topic,
            const std::shared_ptr<FastTopic>& reply_topic);

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
    bool match(const fastrtps::ReplierAttributes& attrs) const;

private:
    std::shared_ptr<FastParticipant> participant_;
    std::shared_ptr<FastTopic> request_topic_;
    std::shared_ptr<FastTopic> reply_topic_;
    fastrtps::Publisher* publisher_ptr_;
    fastrtps::Subscriber* subscriber_ptr_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<uint64_t> unread_count_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_FAST_FAST_ENTITIES_HPP_
