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


#ifndef UXR_AGENT_MIDDLEWARE_FAST_FAST_MIDDLEWARE_HPP_
#define UXR_AGENT_MIDDLEWARE_FAST_FAST_MIDDLEWARE_HPP_

#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/middleware/fast/FastEntities.hpp>
#include <uxr/agent/types/TopicPubSubType.hpp>

#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <memory>

namespace eprosima {
namespace uxr {
namespace middleware {
class CallbackFactory;
} // namespace middleware

class FastMiddleware : public Middleware
{
public:
    FastMiddleware();
    ~FastMiddleware() final = default;

/**********************************************************************************************************************
 * Create functions.
 **********************************************************************************************************************/
    bool create_participant_by_ref(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& ref) override;

    bool create_participant_by_xml(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& xml) override;

    bool create_participant_by_bin(
            uint16_t /* participant_id */,
            const dds::xrce::OBJK_DomainParticipant_Binary& /* participant_xrce */) override { return false; };

    bool create_topic_by_ref(
            uint16_t topic_id,
            uint16_t participant_id,
            const std::string& ref) override;

    bool create_topic_by_xml(
            uint16_t topic_id,
            uint16_t participant_id,
            const std::string& xml) override;

    bool create_topic_by_bin(
            uint16_t /* topic_id */,
            uint16_t /* participant_id */,
            const dds::xrce::OBJK_Topic_Binary& /* topic_xrce */) override { return false; };

    bool create_publisher_by_xml(
            uint16_t publisher_id,
            uint16_t participant_id,
            const std::string&) override;

    bool create_publisher_by_bin(
            uint16_t /* publisher_id */,
            uint16_t /* participant_id */,
            const dds::xrce::OBJK_Publisher_Binary& /* publisher_xrce */) override { return false; };

    bool create_subscriber_by_xml(
            uint16_t subscirber_id,
            uint16_t participant_id,
            const std::string&) override;

    bool create_subscriber_by_bin(
            uint16_t /* subscriber_id */,
            uint16_t /* participant_id */,
            const dds::xrce::OBJK_Subscriber_Binary& /* subscriber_xrce */) override { return false; };
            
    bool create_datawriter_by_ref(
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const std::string& ref) override;

    bool create_datawriter_by_xml(
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const std::string& xml) override;

    bool create_datawriter_by_bin(
            uint16_t /* datawriter_id */,
            uint16_t /* publisher_id */,
            const dds::xrce::OBJK_DataWriter_Binary& /* datawriter_xrce */) override { return false; };

    bool create_datareader_by_ref(
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const std::string& ref) override;

    bool create_datareader_by_xml(
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const std::string& xml) override;

    bool create_datareader_by_bin(
            uint16_t /* datareader_id */,
            uint16_t /* subscriber_id */,
            const dds::xrce::OBJK_DataReader_Binary& /* datareader_xrce */) override { return false; };
            
    bool create_requester_by_ref(
            uint16_t requester_id,
            uint16_t participant_id,
            const std::string& ref) override;

    bool create_requester_by_xml(
            uint16_t requester_id,
            uint16_t participant_id,
            const std::string& xml) override;

    bool create_requester_by_bin(
            uint16_t /* requester_id */,
            uint16_t /* participant_id */,
            const dds::xrce::OBJK_Requester_Binary& /* requester_xrce */) override { return false; };

    bool create_replier_by_ref(
            uint16_t replier_id,
            uint16_t participant_id,
            const std::string& ref) override;

    bool create_replier_by_xml(
            uint16_t replier_id,
            uint16_t participant_id,
            const std::string& xml) override;

    bool create_replier_by_bin(
            uint16_t /* replier_id */,
            uint16_t /* participant_id */,
            const dds::xrce::OBJK_Replier_Binary& /* replier_xrce */) override { return false; };

/**********************************************************************************************************************
 * Delete functions.
 **********************************************************************************************************************/
    bool delete_participant(uint16_t participant_id) override;

    bool delete_topic(uint16_t topic_id) override;

    bool delete_publisher(uint16_t publisher_id) override;

    bool delete_subscriber(uint16_t subscriber_id) override;

    bool delete_datawriter(uint16_t datawriter_id) override;

    bool delete_datareader(uint16_t datareader_id) override;

    bool delete_requester(uint16_t requester_id) override;

    bool delete_replier(uint16_t replier_id) override;

/**********************************************************************************************************************
 * Write/Read functions.
 **********************************************************************************************************************/
    bool write_data(
            uint16_t datawriter_id,
            const std::vector<uint8_t>& data) override;

    bool write_request(
            uint16_t requester_id,
            uint32_t sequence_number,
            const std::vector<uint8_t>& data) override;

    bool write_reply(
            uint16_t replier_id,
            const std::vector<uint8_t>& data) override;

    bool read_data(
            uint16_t datareader_id,
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout) override;

    bool read_request(
            uint16_t replier_id,
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout) override;

    bool read_reply(
            uint16_t reply_id,
            uint32_t& sequence_number,
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout) override;

/**********************************************************************************************************************
 * Matched functions.
 **********************************************************************************************************************/
    bool matched_participant_from_ref(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& ref) const override;

    bool matched_participant_from_xml(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& xml) const override;

    bool matched_participant_from_bin(
            uint16_t /* participant_id */,
            int16_t /* domain_id */,
            const dds::xrce::OBJK_DomainParticipant_Binary& /* participant_xrce */) const override { return false; };

    bool matched_topic_from_ref(
            uint16_t topic_id,
            const std::string& ref) const override;

    bool matched_topic_from_xml(
            uint16_t topic_id,
            const std::string& xml) const override;

    bool matched_topic_from_bin(
            uint16_t /* topic_id */,
            const dds::xrce::OBJK_Topic_Binary& /* topic_xrce */) const override { return false; };

    bool matched_datawriter_from_ref(
            uint16_t datawriter_id,
            const std::string& ref) const override;

    bool matched_datawriter_from_xml(
            uint16_t datawriter_id,
            const std::string& xml) const override;

    bool matched_datawriter_from_bin(
            uint16_t /* datawriter_id */,
            const dds::xrce::OBJK_DataWriter_Binary& /* datawriter_xrce */) const override { return false; };

    bool matched_datareader_from_ref(
            uint16_t datareader_id,
            const std::string& ref) const override;

    bool matched_datareader_from_xml(
            uint16_t datareader_id,
            const std::string& xml) const override;

    bool matched_datareader_from_bin(
            uint16_t /* datareader_id */,
            const dds::xrce::OBJK_DataReader_Binary& /* datareader_xrce */) const override { return false; };

    bool matched_requester_from_ref(
            uint16_t participant_id,
            const std::string& ref) const override;

    bool matched_requester_from_xml(
            uint16_t participant_id,
            const std::string& xml) const override;

    bool matched_requester_from_bin(
            uint16_t /* requester_id */,
            const dds::xrce::OBJK_Requester_Binary& /* requester_xrce */) const override { return false; };

    bool matched_replier_from_ref(
            uint16_t participant_id,
            const std::string& ref) const override;

    bool matched_replier_from_xml(
            uint16_t participant_id,
            const std::string& xml) const override;

    bool matched_replier_from_bin(
            uint16_t /* replier_id */,
            const dds::xrce::OBJK_Replier_Binary& /* replier_xrce */) const override { return false; };
private:
    FastListener listener_;
    std::unordered_map<uint16_t, std::shared_ptr<FastParticipant>> participants_;
    std::unordered_map<uint16_t, std::shared_ptr<FastTopic>> topics_;
    std::unordered_map<uint16_t, std::shared_ptr<FastPublisher>> publishers_;
    std::unordered_map<uint16_t, std::shared_ptr<FastSubscriber>> subscribers_;
    std::unordered_map<uint16_t, std::shared_ptr<FastDataWriter>> datawriters_;
    std::unordered_map<uint16_t, std::shared_ptr<FastDataReader>> datareaders_;
    std::unordered_map<uint16_t, std::shared_ptr<FastRequester>> requesters_;
    std::unordered_map<uint16_t, std::shared_ptr<FastReplier>> repliers_;

    middleware::CallbackFactory& callback_factory_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_FAST_FAST_MIDDLEWARE_HPP_
