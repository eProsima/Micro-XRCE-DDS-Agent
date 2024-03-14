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
#include <uxr/agent/topic/Topic.hpp>
#include <uxr/agent/client/ProxyClient.hpp>
#include <uxr/agent/logger/Logger.hpp>

namespace eprosima {
namespace uxr {

std::unique_ptr<DataWriter> DataWriter::create(
        const dds::xrce::ObjectId& object_id,
        uint16_t publisher_id,
        const std::shared_ptr<ProxyClient>& proxy_client,
        const dds::xrce::DATAWRITER_Representation& representation)
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
                middleware.create_datawriter_by_ref(raw_object_id, publisher_id, ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            created_entity =
                middleware.create_datawriter_by_xml(raw_object_id, publisher_id, xml);
            break;
        }
        case dds::xrce::REPRESENTATION_IN_BINARY:
        {
            auto rep = representation.representation();
            dds::xrce::OBJK_DataWriter_Binary datawriter_xrce;

            fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(const_cast<uint8_t*>(rep.binary_representation().data())), rep.binary_representation().size()};
            eprosima::fastcdr::Cdr::Endianness endianness = static_cast<eprosima::fastcdr::Cdr::Endianness>(representation.endianness());
            eprosima::fastcdr::Cdr cdr(fastbuffer, endianness, eprosima::fastcdr::CdrVersion::XCDRv1);
            datawriter_xrce.deserialize(cdr);

            created_entity = proxy_client->get_middleware().create_datawriter_by_bin(raw_object_id, publisher_id, datawriter_xrce);
            break;
        }
        default:
            break;
    }

    return (created_entity ? std::unique_ptr<DataWriter>(new DataWriter(object_id, proxy_client)) : nullptr);
}

DataWriter::DataWriter(const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<ProxyClient>& proxy_client)
    : XRCEObject{object_id}
    , proxy_client_{proxy_client}
{}

DataWriter::~DataWriter()
{
    proxy_client_->get_middleware().delete_datawriter(get_raw_id());
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
            rv = proxy_client_->get_middleware().matched_datawriter_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.data_writer().representation().xml_string_representation();
            rv = proxy_client_->get_middleware().matched_datawriter_from_xml(get_raw_id(), xml);
            break;
        }
        case dds::xrce::REPRESENTATION_IN_BINARY:
        {
            auto rep = new_object_rep.data_writer().representation();
            dds::xrce::OBJK_DataWriter_Binary datawriter_xrce;

            fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(const_cast<uint8_t*>(rep.binary_representation().data())), rep.binary_representation().size()};
            eprosima::fastcdr::Cdr::Endianness endianness = static_cast<eprosima::fastcdr::Cdr::Endianness>(new_object_rep.endianness());
            eprosima::fastcdr::Cdr cdr(fastbuffer, endianness, eprosima::fastcdr::CdrVersion::XCDRv1);
            datawriter_xrce.deserialize(cdr);

            rv = proxy_client_->get_middleware().matched_datawriter_from_bin(get_raw_id(), datawriter_xrce);
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
    if (proxy_client_->get_middleware().write_data(get_raw_id(), write_data.data().serialized_data()))
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
    if (proxy_client_->get_middleware().write_data(get_raw_id(), data))
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
