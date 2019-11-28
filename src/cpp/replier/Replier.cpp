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

#include <uxr/agent/replier/Replier.hpp>
#include <uxr/agent/participant/Participant.hpp>

namespace eprosima {
namespace uxr {

std::unique_ptr<Replier> Replier::create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant,
        const dds::xrce::REPLIER_Representation& representation)
{
    bool created_entity = false;
    uint16_t raw_object_id = conversion::objectid_to_raw(object_id);

    Middleware& middleware = participant->get_middleware();
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            created_entity = middleware.create_replier_by_ref(raw_object_id, participant->get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            created_entity = middleware.create_replier_by_xml(raw_object_id, participant->get_raw_id(), xml);
            break;
        }
        default:
            break;
    }

    return (created_entity ? std::unique_ptr<Replier>(new Replier(object_id, participant)) : nullptr);
}

Replier::Replier(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant)
    : XRCEObject(object_id)
    , participant_(participant)
{
    participant_->tie_object(object_id);
}

Replier::~Replier()
{
    participant_->untie_object(get_id());
    get_middleware().delete_replier(get_raw_id());
}

bool Replier::write(
        dds::xrce::WRITE_DATA_Payload_Data& write_data)
{
    // TODO.
}

bool Replier::write(
        const std::vector<uint8_t>& data)
{
    // TODO.            
}

bool Replier::read(
        const dds::xrce::READ_DATA_Payload& read_data,
        read_callback read_cb,
        const ReadCallbackArgs& cb_args)
{
    // TODO.
}

bool Replier::matched(
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
            const std::string& ref = new_object_rep.replier().representation().object_reference();
            rv = get_middleware().matched_replier_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.replier().representation().object_reference();
            rv = get_middleware().matched_replier_from_xml(get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
}

Middleware& Replier::get_middleware() const
{
    return participant_->get_middleware();
}

} // namespace uxr
} // namespace eprosima