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
#include <uxr/agent/utils/Functions.hpp>
#include <uxr/agent/config.hpp>
#include <fastcdr/Cdr.h>
#include <fastcdr/exceptions/Exception.h>
#include <iostream>

namespace eprosima {
namespace uxr {

class OutputMessage
{
public:
    OutputMessage(const dds::xrce::MessageHeader& header, size_t size)
        : buf_(new uint8_t[size]{0}),
          size_(size),
          fastbuffer_(reinterpret_cast<char*>(buf_), size_),
          serializer_(fastbuffer_)
    {
        serialize(header);
    }

    ~OutputMessage()
    {
        delete[] buf_;
    }

    uint8_t* get_buf() { return buf_; }
    size_t get_len() { return serializer_.getSerializedDataLength(); }
    template<class T>
    bool append_submessage(dds::xrce::SubmessageId submessage_id, const T& data, uint8_t flags = 0x01);
    bool append_raw_payload(dds::xrce::SubmessageId submessage_id, const uint8_t* buf, size_t len, uint8_t flags = 0x01);
    bool append_fragment(const dds::xrce::SubmessageHeader& subheader, uint8_t* buf, size_t len);

private:
    bool append_subheader(dds::xrce::SubmessageId submessage_id, uint8_t flags, size_t submessage_len);
    template<class T> bool serialize(const T& data);

private:
    uint8_t* buf_;
    size_t size_;
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

inline bool OutputMessage::append_raw_payload(dds::xrce::SubmessageId submessage_id,
                                              const uint8_t* buf,
                                              size_t len,
                                              uint8_t flags)
{
    bool rv = true;
    if (append_subheader(submessage_id, flags, len))
    {
        try
        {
            serializer_.serializeArray(buf, len, fastcdr::Cdr::BIG_ENDIANNESS);
        }
        catch(eprosima::fastcdr::exception::NotEnoughMemoryException & /*exception*/)
        {
            std::cerr << "serialize eprosima::fastcdr::exception::NotEnoughMemoryException" << std::endl;
            rv = false;
        }
    }
    else
    {
        rv = false;
    }
    return rv;
}

inline bool OutputMessage::append_fragment(const dds::xrce::SubmessageHeader& subheader, uint8_t* buf, size_t len)
{
    bool rv = false;
    serializer_.jump((4 - ((serializer_.getCurrentPosition() - serializer_.getBufferPointer()) & 3)) & 3);
    if (serialize(subheader))
    {
        try
        {
            rv = true;
            serializer_.serializeArray(buf, len);
        }
        catch(eprosima::fastcdr::exception::NotEnoughMemoryException & /*exception*/)
        {
            std::cerr << "serialize eprosima::fastcdr::exception::NotEnoughMemoryException" << std::endl;
            rv = false;
        }
    }
    return rv;
}

inline bool OutputMessage::append_subheader(dds::xrce::SubmessageId submessage_id, uint8_t flags, size_t submessage_len)
{
    dds::xrce::SubmessageHeader subheader;
    subheader.submessage_id(submessage_id);
    subheader.flags(flags);
    subheader.submessage_length(uint16_t(submessage_len));

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
        std::cerr << "serialize eprosima::fastcdr::exception::NotEnoughMemoryException" << std::endl;
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
