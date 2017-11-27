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

#include "agent/Serializer.h"

#include "MessageHeader.h"
#include "Payloads.h"
#include "SubMessageHeader.h"

#include <iostream>

namespace eprosima {
namespace micrortps {

Serializer::Serializer(char *buffer, size_t buffer_size) : fastbuffer_(buffer, buffer_size), serializer_(fastbuffer_)
{
}

template <class T> bool Serializer::serialize(const T &data)
{
    try
    {
        data.serialize(serializer_); // Serialize the object:
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException & /*exception*/)
    {
        std::cout << "serialize eprosima::fastcdr::exception::NotEnoughMemoryException" << std::endl;
        return false;
    }
    return true;
}

template bool Serializer::serialize(const MessageHeader &data);
template bool Serializer::serialize(const SubmessageHeader &data);
template bool Serializer::serialize(const RESOURCE_STATUS_Payload &data);
template bool Serializer::serialize(const DATA_Payload_Data &data);
template bool Serializer::serialize(const DATA_Payload_DataSeq &data);
template bool Serializer::serialize(const DATA_Payload_Sample &data);
template bool Serializer::serialize(const DATA_Payload_SampleSeq &data);
template bool Serializer::serialize(const DATA_Payload_PackedSamples &data);
template bool Serializer::serialize(const BaseObjectReply &data);
template bool Serializer::serialize(const ResultStatus &data);

// TODO(borja) No deberia poder serializar. Estan instanciados para tests.
template bool Serializer::serialize(const READ_DATA_Payload &data);
template bool Serializer::serialize(const WRITE_DATA_Payload &data);
template bool Serializer::serialize(const CREATE_Payload &data);
template bool Serializer::serialize(const CREATE_CLIENT_Payload &data);
template bool Serializer::serialize(const DELETE_RESOURCE_Payload &data);

template <class T> bool Serializer::deserialize(T &data)
{
    try
    {
        data.deserialize(serializer_); // Deserialize the object:
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException & /*exception*/)
    {
        std::cout << "deserialize eprosima::fastcdr::exception::NotEnoughMemoryException" << std::endl;
        return false;
    }
    return true;
}

template bool Serializer::deserialize(MessageHeader &data);
template bool Serializer::deserialize(SubmessageHeader &data);
template bool Serializer::deserialize(CREATE_Payload &data);
template bool Serializer::deserialize(CREATE_CLIENT_Payload &data);
template bool Serializer::deserialize(DELETE_RESOURCE_Payload &data);
template bool Serializer::deserialize(RESOURCE_STATUS_Payload &data);
template bool Serializer::deserialize(READ_DATA_Payload &data);
template bool Serializer::deserialize(WRITE_DATA_Payload &data);
// TODO(borja) No deberia poder serializar. Estan instanciados para tests.
template bool Serializer::deserialize(DATA_Payload_Data &data);
template bool Serializer::deserialize(DATA_Payload_DataSeq &data);
template bool Serializer::deserialize(DATA_Payload_Sample &data);
template bool Serializer::deserialize(DATA_Payload_SampleSeq &data);
template bool Serializer::deserialize(DATA_Payload_PackedSamples &data);

size_t Serializer::get_serialized_size()
{
    return serializer_.getSerializedDataLength();
}

bool Serializer::bufferEnd()
{
    return fastbuffer_.getBufferSize() == serializer_.getSerializedDataLength();
}

void Serializer::force_new_submessage_align()
{
    size_t align = (4 - ((serializer_.getCurrentPosition() - serializer_.getBufferPointer()) % 4)) & 3;
    serializer_.jump(align);
}

} /* namespace micrortps */
} /* namespace eprosima */
