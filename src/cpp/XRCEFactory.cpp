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

#include "agent/XRCEFactory.h"

#include "MessageHeader.h"
#include "Payloads.h"
#include "SubMessageHeader.h"

namespace eprosima {
namespace micrortps {

size_t XRCEFactory::get_total_size()
{
    return serializer_.get_serialized_size();
}

void XRCEFactory::header(const MessageHeader& header)
{
    serializer_.serialize(header);
}

void XRCEFactory::status(const RESOURCE_STATUS_Payload& payload)
{
    submessage_header(STATUS, 0x07, static_cast<uint16_t>(payload.getCdrSerializedSize()));
    serializer_.serialize(payload);
}

void XRCEFactory::data(const DATA_Payload_Data& payload)
{
    reply(DATA, payload);
}

void XRCEFactory::data(const DATA_Payload_Sample& payload)
{
    reply(DATA, payload);
}

void XRCEFactory::data(const DATA_Payload_DataSeq& payload)
{
    reply(DATA, payload);
}

void XRCEFactory::data(const DATA_Payload_SampleSeq& payload)
{
    reply(DATA, payload);
}

void XRCEFactory::data(const DATA_Payload_PackedSamples& payload)
{
    reply(DATA, payload);
}

void XRCEFactory::submessage_header(eprosima::micrortps::SubmessageId submessage_id, uint8_t flags, uint16_t submessage_length)
{
    SubmessageHeader subMessage = SubmessageHeader();
    subMessage.submessage_id(submessage_id);
    subMessage.flags(flags);
    subMessage.submessage_length(submessage_length);
    serializer_.serialize(subMessage); // Add submessage
}

void XRCEFactory::reply(eprosima::micrortps::SubmessageId m_submessage_id, const BaseObjectReply& object_reply)
{
    submessage_header(m_submessage_id, 0x07, static_cast<uint16_t>(object_reply.getCdrSerializedSize()));
    serializer_.serialize(object_reply);
}

} /* namespace micrortps */
} /* namespace eprosima */
