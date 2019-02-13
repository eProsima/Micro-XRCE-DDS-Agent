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

namespace eprosima {
namespace fastrtps {

class ParticipantAttributes;
class PublisherAttributes;
class SubscriberAttributes;

} // namespace fastrtps
} // namespace eprosima

namespace eprosima {
namespace uxr {

class FastParticipant : public fastrtps::ParticipantListener
{
public:
    FastParticipant() = default;
    ~FastParticipant() override;

    bool create_by_ref(const std::string& ref);
    // TODO (julian: #4372): add const qualifier in attrs.
    bool create_by_attributes(fastrtps::ParticipantAttributes& attrs);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;
    bool remove();
    void onParticipantDiscovery(fastrtps::Participant*, fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;
    fastrtps::Participant* get_ptr() const { return ptr_; }

private:
    fastrtps::Participant* ptr_;
};

class FastTopic : public TopicPubSubType
{
public:
    FastTopic() : TopicPubSubType(false), participant_(nullptr) {}
    ~FastTopic();

    bool create_by_attributes(const fastrtps::TopicAttributes& attrs, FastParticipant* participant);
    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;

private:
    FastParticipant* participant_;
};

class FastPublisher
{
public:
    FastPublisher(uint16_t participant_id) : participant_id_(participant_id) {}
    ~FastPublisher() = default;

    uint16_t get_participant_id() { return participant_id_; }

private:
    uint16_t participant_id_;
};

class FastSubscriber
{
public:
    FastSubscriber(uint16_t participant_id) : participant_id_(participant_id) {}
    ~FastSubscriber() = default;

    uint16_t get_participant_id() { return participant_id_; }

private:
    uint16_t participant_id_;
};

class FastDataWriter : public fastrtps::PublisherListener
{
public:
    FastDataWriter() = default;
    ~FastDataWriter() override;

    bool create_by_ref(const std::string& ref, const FastParticipant* participant, std::string& topic_name);
    // TODO (julian: #4372): add const qualifier in attrs.
    bool create_by_attributes(fastrtps::PublisherAttributes& attrs,
                              const FastParticipant* participant,
                              std::string& topic_name);

    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;

    bool write(std::vector<uint8_t>& data);
    void onPublicationMatched(fastrtps::Publisher*, fastrtps::rtps::MatchingInfo& info) override;
    fastrtps::Publisher* get_ptr() { return ptr_; }

private:
    fastrtps::Publisher* ptr_;
};

class FastDataReader : public fastrtps::SubscriberListener
{
public:
    FastDataReader();
    ~FastDataReader() override;

    bool create_by_ref(const std::string& ref,
                       const FastParticipant* participant,
                       std::string& topic_name);

    // TODO (julian: #4372): add const qualifier in attrs.
    bool create_by_attributes(fastrtps::SubscriberAttributes& attrs,
                              const FastParticipant* participant,
                              std::string& topic_name);

    bool match_from_ref(const std::string& ref) const;
    bool match_from_xml(const std::string& xml) const;

    bool set_on_new_data_cb(OnNewData on_new_data_cb);
    bool unset_on_new_data_cb();
    bool read(std::vector<uint8_t>* data);
    void onSubscriptionMatched(fastrtps::Subscriber* sub, fastrtps::rtps::MatchingInfo& info) override;
    void onNewDataMessage(fastrtps::Subscriber*) override;
    fastrtps::Subscriber* get_ptr() { return ptr_; }

private:
    fastrtps::Subscriber* ptr_;
    OnNewData on_new_data_cb_;
};


} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_FAST_FAST_ENTITIES_HPP_
