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

#ifndef _MICRORTPS_AGENT_MESSAGE_INPUT_MESSAGE_HPP_
#define _MICRORTPS_AGENT_MESSAGE_INPUT_MESSAGE_HPP_

#include <micrortps/agent/types/MessageHeader.hpp>
#include <micrortps/agent/types/SubMessageHeader.hpp>
#include <fastcdr/Cdr.h>
#include <fastcdr/exceptions/Exception.h>
#include <iostream>

namespace eprosima {
namespace micrortps {

class InputMessage
{
public:
    InputMessage(uint8_t* buf, size_t len)
        : buf_(new uint8_t[len]),
          len_(len),
          header_(),
          subheader_(),
          fastbuffer_((char*)(buf_), len_),
          deserializer_(fastbuffer_)
    {
        memcpy(buf_, buf, len);
        deserialize(header_);
    }

    ~InputMessage()
    {
        delete[] buf_;
    }

    const dds::xrce::MessageHeader& get_header() const { return header_; }
    const dds::xrce::SubmessageHeader& get_subheader() const { return subheader_; }
    template<class T> bool get_payload(T& data);
    bool prepare_next_submessage();

private:
    template<class T> bool deserialize(T& data);

private:
    uint8_t* buf_;
    size_t len_;
    dds::xrce::MessageHeader header_;
    dds::xrce::SubmessageHeader subheader_;
    fastcdr::FastBuffer fastbuffer_;
    fastcdr::Cdr deserializer_;
};

inline bool InputMessage::prepare_next_submessage()
{
    bool rv = false;
    deserializer_.jump((4 - ((deserializer_.getCurrentPosition() - deserializer_.getBufferPointer()) & 3)) & 3);
    if (fastbuffer_.getBufferSize() > deserializer_.getSerializedDataLength())
    {
        rv = deserialize(subheader_);
    }
    return rv;
}

template<class T>
inline bool InputMessage::get_payload(T& data)
{
    bool rv = true;
    try
    {
        data.deserialize(deserializer_);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException & /*exception*/)
    {
        std::cout << "deserialize eprosima::fastcdr::exception::NotEnoughMemoryException" << std::endl;
        rv = false;
    }
    return rv;
}

template bool InputMessage::get_payload(dds::xrce::CREATE_CLIENT_Payload& data);
template bool InputMessage::get_payload(dds::xrce::CREATE_Payload& data);
template bool InputMessage::get_payload(dds::xrce::DELETE_Payload& data);
template bool InputMessage::get_payload(dds::xrce::READ_DATA_Payload& data);
template bool InputMessage::get_payload(dds::xrce::WRITE_DATA_Payload_Data& data);
template bool InputMessage::get_payload(dds::xrce::HEARTBEAT_Payload& data);
template bool InputMessage::get_payload(dds::xrce::ACKNACK_Payload& data);

template<class T>
inline bool InputMessage::deserialize(T& data)
{
    bool rv = true;
    try
    {
        data.deserialize(deserializer_);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException & /*exception*/)
    {
        std::cout << "deserialize eprosima::fastcdr::exception::NotEnoughMemoryException" << std::endl;
        rv = false;
    }
    return rv;
}

template bool InputMessage::deserialize(dds::xrce::MessageHeader& data);
template bool InputMessage::deserialize(dds::xrce::SubmessageHeader& data);

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_MESSAGE_INPUT_MESSAGE_HPP_
