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
namespace uxr {
namespace testing {

dds::xrce::MessageHeader CommonData::generate_message_header() const
{
    dds::xrce::MessageHeader message_header;
    message_header.client_key(client_key);
    message_header.session_id(session_id);
    message_header.stream_id(stream_id);
    message_header.sequence_nr(sequence_nr);
    return message_header;
}

dds::xrce::SubmessageHeader CommonData::generate_submessage_header(const dds::xrce::SubmessageId& submessage_id,
                                                                   uint16_t length) const
{
    dds::xrce::SubmessageHeader submessage_header;
    submessage_header.submessage_id(submessage_id);
    submessage_header.flags(flags);
    submessage_header.submessage_length(length);
    return submessage_header;
}

dds::xrce::CREATE_CLIENT_Payload CommonData::generate_create_client_payload() const
{
    dds::xrce::CREATE_CLIENT_Payload create_data;
    create_data.client_representation(generate_client_representation());
    return create_data;
}

dds::xrce::CREATE_Payload CommonData::generate_create_payload(const dds::xrce::ObjectKind& object_kind) const
{
    dds::xrce::CREATE_Payload create_data;
    create_data.request_id(request_id);
    create_data.object_id(object_id);
    create_data.object_representation(generate_object_variant(object_kind));
    return create_data;
}

dds::xrce::DELETE_Payload CommonData::generate_delete_resource_payload(const dds::xrce::ObjectId& obj_id) const
{
    dds::xrce::DELETE_Payload delete_payload;
    delete_payload.object_id(obj_id);
    delete_payload.request_id(request_id);
    return delete_payload;
}

dds::xrce::CLIENT_Representation CommonData::generate_client_representation() const
{
    dds::xrce::CLIENT_Representation client_representation;
    client_representation.xrce_cookie(dds::xrce::XRCE_COOKIE);
    client_representation.xrce_version(dds::xrce::XRCE_VERSION);
    client_representation.xrce_vendor_id(vendor_id);
    client_representation.client_key(client_key);
    client_representation.session_id();
    client_representation.properties();
    return client_representation;
}

dds::xrce::OBJK_PUBLISHER_Representation CommonData::generate_publisher_representation() const
{
    dds::xrce::OBJK_PUBLISHER_Representation publisher_rep;
    /* TODO (Julian): publisher do not support refenrence representation in the new standard. */
//    publisher_rep.representation().object_reference("PUBLISHER");
    publisher_rep.participant_id({{4, 4}});
    return publisher_rep;
}

dds::xrce::OBJK_SUBSCRIBER_Representation CommonData::generate_subscriber_representation() const
{
    dds::xrce::OBJK_SUBSCRIBER_Representation subscriber_rep;
    /* TODO (Julian): subscriber do not support refenrence representation in the new standard. */
//    subscriber_rep.representation().object_reference("SUBSCRIBER");
    subscriber_rep.participant_id({{4, 4}});
    return subscriber_rep;
}

dds::xrce::OBJK_PARTICIPANT_Representation CommonData::generate_participant_representation() const
{
    dds::xrce::OBJK_PARTICIPANT_Representation participant_rep;
    participant_rep.representation().object_reference("PARTICIPA");
    return participant_rep;
}

dds::xrce::ObjectVariant CommonData::generate_object_variant(const dds::xrce::ObjectKind& object_kind) const
{
    dds::xrce::ObjectVariant variant;
    switch(object_kind)
    {
        case dds::xrce::OBJK_PUBLISHER:
        {
            variant.publisher(generate_publisher_representation());
            break;
        }
        case dds::xrce::OBJK_SUBSCRIBER:
        {
            variant.subscriber(generate_subscriber_representation());
            break;
        }
        case dds::xrce::OBJK_PARTICIPANT:
        {
            variant.participant(generate_participant_representation());
            break;
        }
        /* TODO (Julian). */
//        case OBJECTKIND::INVALID:
//        case OBJECTKIND::TOPIC:
//        case OBJECTKIND::DATAWRITER:
//        case OBJECTKIND::DATAREADER:
//        case OBJECTKIND::TYPE:
//        case OBJECTKIND::QOSPROFILE:
//        case OBJECTKIND::APPLICATION:
        default:
            break;
    }
    return variant;
}

dds::xrce::STATUS_Payload CommonData::generate_resource_status_payload(uint8_t status,
                                                                       uint8_t implementation_status) const
{
    dds::xrce::STATUS_Payload status_payload;
    status_payload.related_request().object_id(object_id);
    status_payload.related_request().request_id(request_id);
    status_payload.result().status((dds::xrce::StatusValue)status);
    status_payload.result().implementation_status(implementation_status);
    return status_payload;
}

dds::xrce::READ_DATA_Payload CommonData::generate_read_data_payload(const Optional<std::string>& filter,
                                                                    const dds::xrce::DataFormat& format) const
{
    dds::xrce::READ_DATA_Payload read_data;
    read_data.object_id(object_id);
    read_data.request_id(request_id);
    read_data.read_specification().data_format(format);
    if(filter)
    {
        read_data.read_specification().content_filter_expression(*filter);
    }
    dds::xrce::DataDeliveryControl data_control;
    data_control.max_samples(max_samples);
    data_control.max_elapsed_time(max_elapsed_time);
    data_control.max_bytes_per_second(max_rate);
    read_data.read_specification().delivery_control(data_control);
    return read_data;
}

dds::xrce::WRITE_DATA_Payload_Data CommonData::generate_write_data_payload() const
{
    dds::xrce::WRITE_DATA_Payload_Data write_payload;
    write_payload.object_id(object_id);
    write_payload.request_id(request_id);
    dds::xrce::SampleData sample;
    sample.serialized_data({0x00, 0x11, 0x22, 0x33});
    write_payload.data(sample);
    return write_payload;
}

dds::xrce::DATA_Payload_Data CommonData::generate_data_payload_data() const
{
    dds::xrce::DATA_Payload_Data data_payload;
    data_payload.object_id(object_id);
    data_payload.request_id(request_id);

    dds::xrce::SampleData sample;
    sample.serialized_data({0x00, 0x11, 0x22, 0x33});
    data_payload.data(sample);
    return data_payload;
}
} // namespace testing
} // namespace uxr
} // namespace eprosima
