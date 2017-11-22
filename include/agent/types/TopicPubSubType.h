// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#ifndef _PUBSUBTYPES_H_
#define _PUBSUBTYPES_H_

#include <fastrtps/TopicDataType.h>

#include <vector>

using namespace eprosima::fastrtps;

/*!
 * @brief This class represents the TopicDataType for micrortps. It handles data as raw buffer.
 */
class TopicPubSubType: public TopicDataType
{
public:
    typedef std::vector<unsigned char> type;

    explicit TopicPubSubType(bool with_key);
    virtual ~TopicPubSubType() override = default;
    bool serialize(void *data, SerializedPayload_t *payload);
    bool deserialize(SerializedPayload_t *payload, void *data);
    std::function<uint32_t()> getSerializedSizeProvider(void* data);
    bool getKey(void *data, InstanceHandle_t *ihandle);
    void* createData();
    void deleteData(void * data);
};

#endif // _PUBSUBTYPE_H_
