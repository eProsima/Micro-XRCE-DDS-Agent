#include "agent/XRCEFactory.h"

#include "agent/MessageHeader.h"
#include "agent/SubMessageHeader.h"
#include "agent/Payloads.h"

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