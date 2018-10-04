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

#ifndef _UXR_AGENT_MESSAGE_OUTPUT_MESSAGE_HPP_
#define _UXR_AGENT_MESSAGE_OUTPUT_MESSAGE_HPP_

#include <uxr/agent/types/MessageHeader.hpp>
#include <uxr/agent/types/SubMessageHeader.hpp>
#include <fastcdr/Cdr.h>
#include <fastcdr/exceptions/Exception.h>
#include <iostream>

// TODO (julian): get MTU from transport.
const size_t mtu_size = 512;

namespace eprosima {
namespace uxr {

class OutputMessage
{
public:
    OutputMessage(const dds::xrce::MessageHeader& header)
        : buf_{},
          fastbuffer_(reinterpret_cast<char*>(buf_.data()), buf_.size()),
          serializer_(fastbuffer_)
    {
        serialize(header);
    }

    uint8_t* get_buf() { return buf_.data(); }
    size_t get_len() { return serializer_.getSerializedDataLength(); }
    template<class T>
    bool append_submessage(dds::xrce::SubmessageId submessage_id, const T& data, uint8_t flags = 0x01);

private:
    bool append_subheader(dds::xrce::SubmessageId submessage_id, uint8_t flags, size_t submessage_len);
    template<class T> bool serialize(const T& data);

private:
    std::array<uint8_t, mtu_size> buf_;
    fastcdr::FastBuffer fastbuffer_;
    fastcdr::Cdr serializer_;
};

template<class T>
inline bool OutputMessage::append_submessage(dds::xrce::SubmessageId submessage_id, const T& data, uint8_t flags)
{
    bool rv = false;
    if (append_subheader(submessage_id, flags, data.getCdrSerializedSize()))
    {
        rv = serialize(data);
    }
    return rv;
}

inline bool OutputMessage::append_subheader(dds::xrce::SubmessageId submessage_id, uint8_t flags, size_t submessage_len)
{
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(submessage_id);
    subheader.flags(flags);
    subheader.submessage_length(static_cast<uint16_t>(submessage_len));

    serializer_.jump((4 - ((serializer_.getCurrentPosition() - serializer_.getBufferPointer()) & 3)) & 3);
    return serialize(subheader);
}

template<class T>
inline bool OutputMessage::serialize(const T& data)
{
    bool rv = true;
    try
    {
        data.serialize(serializer_);
    }
    catch(eprosima::fastcdr::exception::NotEnoughMemoryException & /*exception*/)
    {
        std::cout << "serialize eprosima::fastcdr::exception::NotEnoughMemoryException" << std::endl;
        rv = false;
    }
    return rv;
}

template bool OutputMessage::serialize(const dds::xrce::MessageHeader& data);
template bool OutputMessage::serialize(const dds::xrce::SubmessageHeader& data);
template bool OutputMessage::serialize(const dds::xrce::STATUS_Payload& data);
template bool OutputMessage::serialize(const dds::xrce::STATUS_AGENT_Payload& data);

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_MESSAGE_OUTPUT_MESSAGE_HPP_
