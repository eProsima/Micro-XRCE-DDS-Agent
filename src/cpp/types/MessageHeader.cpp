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

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace
{
char dummy;
}
#endif

#include <uxr/agent/types/MessageHeader.hpp>
#include <fastcdr/Cdr.h>
#include <fastcdr/exceptions/BadParamException.h>

using namespace eprosima::fastcdr::exception;

#include <utility>

namespace dds { namespace xrce {

MessageHeader::MessageHeader()
    : m_session_id{}
    , m_stream_id{}
    , m_sequence_nr{}
    , m_client_key{}
{
}

MessageHeader::~MessageHeader()
{
}

MessageHeader::MessageHeader(const MessageHeader &x)
{
    m_session_id  = x.m_session_id;
    m_stream_id   = x.m_stream_id;
    m_sequence_nr = x.m_sequence_nr;
    m_client_key  = x.m_client_key;
}

MessageHeader::MessageHeader(MessageHeader &&x)
{
    m_session_id  = x.m_session_id;
    m_stream_id   = x.m_stream_id;
    m_sequence_nr = x.m_sequence_nr;
    m_client_key  = std::move(x.m_client_key);
}

MessageHeader &MessageHeader::operator=(const MessageHeader &x)
{
    m_session_id  = x.m_session_id;
    m_stream_id   = x.m_stream_id;
    m_sequence_nr = x.m_sequence_nr;
    m_client_key  = x.m_client_key;

    return *this;
}

MessageHeader &MessageHeader::operator=(MessageHeader &&x)
{
    m_session_id  = x.m_session_id;
    m_stream_id   = x.m_stream_id;
    m_sequence_nr = x.m_sequence_nr;
    m_client_key  = std::move(x.m_client_key);

    return *this;
}

size_t MessageHeader::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);
    current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    return current_alignment - initial_alignment;
}

size_t MessageHeader::getCdrSerializedSize(size_t current_alignment) const
{
    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    current_alignment += 2 + eprosima::fastcdr::Cdr::alignment(current_alignment, 2);

    if (128 > m_session_id)
    {
        current_alignment += ((4) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    }

    return current_alignment - initial_alignment;
}

void MessageHeader::serialize(eprosima::fastcdr::Cdr &scdr) const
{
    scdr << m_session_id;
    scdr << m_stream_id;
    scdr.serialize(m_sequence_nr, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS);
    if (128 > m_session_id)
    {
        scdr << m_client_key;
    }
}

void MessageHeader::deserialize(eprosima::fastcdr::Cdr &dcdr)
{
    dcdr >> m_session_id;
    dcdr >> m_stream_id;
    dcdr.deserialize(m_sequence_nr, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS);
    if (128 > m_session_id)
    {
        dcdr >> m_client_key;
    }
}

} } //namespace dds::xrce
