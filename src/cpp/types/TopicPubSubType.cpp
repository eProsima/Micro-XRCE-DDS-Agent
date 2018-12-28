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

#include <uxr/agent/types/TopicPubSubType.hpp>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

namespace eprosima {
namespace uxr {

TopicPubSubType::TopicPubSubType(bool with_key) {
    m_typeSize = 1024 + 4 /*encapsulation*/;
    m_isGetKeyDefined = with_key;
}

bool TopicPubSubType::serialize(void *data, rtps::SerializedPayload_t *payload)
{
    bool rv = false;
    std::vector<unsigned char>* buffer = reinterpret_cast<std::vector<unsigned char>*>(data);
    payload->data[0] = 0;
    payload->data[1] = 1;
    payload->data[2] = 0;
    payload->data[3] = 0;
    if (buffer->size() <= (payload->max_size - 4))
    {
        memcpy(&payload->data[4], buffer->data(), buffer->size());
        payload->length = uint32_t(buffer->size() + 4); //Get the serialized length
        rv = true;
    }
    return rv;
}

bool TopicPubSubType::deserialize(rtps::SerializedPayload_t* payload, void* data)
{
    std::vector<unsigned char>* buffer = reinterpret_cast<std::vector<unsigned char>*>(data);
    buffer->assign(payload->data + 4, payload->data + payload->length);

    return true;
}

std::function<uint32_t()> TopicPubSubType::getSerializedSizeProvider(void* data) {
    return [data]() -> uint32_t
    {
        return (uint32_t)reinterpret_cast<std::vector<unsigned char>*>(data)->size() + 4 /*encapsulation*/;
    };
}

void* TopicPubSubType::createData() {
    return (void*)new std::vector<unsigned char>;
}

void TopicPubSubType::deleteData(void* data) {
    delete((std::vector<unsigned char>*)data);
}

bool TopicPubSubType::getKey(void *data, rtps::InstanceHandle_t* handle, bool force_md5)
{
    // TODO.
    (void) data;
    (void) handle;
    (void) force_md5;
    return m_isGetKeyDefined;
}

} // namespace uxr
} // namespace eprosima
