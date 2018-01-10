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

#include <DDSXRCETypes.h>
#include <MessageHeader.h>
#include <Payloads.h>
#include <SubMessageHeader.h>

namespace eprosima {
namespace micrortps {
namespace testing {

class CommonData
{
  protected:
    const ClientKey client_key      = {{0xF1, 0xF2, 0xF3, 0xF4}};
    const XrceVendorId vendor_id    = {{0x00, 0x01}};
    const RequestId request_id      = {{1, 2}};
    const ObjectId object_id        = {{10, 20}};
    const uint8_t session_id        = 0x01;
    const uint8_t stream_id         = 0x04;
    const uint16_t sequence_nr      = 0x0001;
    const uint8_t flags             = 0x07;
    const uint16_t max_samples      = 0x0001;
    const uint32_t max_elapsed_time = 0x00000001;
    const uint32_t max_rate         = 0x00000012;

    MessageHeader generate_message_header() const;

    SubmessageHeader generate_submessage_header(const SubmessageId& submessage_id, uint16_t length) const;

    CREATE_CLIENT_Payload generate_create_client_payload() const;
    CREATE_Payload generate_create_payload(const OBJECTKIND& object_kind) const;

    DELETE_RESOURCE_Payload generate_delete_resource_payload(const ObjectId& obj_id) const;

    OBJK_CLIENT_Representation generate_client_representation() const;

    OBJK_PUBLISHER_Representation generate_publisher_representation() const;

    OBJK_SUBSCRIBER_Representation generate_subscriber_representation() const;

    ObjectVariant generate_object_variant(const OBJECTKIND& object_kind) const;

    RESOURCE_STATUS_Payload generate_resource_status_payload(uint8_t status, uint8_t implementation_status) const;

    READ_DATA_Payload generate_read_data_payload(const Optional<std::string>& filter, const DataFormat& format) const;

    WRITE_DATA_Payload generate_write_data_payload() const;

    DATA_Payload_Data generate_data_payload_data(uint8_t status, uint8_t implementation_status) const;
};
} // namespace testing
} // namespace micrortps
} // namespace eprosima

#endif // !_TESTS_COMMON_H
