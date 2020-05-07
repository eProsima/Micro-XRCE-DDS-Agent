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

#include <uxr/agent/datawriter/DataWriter.hpp>
#include <uxr/agent/publisher/Publisher.hpp>
#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/topic/Topic.hpp>
#include <uxr/agent/client/ProxyClient.hpp>
#include <uxr/agent/logger/Logger.hpp>

namespace eprosima {
namespace uxr {

std::unique_ptr<DataWriter> DataWriter::create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Publisher>& publisher,
        const dds::xrce::DATAWRITER_Representation& representation)
{
    bool created_entity = false;
    uint16_t raw_object_id = conversion::objectid_to_raw(object_id);

    Middleware& middleware = publisher->get_participant()->get_proxy_client()->get_middleware();
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            created_entity =
                middleware.create_datawriter_by_ref(raw_object_id, publisher->get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            created_entity =
                middleware.create_datawriter_by_xml(raw_object_id, publisher->get_raw_id(), xml);
            break;
        }
        default:
            break;
    }

    return (created_entity ? std::unique_ptr<DataWriter>(new DataWriter(object_id, publisher)) : nullptr);
}

DataWriter::DataWriter(const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Publisher>& publisher)
    : XRCEObject{object_id}
    , publisher_{publisher}
{}

DataWriter::~DataWriter()
{
    publisher_->get_participant()->get_proxy_client()->get_middleware().delete_datawriter(get_raw_id());
}

bool DataWriter::matched(const dds::xrce::ObjectVariant& new_object_rep) const
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
            const std::string& ref = new_object_rep.data_writer().representation().object_reference();
            rv = publisher_->get_participant()->get_proxy_client()->get_middleware().matched_datawriter_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.data_writer().representation().xml_string_representation();
            rv = publisher_->get_participant()->get_proxy_client()->get_middleware().matched_datawriter_from_xml(get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
}

bool DataWriter::write(dds::xrce::WRITE_DATA_Payload_Data& write_data)
{
    bool rv = false;
    if (publisher_->get_participant()->get_proxy_client()->get_middleware().write_data(get_raw_id(), write_data.data().serialized_data()))
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

bool DataWriter::write(const std::vector<uint8_t>& data)
{
    bool rv = false;
    if (publisher_->get_participant()->get_proxy_client()->get_middleware().write_data(get_raw_id(), data))
    {
        UXR_AGENT_LOG_MESSAGE(
            UXR_DECORATE_YELLOW("[** <<DDS>> **]"),
            get_raw_id(),
            data.data(),
            data.size());
        rv = true;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
