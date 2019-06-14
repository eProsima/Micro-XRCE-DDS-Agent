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

#include <fastrtps/TopicDataType.h>

#include <vector>

using namespace eprosima::fastrtps;
namespace eprosima {
namespace uxr {

class TopicPubSubType: public TopicDataType
{
public:
    typedef std::vector<unsigned char> type;

    explicit TopicPubSubType(bool with_key);
    ~TopicPubSubType() override = default;
    bool serialize(void* data, rtps::SerializedPayload_t* payload) override;
    bool deserialize(rtps::SerializedPayload_t* payload, void* data) override;
    std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
    bool getKey(void* data, rtps::InstanceHandle_t* ihandle, bool force_md5 = false) override;
    void* createData() override;
    void deleteData(void* data) override;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TYPES_TOPICPUBSUBTYPES_HPP_
