// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _UXR_AGENT_TYPES_TOPICPUBSUBTYPES_HPP_
#define _UXR_AGENT_TYPES_TOPICPUBSUBTYPES_HPP_

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <vector>

namespace eprosima {
namespace uxr {

class TopicPubSubType: public fastdds::dds::TopicDataType
{
public:
    typedef std::vector<unsigned char> type;

    explicit TopicPubSubType(bool with_key);
    ~TopicPubSubType() override = default;
    bool serialize(const void* const data, fastdds::rtps::SerializedPayload_t& payload, fastdds::dds::DataRepresentationId_t data_representation) override;
    bool deserialize(fastdds::rtps::SerializedPayload_t& payload, void* data) override;

    uint32_t calculate_serialized_size(
        const void* const data,
        eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    void* create_data() override;

    void delete_data(
                void* data) override;

    bool compute_key(
                fastdds::rtps::SerializedPayload_t& payload,
                fastdds::rtps::InstanceHandle_t& ihandle,
                bool force_md5) override;

    bool compute_key(
            const void* const data,
            fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5) override;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TYPES_TOPICPUBSUBTYPES_HPP_
