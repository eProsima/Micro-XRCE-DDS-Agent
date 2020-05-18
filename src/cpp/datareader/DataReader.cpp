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

#include <uxr/agent/datareader/DataReader.hpp>
#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/topic/Topic.hpp>
#include <uxr/agent/client/ProxyClient.hpp>
#include <uxr/agent/utils/TokenBucket.hpp>
#include <uxr/agent/logger/Logger.hpp>

namespace eprosima {
namespace uxr {

std::unique_ptr<DataReader> DataReader::create(
        const dds::xrce::ObjectId& object_id,
        uint16_t subscriber_id,
        const std::shared_ptr<ProxyClient>& proxy_client,
        const dds::xrce::DATAREADER_Representation& representation)
{
    bool created_entity = false;
    uint16_t raw_object_id = conversion::objectid_to_raw(object_id);

    Middleware& middleware = proxy_client->get_middleware();
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            created_entity =
                middleware.create_datareader_by_ref(raw_object_id, subscriber_id, ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            created_entity =
                middleware.create_datareader_by_xml(raw_object_id, subscriber_id, xml);
            break;
        }
        default:
            break;
    }

    return (created_entity ? std::unique_ptr<DataReader>(new DataReader(object_id, proxy_client)) : nullptr);
}

DataReader::DataReader(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<ProxyClient>& proxy_client)
    : XRCEObject{object_id}
    , proxy_client_{proxy_client}
    , reader_{}
{}

DataReader::~DataReader() noexcept
{
    reader_.stop_reading();
    proxy_client_->get_middleware().delete_datareader(get_raw_id());
}

bool DataReader::matched(
        const dds::xrce::ObjectVariant& new_object_rep) const
{
    /* Check ObjectKind. */
    if ((get_id().at(1) & 0x0F) != new_object_rep._d())
    {
        return false;
    }

    bool rv = false;
    switch (new_object_rep.data_reader().representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = new_object_rep.data_reader().representation().object_reference();
            rv = proxy_client_->get_middleware().matched_datareader_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.data_reader().representation().xml_string_representation();
            rv = proxy_client_->get_middleware().matched_datareader_from_xml(get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
}

bool DataReader::read(
        const dds::xrce::READ_DATA_Payload& read_data,
        Reader<bool>::WriteFn write_fn,
        WriteFnArgs& write_args)
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

    /* TODO (julianbermudez): implement different data formats.
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

    write_args.client = proxy_client_;

    using namespace std::placeholders;
    return (reader_.stop_reading() &&
            reader_.start_reading(delivery_control, std::bind(&DataReader::read_fn, this, _1, _2, _3), false, write_fn, write_args));
}

bool DataReader::read_fn(
        bool,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    if (proxy_client_->get_middleware().read_data(get_raw_id(), data, timeout))
    {
        UXR_AGENT_LOG_MESSAGE(
            UXR_DECORATE_YELLOW("[==>> DDS <<==]"),
            get_raw_id(),
            data.data(),
            data.size());
        rv = true;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
