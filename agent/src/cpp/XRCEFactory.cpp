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

#include "agent/MessageHeader.h"
#include "agent/Payloads.h"
#include "agent/SubMessageHeader.h"

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

void XRCEFactory::status(const Status& payload)
{
    submessage_header(STATUS, 0x07, static_cast<uint16_t>(payload.getCdrSerializedSize(payload)));
    serializer_.serialize(payload);
}

void XRCEFactory::data(const DATA_PAYLOAD& payload)
{
    submessage_header(DATA, 0x07, static_cast<uint16_t>(payload.getCdrSerializedSize(payload)));
    serializer_.serialize(payload);
}

void XRCEFactory::submessage_header(uint8_t submessage_id, uint8_t flags, uint16_t submessage_length)
{
    SubmessageHeader subMessage = SubmessageHeader();
    subMessage.submessage_id(submessage_id);
    subMessage.flags(flags);
    subMessage.submessage_length(submessage_length);
    serializer_.serialize(subMessage); // Add submessage
}


} /* namespace micrortps */
} /* namespace eprosima */
