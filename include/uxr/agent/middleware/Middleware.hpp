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

#ifndef _UXR_AGENT_MIDDLEWARE_HPP_
#define _UXR_AGENT_MIDDLEWARE_HPP_

#include <string>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <functional>

namespace eprosima {
namespace uxr {

typedef std::function<void ()> OnNewData;

class Middleware
{
public:
    Middleware() = default;
    virtual ~Middleware() = default;

    /* Creation functions. */
    virtual bool create_participant_from_ref(uint16_t participant_id, const std::string& ref) = 0;
    virtual bool create_participant_from_xml(uint16_t participant_id, const std::string& xml) = 0;

    virtual bool create_topic_from_ref(uint16_t topic_id, uint16_t participant_id, const std::string& ref) = 0;
    virtual bool create_topic_from_xml(uint16_t topic_id, uint16_t participant_id, const std::string& xml) = 0;

    virtual bool create_publisher_from_xml(uint16_t publisher_id, uint16_t participant_id, const std::string& xml) = 0;

    virtual bool create_subscriber_from_xml(uint16_t subscriber_id, uint16_t participant_id, const std::string& xml) = 0;

    virtual bool create_datawriter_from_ref(uint16_t datawriter_id,
                                            uint16_t publisher_id,
                                            const std::string& ref,
                                            uint16_t& associated_topic_id) = 0;

    virtual bool create_datawriter_from_xml(uint16_t datawriter_id,
                                            uint16_t publisher_id,
                                            const std::string& xml,
                                            uint16_t& associated_topic_id) = 0;

    virtual bool create_datareader_from_ref(uint16_t datareader_id,
                                            uint16_t subscriber_id,
                                            const std::string& ref,
                                            uint16_t& associated_topic_id,
                                            OnNewData on_new_data_cb) = 0;

    virtual bool create_datareader_from_xml(uint16_t datareader_id,
                                            uint16_t subscriber_id,
                                            const std::string& xml,
                                            uint16_t& associated_topic_id,
                                            OnNewData on_new_data_cb) = 0;

    /* Deletion functions. */
    virtual bool delete_participant(uint16_t participant_id) = 0;
    virtual bool delete_topic(uint16_t topic_id, uint16_t participant_id) = 0;
    virtual bool delete_publisher(uint16_t publisher_id, uint16_t participant_id) = 0;
    virtual bool delete_subscriber(uint16_t subscriber_id, uint16_t participant_id) = 0;
    virtual bool delete_datawriter(uint16_t datawriter_id, uint16_t publisher_id) = 0;
    virtual bool delete_datareader(uint16_t datareader_id, uint16_t subscriber_id) = 0;

    /* Write and read functions. */
    virtual bool write_data(uint16_t datawriter_id, std::vector<uint8_t>& data) = 0;
    virtual bool read_data(uint16_t datareader_id, std::vector<uint8_t>& data) = 0;

    /* Matching functions. */
    virtual bool matched_participant_from_ref(uint16_t participant_id, const std::string& ref) = 0;
    virtual bool matched_participant_from_xml(uint16_t participant_id, const std::string& xml) = 0;

    virtual bool matched_topic_from_ref(uint16_t topic_id, const std::string& ref) = 0;
    virtual bool matched_topic_from_xml(uint16_t topic_id, const std::string& xml) = 0;

    virtual bool matched_datawriter_from_ref(uint16_t datawriter_id, const std::string& ref) = 0;
    virtual bool matched_datawriter_from_xml(uint16_t datawriter_id, const std::string& xml) = 0;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_MIDDLEWARE_HPP_
