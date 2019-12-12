// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/requester/Requester.hpp>
#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/logger/Logger.hpp>
#include <uxr/agent/utils/Conversion.hpp>

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

namespace eprosima {
namespace uxr {

std::unique_ptr<Requester> Requester::create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant,
        const dds::xrce::REQUESTER_Representation& representation)
{
    bool created_entity = false;
    uint16_t raw_object_id = conversion::objectid_to_raw(object_id);

    Middleware& middleware = participant->get_middleware();
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            created_entity = middleware.create_requester_by_ref(raw_object_id, participant->get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            created_entity = middleware.create_requester_by_xml(raw_object_id, participant->get_raw_id(), xml);
            break;
        }
        default:
            break;
    }

    return (created_entity ? std::unique_ptr<Requester>(new Requester(object_id, participant)) : nullptr);
}

Requester::Requester(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant)
    : XRCEObject(object_id)
    , participant_(participant)
{
    participant_->tie_object(object_id);
}

Requester::~Requester()
{
    participant_->untie_object(get_id());
    get_middleware().delete_requester(get_raw_id());
}

bool Requester::matched(
        const dds::xrce::ObjectVariant& new_object_rep) const
{
    /* Check ObjectKind. */
    if ((get_id().at(1) & 0x0F) != new_object_rep._d())
    {
        return false;
    }

    bool rv = false;
    switch (new_object_rep.data_writer().representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = new_object_rep.requester().representation().object_reference();
            rv = get_middleware().matched_requester_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.requester().representation().object_reference();
            rv = get_middleware().matched_requester_from_xml(get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
}

Middleware& Requester::get_middleware() const
{
    return participant_->get_middleware();
}

bool Requester::write(
        dds::xrce::WRITE_DATA_Payload_Data& write_data,
        const dds::xrce::RequestId& request_id)
{
    bool rv = false;
    uint32_t sequence_number = (get_raw_id() << 16) + (request_id[0] << 8) + (request_id[1]);

    if (get_middleware().write_request(get_raw_id(), sequence_number,  write_data.data().serialized_data()))
    {
        UXR_AGENT_LOG_MESSAGE(
            UXR_DECORATE_YELLOW("[** <<DDS>> **]"),
            get_raw_id(),
            write_data.data().serialized_data().data(),
            write_data.data().serialized_data().size());
        rv = true;
    }

    return rv;
}

bool Requester::read(
        const dds::xrce::READ_DATA_Payload& read_data,
        Reader<bool>::WriteFn write_fn,
        const WriteFnArgs& write_args)
{
    dds::xrce::DataDeliveryControl delivery_control;
    if (read_data.read_specification().has_delivery_control())
    {
        delivery_control = read_data.read_specification().delivery_control();
    }
    else
    {
        delivery_control.max_elapsed_time(0);
        delivery_control.max_bytes_per_second(0);
        delivery_control.max_samples(1);
    }

    /* TODO (julianbermudez): implement different formats.
    switch (read_data.read_specification().data_format())
    {
        case dds::xrce::FORMAT_DATA:
            break;
        case dds::xrce::FORMAT_SAMPLE:
            break;
        case dds::xrce::FORMAT_DATA_SEQ:
            break;
        case dds::xrce::FORMAT_SAMPLE_SEQ:
            break;
        case dds::xrce::FORMAT_PACKED_SAMPLES:
            break;
        default:
            break;
    }
    */

    using namespace std::placeholders;
    return (reader_.stop_reading() &&
            reader_.start_reading(delivery_control, std::bind(&Requester::read_fn, this, _1, _2, _3), false, write_fn, write_args));
    return false;
}

bool Requester::read_fn(
        bool,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    uint32_t sequence_number = 0;
    dds::xrce::BaseObjectRequest request;
    std::vector<uint8_t> temp_data;

    if (get_middleware().read_reply(get_raw_id(), sequence_number, temp_data, timeout))
    {
        request.object_id() = get_id();
        request.request_id()[0] = uint8_t((sequence_number >> 8) & 0xFF);
        request.request_id()[1] = uint8_t(sequence_number & 0xFF);

        data.resize(request.getMaxCdrSerializedSize() + temp_data.size());
        fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(data.data()), data.size()};
        fastcdr::Cdr serializer(fastbuffer);

        request.serialize(serializer);
        serializer.serializeArray(temp_data.data(), temp_data.size());

        UXR_AGENT_LOG_MESSAGE(
            UXR_DECORATE_YELLOW("[==>> DDS <<==]"),
            get_raw_id(),
            temp_data.data(),
            temp_data.size());
        rv = true;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima