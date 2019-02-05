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
#include <uxr/agent/middleware/Middleware.hpp>

namespace eprosima {
namespace uxr {

Participant* Participant::create(
        const dds::xrce::ObjectId& object_id,
        const dds::xrce::OBJK_PARTICIPANT_Representation& representation,
        Middleware* middleware)
{
    bool created_entity = false;
    uint16_t raw_object_id = uint16_t((object_id[0] << 8) + object_id[1]);

    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref_rep = representation.representation().object_reference();
            created_entity = middleware->create_participant_from_ref(raw_object_id, ref_rep);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml_rep = representation.representation().xml_string_representation();
            created_entity = middleware->create_participant_from_xml(raw_object_id, xml_rep);
            break;
        }
        default:
            break;
    }

    return (created_entity ? new Participant(object_id, middleware) : nullptr);
}

Participant::Participant(
        const dds::xrce::ObjectId& id,
        Middleware* middleware)
    : XRCEObject(id, middleware)
{}

Participant::~Participant()
{
    middleware_->delete_participant(get_raw_id());
}

void Participant::release(ObjectContainer& root_objects)
{
    while (!tied_objects_.empty())
    {
        auto obj = tied_objects_.begin();
        root_objects.at(*obj)->release(root_objects);
        root_objects.erase(*obj);
    }
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
            rv = middleware_->matched_participant_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.participant().representation().xml_string_representation();
            rv = middleware_->matched_participant_from_xml(get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
