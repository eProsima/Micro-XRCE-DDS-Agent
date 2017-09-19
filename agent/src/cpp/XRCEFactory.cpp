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
#include "agent/SubMessageHeader.h"
#include "agent/Payloads.h"

namespace eprosima {
namespace micrortps {

size_t XRCEFactory::get_total_size()
{
    return serializer_.get_serialized_size();
}

void XRCEFactory::header(int32_t client_key, uint8_t session_id, uint8_t stream_id, uint16_t sequence_nr)
{
    MessageHeader myHeader;
    myHeader.client_key(client_key);
    myHeader.session_id(session_id);
    myHeader.stream_id(stream_id);
    myHeader.sequence_nr(sequence_nr);
    serializer_.serialize(myHeader); // Add message header
}

void XRCEFactory::status(const RESOURCE_STATUS_PAYLOAD& payload)
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
