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

#include "agent/MessageHeader.h"
#include "agent/SubMessageHeader.h"
#include "agent/Payloads.h"

Serializer::Serializer(char* const buffer_, size_t buffer_size) :
    fastbuffer_(buffer_, buffer_size),
    serializer_(fastbuffer_, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR)	
{
}

template<class T>
bool Serializer::serialize(const T& data)
{
    try
    {
        data.serialize(serializer_); // Serialize the object:
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }
    return true;
}

template bool Serializer::serialize(const MessageHeader& data);
template bool Serializer::serialize(const SubmessageHeader& data);
template bool Serializer::serialize(const CREATE_PAYLOAD& data);
template bool Serializer::serialize(const DELETE_PAYLOAD& data);
template bool Serializer::serialize(const RESOURCE_STATUS_PAYLOAD& data);
template bool Serializer::serialize(const DATA_PAYLOAD& data);
template bool Serializer::serialize(const READ_DATA_PAYLOAD& data);
template bool Serializer::serialize(const WRITE_DATA_PAYLOAD& data);

template<class T> bool Serializer::deserialize(T& data)
{
    try
    {
        data.deserialize(serializer_); //Deserialize the object:
    }
    catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
    {
        return false;
    }
    return true;
}

template bool Serializer::deserialize(MessageHeader& data);
template bool Serializer::deserialize(SubmessageHeader& data);
template bool Serializer::deserialize(CREATE_PAYLOAD& data);
template bool Serializer::deserialize(DELETE_PAYLOAD& data);
template bool Serializer::deserialize(RESOURCE_STATUS_PAYLOAD& data);
template bool Serializer::deserialize(DATA_PAYLOAD& data);
template bool Serializer::deserialize(READ_DATA_PAYLOAD& data);
template bool Serializer::deserialize(WRITE_DATA_PAYLOAD& data);


size_t Serializer::get_serialized_size()
{
    return serializer_.getSerializedDataLength();
}

bool Serializer::bufferEnd()
{
    return fastbuffer_.getBufferSize() == serializer_.getSerializedDataLength();
}

void Serializer::align()
{

}