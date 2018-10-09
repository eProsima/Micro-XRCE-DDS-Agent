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
    virtual ~TopicPubSubType() override = default;
    bool serialize(void *data, rtps::SerializedPayload_t *payload);
    bool deserialize(rtps::SerializedPayload_t *payload, void *data);
    std::function<uint32_t()> getSerializedSizeProvider(void* data);
    bool getKey(void *data, rtps::InstanceHandle_t *ihandle);
    void* createData();
    void deleteData(void * data);
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_TYPES_TOPICPUBSUBTYPES_HPP_
