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

#ifndef _TESTS_COMMON_H
#define _TESTS_COMMON_H

#include <uxr/agent/types/XRCETypes.hpp>
#include <uxr/agent/types/MessageHeader.hpp>
#include <uxr/agent/types/SubMessageHeader.hpp>

namespace eprosima {
namespace uxr {
namespace testing {

class CommonData
{
  protected:
    const dds::xrce::ClientKey client_key      = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const dds::xrce::XrceVendorId vendor_id    = {{0x00, 0x01}};
    const dds::xrce::RequestId request_id      = {{1, 2}};
    const dds::xrce::ObjectId object_id        = {{10, 20}};
    const uint8_t session_id                   = 0x01;
    const uint8_t stream_id                    = 0x04;
    const uint16_t sequence_nr                 = 0x0001;
    const uint8_t flags                        = 0x07;
    const uint16_t max_samples                 = 0x0001;
    const uint16_t max_elapsed_time            = 0x0001;
    const uint16_t max_rate                    = 0x0012;

    dds::xrce::MessageHeader generate_message_header() const;

    dds::xrce::SubmessageHeader generate_submessage_header(const dds::xrce::SubmessageId& submessage_id, uint16_t length) const;

    dds::xrce::CREATE_CLIENT_Payload generate_create_client_payload() const;
    dds::xrce::CREATE_Payload generate_create_payload(const dds::xrce::ObjectKind& object_kind) const;
    dds::xrce::DELETE_Payload generate_delete_resource_payload(const dds::xrce::ObjectId& obj_id) const;
    dds::xrce::CLIENT_Representation generate_client_representation() const;
    dds::xrce::OBJK_PUBLISHER_Representation generate_publisher_representation() const;
    dds::xrce::OBJK_SUBSCRIBER_Representation generate_subscriber_representation() const;
    dds::xrce::OBJK_PARTICIPANT_Representation generate_participant_representation() const;
    dds::xrce::ObjectVariant generate_object_variant(const dds::xrce::ObjectKind& object_kind) const;
    dds::xrce::STATUS_Payload generate_resource_status_payload(uint8_t status, uint8_t implementation_status) const;
    dds::xrce::READ_DATA_Payload generate_read_data_payload(const Optional<std::string>& filter, const dds::xrce::DataFormat& format) const;
    dds::xrce::WRITE_DATA_Payload_Data generate_write_data_payload() const;
    dds::xrce::DATA_Payload_Data generate_data_payload_data() const;
};
} // namespace testing
} // namespace uxr
} // namespace eprosima

#endif // !_TESTS_COMMON_H
