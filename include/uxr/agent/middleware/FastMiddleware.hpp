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


#ifndef _UXR_AGENT_FAST_MIDDLEWARE_HPP_
#define _UXR_AGENT_FAST_MIDDLEWARE_HPP_

#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/types/TopicPubSubType.hpp>
#include <fastrtps/participant/ParticipantListener.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/subscriber/SubscriberListener.h>

#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <memory>

namespace eprosima {
namespace fastrtps {

//class Participant;

} // namespace fastrtps
} // namespace eprosima

namespace eprosima {
namespace uxr {

/*************************************************************************************************
 * Fake Publisher & Subscriber.
 *************************************************************************************************/
class FakePublisher
{
public:
    FakePublisher(uint16_t participant_id) : participant_id_(participant_id) {}

    uint16_t get_participant_id() { return participant_id_; }

private:
    uint16_t participant_id_;
};

class FakeSubscriber
{
public:
    FakeSubscriber(uint16_t participant_id) : participant_id_(participant_id) {}

    uint16_t get_participant_id() { return participant_id_; }

private:
    uint16_t participant_id_;
};

/*************************************************************************************************
 * Middleware.
 *************************************************************************************************/
class FastMiddleware : public Middleware,
                       public fastrtps::ParticipantListener,
                       public fastrtps::PublisherListener,
                       public fastrtps::SubscriberListener
{
public:
    FastMiddleware() = default;
    virtual ~FastMiddleware() override = default;

    /* Creation functions. */
    bool create_participant_from_ref(uint16_t participant_id, const std::string& ref) override;
    bool create_participant_from_xml(uint16_t participant_id, const std::string& xml) override;

    bool create_topic_from_ref(uint16_t topic_id, uint16_t participant_id, const std::string& ref) override;
    bool create_topic_from_xml(uint16_t topic_id, uint16_t participant_id, const std::string& xml) override;

    bool create_publisher_from_ref(uint16_t publisher_id, uint16_t participant_id, const std::string&) override;
    bool create_publisher_from_xml(uint16_t publisher_id, uint16_t participant_id, const std::string&) override;

    bool create_subcriber_from_ref(uint16_t subscriber_id, uint16_t participant_id, const std::string&) override;
    bool create_subcriber_from_xml(uint16_t subscirber_id, uint16_t participant_id, const std::string&) override;

    bool create_datawriter_from_ref(uint16_t datawriter_id,
                                    uint16_t publisher_id,
                                    const std::string& ref,
                                    uint16_t& associated_topic_id) override;

    bool create_datawriter_from_xml(uint16_t datawriter_id,
                                    uint16_t publisher_id,
                                    const std::string& xml,
                                    uint16_t& associated_topic_id) override;

    bool create_datareader_from_ref(uint16_t datareader_id,
                                    uint16_t subscriber_id,
                                    const std::string& ref,
                                    uint16_t& associated_topic_id) override;

    bool create_datareader_from_xml(uint16_t datareader_id,
                                    uint16_t subscriber_id,
                                    const std::string& xml,
                                    uint16_t& associated_topic_id) override;

    /* Deletion functions. */
    bool delete_participant(uint16_t participant_id) override;
    bool delete_topic(uint16_t topic_id, uint16_t participant_id) override;
    bool delete_publisher(uint16_t publisher_id, uint16_t participant_id) override;
    bool delete_subcriber(uint16_t subscriber_id, uint16_t participant_id) override;
    bool delete_datawriter(uint16_t datawriter_id, uint16_t publisher_id) override;
    bool delete_datareader(uint16_t datareader_id, uint16_t subscriber_id) override;

    /* Write and read functions. */
    bool write_data(uint16_t datawriter_id, uint8_t* buf, size_t len) override;
    bool write_data(uint16_t datawriter_id, std::vector<uint8_t>& data) override;
    bool read_data(uint16_t datareader_id) override; // TODO (julian).

private:
    void register_topic(const std::string& topic_name, uint16_t topic_id);
    void unregister_topic(const std::string& topic_name);
    bool check_register_topic(const std::string& topic_name, uint16_t& topic_id);

private:
    std::unordered_map<uint16_t, fastrtps::Participant*> participants_;
    std::unordered_map<uint16_t, TopicPubSubType*> topics_;
    std::unordered_map<uint16_t, FakePublisher> publishers_;
    std::unordered_map<uint16_t, FakeSubscriber> subscribers_;
    std::unordered_map<uint16_t, fastrtps::Publisher*> datawriters_;
    std::unordered_map<uint16_t, fastrtps::Subscriber*> datareaders_;

    std::unordered_map<std::string, uint16_t> registered_topics_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_FAST_MIDDLEWARE_HPP_
