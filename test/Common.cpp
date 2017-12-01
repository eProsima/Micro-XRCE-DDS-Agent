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

#include "Common.h"

namespace eprosima {
namespace micrortps {
namespace testing {

MessageHeader CommonData::generate_message_header() const
{
    MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);
    return message_header;
}

SubmessageHeader CommonData::generate_submessage_header(const SubmessageId& submessage_id, uint16_t length) const
{
    SubmessageHeader submessage_header;
    submessage_header.submessage_id(submessage_id);
    submessage_header.flags(flags);
    submessage_header.submessage_length(length);
    return submessage_header;
}

CREATE_CLIENT_Payload CommonData::generate_create_client_payload() const
{
    CREATE_CLIENT_Payload create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation(generate_client_representation());
    return create_data;
}

CREATE_Payload CommonData::generate_create_payload(const OBJECTKIND& object_kind) const
{
    CREATE_Payload create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation(generate_object_variant(object_kind));
    return create_data;
}

DELETE_RESOURCE_Payload CommonData::generate_delete_resource_payload(const ObjectId& obj_id) const
{
    DELETE_RESOURCE_Payload delete_payload;
    delete_payload.object_id(obj_id);
    delete_payload.request_id(request_id);
    return delete_payload;
}

OBJK_CLIENT_Representation CommonData::generate_client_representation() const
{
    OBJK_CLIENT_Representation client_representation;
    client_representation.xrce_cookie(XRCE_COOKIE);
    client_representation.xrce_version(XRCE_VERSION);
    client_representation.xrce_vendor_id(vendor_id);
    client_representation.client_timestamp();
    client_representation.session_id();
    return client_representation;
}

OBJK_PUBLISHER_Representation CommonData::generate_publisher_representation() const
{
    OBJK_PUBLISHER_Representation publisher_rep;
    publisher_rep.representation().object_reference("PUBLISHER");
    publisher_rep.participant_id({{4, 4}});
    return publisher_rep;
}

OBJK_SUBSCRIBER_Representation CommonData::generate_subscriber_representation() const
{
    OBJK_SUBSCRIBER_Representation subscriber_rep;
    subscriber_rep.representation().object_reference("SUBSCRIBER");
    subscriber_rep.participant_id({{4, 4}});
    return subscriber_rep;
}

ObjectVariant CommonData::generate_object_variant(const OBJECTKIND& object_kind) const
{
    ObjectVariant variant;
    switch(object_kind)
    {
        case OBJECTKIND::PUBLISHER:
        {
            variant.publisher(generate_publisher_representation());
            break;
        }
        case OBJECTKIND::SUBSCRIBER:
        {
            variant.subscriber(generate_subscriber_representation());
            break;
        }
        case OBJECTKIND::INVALID:
        case OBJECTKIND::PARTICIPANT:
        case OBJECTKIND::TOPIC:
        case OBJECTKIND::DATAWRITER:
        case OBJECTKIND::DATAREADER:
        case OBJECTKIND::TYPE:
        case OBJECTKIND::QOSPROFILE:
        case OBJECTKIND::APPLICATION:
        default:
            break;
    }
    return variant;
}

RESOURCE_STATUS_Payload CommonData::generate_resource_status_payload(uint8_t status,
                                                                     uint8_t implementation_status) const
{
    RESOURCE_STATUS_Payload resource_status;
    resource_status.object_id(object_id);
    resource_status.request_id(request_id);
    resource_status.result().status(status);
    resource_status.result().implementation_status(implementation_status);
    return resource_status;
}

READ_DATA_Payload CommonData::generate_read_data_payload(const Optional<std::string>& filter,
                                                         const DataFormat& format) const
{
    READ_DATA_Payload read_data;
    read_data.object_id(object_id);
    read_data.request_id(request_id);
    if(filter)
    {
        read_data.read_specification().content_filter_expression(*filter);
    }
    DataDeliveryControl data_control;
    data_control.max_samples(max_samples);
    data_control.max_elapsed_time(max_elapsed_time);
    data_control.max_rate(max_rate);
    read_data.read_specification().delivery_config().delivery_control(data_control, format);
    return read_data;
}

WRITE_DATA_Payload CommonData::generate_write_data_payload() const
{
    WRITE_DATA_Payload write_payload;
    write_payload.object_id(object_id);
    write_payload.request_id(request_id);
    SampleData sample;
    sample.serialized_data({0x00, 0x11, 0x22, 0x33});
    write_payload.data_to_write().data(sample);
    return write_payload;
}

DATA_Payload_Data CommonData::generate_data_payload_data(uint8_t status, uint8_t implementation_status) const
{
    DATA_Payload_Data data_payload;
    data_payload.object_id(object_id);
    data_payload.request_id(request_id);
    data_payload.result().status(status);
    data_payload.result().implementation_status(implementation_status);

    SampleData sample;
    sample.serialized_data({0x00, 0x11, 0x22, 0x33});
    data_payload.data(sample);
    return data_payload;
}
} // namespace testing
} // namespace micrortps
} // namespace eprosima
