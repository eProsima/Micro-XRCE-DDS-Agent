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
    max_serialized_type_size = 1024 + 4 /*encapsulation*/;
    is_compute_key_provided = with_key;
}

bool TopicPubSubType::serialize(const void* const data, fastdds::rtps::SerializedPayload_t &payload, fastdds::dds::DataRepresentationId_t /* data_representation */)
{
    bool rv = false;

    // Reinterpret cast back to a std::vector from data pointer
    void * non_const_data = const_cast<void*>(data);
    std::vector<unsigned char>* buffer = reinterpret_cast<std::vector<unsigned char>*>(non_const_data);

    // Representation header
    payload.data[0] = 0;
    payload.data[1] = 1;
    payload.data[2] = 0;
    payload.data[3] = 0;

    if (buffer->size() <= (payload.max_size - 4))
    {
        memcpy(&payload.data[4], buffer->data(), buffer->size());
        payload.length = uint32_t(buffer->size() + 4); //Get the serialized length
        rv = true;
    }

    return rv;
}

bool TopicPubSubType::deserialize(fastdds::rtps::SerializedPayload_t& payload, void* data)
{
    std::vector<unsigned char>* buffer = reinterpret_cast<std::vector<unsigned char>*>(data);
    buffer->assign(payload.data + 4, payload.data + payload.length);

    return true;
}

uint32_t TopicPubSubType::calculate_serialized_size(
    const void* const data,
    eprosima::fastdds::dds::DataRepresentationId_t /* data_representation */)
{
    void * non_const_data = const_cast<void*>(data);
    std::vector<unsigned char>* buffer = reinterpret_cast<std::vector<unsigned char>*>(non_const_data);

    return static_cast<uint32_t>(buffer->size() + 4); /*encapsulation*/
}

void* TopicPubSubType::create_data()
{
    return (void*)new std::vector<unsigned char>;
}

void TopicPubSubType::delete_data(
    void* data)
{
    delete((std::vector<unsigned char>*)data);
}

bool TopicPubSubType::compute_key(
    fastdds::rtps::SerializedPayload_t& /* payload */,
    fastdds::rtps::InstanceHandle_t& /* ihandle */,
    bool /* force_md5 */)
{
    // TODO(pgarrido): Implement this function.
    return false;
}

bool TopicPubSubType::compute_key(
    const void* const /* data */,
    fastdds::rtps::InstanceHandle_t& /* ihandle */,
    bool /* force_md5 */)
{
    // TODO(pgarrido): Implement this function.
    return false;
}

} // namespace uxr
} // namespace eprosima
