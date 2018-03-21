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

#include "XRCETypes.h"
#include "MessageHeader.h"
#include "SubMessageHeader.h"

#include <iostream>

namespace eprosima {
namespace micrortps {

Serializer::Serializer(char *buffer, size_t buffer_size) : fastbuffer_(buffer, buffer_size), serializer_(fastbuffer_)
{
}

template <class T> bool Serializer::serialize(const T& data)
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

template bool Serializer::serialize(const dds::xrce::MessageHeader& data);
template bool Serializer::serialize(const dds::xrce::SubmessageHeader& data);
template bool Serializer::serialize(const dds::xrce::STATUS_Payload& data);
template bool Serializer::serialize(const dds::xrce::DATA_Payload_Data& data);
template bool Serializer::serialize(const dds::xrce::DATA_Payload_DataSeq& data);
template bool Serializer::serialize(const dds::xrce::DATA_Payload_Sample& data);
template bool Serializer::serialize(const dds::xrce::DATA_Payload_SampleSeq& data);
template bool Serializer::serialize(const dds::xrce::DATA_Payload_PackedSamples& data);
template bool Serializer::serialize(const dds::xrce::BaseObjectReply& data);
template bool Serializer::serialize(const dds::xrce::BaseObjectRequest& data);
template bool Serializer::serialize(const dds::xrce::ResultStatus& data);

// TODO(Borja) No deberia poder serializar. Estan instanciados para tests.
template bool Serializer::serialize(const dds::xrce::READ_DATA_Payload& data);
template bool Serializer::serialize(const dds::xrce::WRITE_DATA_Payload_Data& data);
template bool Serializer::serialize(const dds::xrce::CREATE_Payload& data);
template bool Serializer::serialize(const dds::xrce::CREATE_CLIENT_Payload& data);
template bool Serializer::serialize(const dds::xrce::DELETE_Payload& data);
template bool Serializer::serialize(const dds::xrce::HEARTBEAT_Payload& data);
template bool Serializer::serialize(const dds::xrce::ACKNACK_Payload& data);

template <class T> bool Serializer::deserialize(T& data)
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

template bool Serializer::deserialize(dds::xrce::MessageHeader& data);
template bool Serializer::deserialize(dds::xrce::SubmessageHeader& data);
template bool Serializer::deserialize(dds::xrce::CREATE_Payload& data);
template bool Serializer::deserialize(dds::xrce::CREATE_CLIENT_Payload& data);
template bool Serializer::deserialize(dds::xrce::DELETE_Payload& data);
template bool Serializer::deserialize(dds::xrce::STATUS_Payload& data);
template bool Serializer::deserialize(dds::xrce::READ_DATA_Payload& data);
template bool Serializer::deserialize(dds::xrce::WRITE_DATA_Payload_Data& data);
template bool Serializer::deserialize(dds::xrce::HEARTBEAT_Payload& data);
template bool Serializer::deserialize(dds::xrce::ACKNACK_Payload& data);
template bool Serializer::deserialize(dds::xrce::BaseObjectRequest& data);

/* TODO (Borja): externalize templates, they are only useful for testing. */
template bool Serializer::deserialize(dds::xrce::DATA_Payload_Data& data);
template bool Serializer::deserialize(dds::xrce::DATA_Payload_DataSeq& data);
template bool Serializer::deserialize(dds::xrce::DATA_Payload_Sample& data);
template bool Serializer::deserialize(dds::xrce::DATA_Payload_SampleSeq& data);
template bool Serializer::deserialize(dds::xrce::DATA_Payload_PackedSamples& data);

char* Serializer::get_current_position()
{
    return serializer_.getCurrentPosition();
}

size_t Serializer::get_remainder_size()
{
    return fastbuffer_.getBufferSize() - serializer_.getSerializedDataLength();
}

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
