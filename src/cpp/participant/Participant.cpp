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

#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/client/ProxyClient.hpp>
#include <uxr/agent/types/XRCETypes.hpp>

namespace eprosima {
namespace uxr {

std::unique_ptr<Participant> Participant::create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<ProxyClient>& proxy_client,
        const dds::xrce::OBJK_PARTICIPANT_Representation& representation)
{
    bool created_entity = false;
    uint16_t raw_object_id = conversion::objectid_to_raw(object_id);

    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref_rep = representation.representation().object_reference();
            created_entity = proxy_client->get_middleware().create_participant_by_ref(raw_object_id, representation.domain_id(), ref_rep);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml_rep = representation.representation().xml_string_representation();
            created_entity = proxy_client->get_middleware().create_participant_by_xml(raw_object_id, representation.domain_id(), xml_rep);
            break;
        }
        case dds::xrce::REPRESENTATION_IN_BINARY:
        {
            auto rep = representation.representation();
            dds::xrce::OBJK_DomainParticipant_Binary participant_xrce;
            participant_xrce.domain_id(representation.domain_id());

            fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(const_cast<uint8_t*>(rep.binary_representation().data())), rep.binary_representation().size()};
            eprosima::fastcdr::Cdr::Endianness endianness = static_cast<eprosima::fastcdr::Cdr::Endianness>(representation.endianness());
            eprosima::fastcdr::Cdr cdr(fastbuffer, endianness, eprosima::fastcdr::CdrVersion::XCDRv1);
            participant_xrce.deserialize(cdr);

            created_entity = proxy_client->get_middleware().create_participant_by_bin(raw_object_id, participant_xrce);
            break;
        }
        default:
            break;
    }

    return (created_entity ? std::unique_ptr<Participant>(new Participant(object_id, proxy_client)) : nullptr);
}

Participant::Participant(
        const dds::xrce::ObjectId& id,
        const std::shared_ptr<ProxyClient>& proxy_client)
    : XRCEObject(id)
    , proxy_client_{proxy_client}
{}

Participant::~Participant()
{
    proxy_client_->get_middleware().delete_participant(get_raw_id());
}

bool Participant::matched(const dds::xrce::ObjectVariant& new_object_rep) const
{
    /* Check ObjectKind. */
    if ((get_id().at(1) & 0x0F) != new_object_rep._d())
    {
        return false;
    }

    bool rv = false;
    switch (new_object_rep.participant().representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = new_object_rep.participant().representation().object_reference();
            const int16_t domain_id = new_object_rep.participant().domain_id();
            rv = proxy_client_->get_middleware().matched_participant_from_ref(get_raw_id(), domain_id, ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.participant().representation().xml_string_representation();
            const int16_t domain_id = new_object_rep.participant().domain_id();
            rv = proxy_client_->get_middleware().matched_participant_from_xml(get_raw_id(), domain_id, xml);
            break;
        }
        case dds::xrce::REPRESENTATION_IN_BINARY:
        {
            auto rep = new_object_rep.participant().representation();
            dds::xrce::OBJK_DomainParticipant_Binary participant_xrce;
            int16_t domain_id = new_object_rep.participant().domain_id();
            participant_xrce.domain_id(domain_id);

            fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(const_cast<uint8_t*>(rep.binary_representation().data())), rep.binary_representation().size()};
            eprosima::fastcdr::Cdr::Endianness endianness = static_cast<eprosima::fastcdr::Cdr::Endianness>(new_object_rep.endianness());
            eprosima::fastcdr::Cdr cdr(fastbuffer, endianness, eprosima::fastcdr::CdrVersion::XCDRv1);
            participant_xrce.deserialize(cdr);

            rv = proxy_client_->get_middleware().matched_participant_from_bin(get_raw_id(), domain_id, participant_xrce);
            break;
        }
        default:
            break;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
