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

#ifndef UXR_AGENT_MIDDLEWARE_MIDDLEWARE_HPP_
#define UXR_AGENT_MIDDLEWARE_MIDDLEWARE_HPP_

#include <uxr/agent/config.hpp>

#include <string>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <functional>
#include <chrono>

namespace eprosima {
namespace uxr {

class Middleware
{
public:
    enum class Kind : uint8_t
    {
        NONE,
    #ifdef UAGENT_FAST_PROFILE
        FAST,
    #endif

    #ifdef UAGENT_CED_PROFILE
        CED,
    #endif
    };

    Middleware() = default;
    virtual ~Middleware() = default;

/**********************************************************************************************************************
 * Create functions.
 **********************************************************************************************************************/
    virtual bool create_participant_by_ref(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& ref) = 0;

    virtual bool create_participant_by_xml(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& xml) = 0;

    virtual bool create_topic_by_ref(
            uint16_t topic_id,
            uint16_t participant_id,
            const std::string& ref) = 0;

    virtual bool create_topic_by_xml(
            uint16_t topic_id,
            uint16_t participant_id,
            const std::string& xml) = 0;

    virtual bool create_publisher_by_xml(
            uint16_t publisher_id,
            uint16_t participant_id,
            const std::string& xml) = 0;

    virtual bool create_subscriber_by_xml(
            uint16_t subscriber_id,
            uint16_t participant_id,
            const std::string& xml) = 0;

    virtual bool create_datawriter_by_ref(
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const std::string& ref,
            uint16_t& associated_topic_id) = 0;

    virtual bool create_datawriter_by_xml(
            uint16_t datawriter_id,
            uint16_t publisher_id,
            const std::string& xml,
            uint16_t& associated_topic_id) = 0;

    virtual bool create_datareader_by_ref(
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const std::string& ref,
            uint16_t& associated_topic_id) = 0;

    virtual bool create_datareader_by_xml(
            uint16_t datareader_id,
            uint16_t subscriber_id,
            const std::string& xml,
            uint16_t& associated_topic_id) = 0;

    virtual bool create_requester_by_ref(
            uint16_t requester_id,
            uint16_t participant_id,
            const std::string& ref) = 0;

    virtual bool create_requester_by_xml(
            uint16_t requester_id,
            uint16_t participant_id,
            const std::string& xml) = 0;

    virtual bool create_replier_by_ref(
            uint16_t replier_id,
            uint16_t participant_id,
            const std::string& ref) = 0;

    virtual bool create_replier_by_xml(
            uint16_t replier_id,
            uint16_t participant_id,
            const std::string& xml) = 0;

/**********************************************************************************************************************
 * Delete functions.
 **********************************************************************************************************************/
    virtual bool delete_participant(uint16_t participant_id) = 0;

    virtual bool delete_topic(uint16_t topic_id) = 0;

    virtual bool delete_publisher(uint16_t publisher_id) = 0;

    virtual bool delete_subscriber(uint16_t subscriber_id) = 0;

    virtual bool delete_datawriter(uint16_t datawriter_id) = 0;

    virtual bool delete_datareader(uint16_t datareader_id) = 0;

    virtual bool delete_requester(uint16_t requester_id) = 0;

    virtual bool delete_replier(uint16_t replier_id) = 0;

/**********************************************************************************************************************
 * Write/Read functions.
 **********************************************************************************************************************/
    virtual bool write_data(
            uint16_t datawriter_id,
            const std::vector<uint8_t>& data) = 0;

    virtual bool write_request(
            uint16_t requester_id,
            const std::vector<uint8_t>& data) = 0;

    virtual bool write_reply(
            uint16_t replier_id,
            const std::vector<uint8_t>& data) = 0;

    virtual bool read_data(
            uint16_t datareader_id,
            std::vector<uint8_t>& data,
            std::chrono::milliseconds timeout) = 0;

/**********************************************************************************************************************
 * Matched functions.
 **********************************************************************************************************************/
    virtual bool matched_participant_from_ref(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& ref) const = 0;

    virtual bool matched_participant_from_xml(
            uint16_t participant_id,
            int16_t domain_id,
            const std::string& xml) const = 0;

    virtual bool matched_topic_from_ref(
            uint16_t topic_id,
            const std::string& ref) const = 0;

    virtual bool matched_topic_from_xml(
            uint16_t topic_id,
            const std::string& xml) const = 0;

    virtual bool matched_datawriter_from_ref(
            uint16_t datawriter_id,
            const std::string& ref) const = 0;

    virtual bool matched_datawriter_from_xml(
            uint16_t datawriter_id,
            const std::string& xml) const = 0;

    virtual bool matched_datareader_from_ref(
            uint16_t datareader_id,
            const std::string& ref) const = 0;

    virtual bool matched_datareader_from_xml(
            uint16_t datareader_id,
            const std::string& xml) const = 0;

    virtual bool matched_requester_from_ref(
            uint16_t requester_id,
            const std::string& ref) const = 0;

    virtual bool matched_requester_from_xml(
            uint16_t requester_id,
            const std::string& xml) const = 0;

    virtual bool matched_replier_from_ref(
            uint16_t replier_id,
            const std::string& ref) const = 0;

    virtual bool matched_replier_from_xml(
            uint16_t replier_id,
            const std::string& xml) const = 0;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MIDDLEWARE_MIDDLEWARE_HPP_
