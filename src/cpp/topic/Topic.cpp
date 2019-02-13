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

#include <uxr/agent/topic/Topic.hpp>
#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/participant/Participant.hpp>

namespace eprosima {
namespace uxr {

std::unique_ptr<Topic> Topic::create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant,
        const dds::xrce::OBJK_TOPIC_Representation& representation)
{
    bool created_entity = false;
    uint16_t raw_object_id = uint16_t((object_id[0] << 8) + object_id[1]);

    Middleware& middleware = participant->get_middleware();
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            created_entity = middleware.create_topic_by_ref(raw_object_id, participant->get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            created_entity = middleware.create_topic_by_xml(raw_object_id, participant->get_raw_id(), xml);
            break;
        }
        default:
            break;
    }

    return (created_entity ? std::unique_ptr<Topic>(new Topic(object_id, participant)) : nullptr);
}

Topic::Topic(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant)
    : XRCEObject(object_id)
    , participant_(participant)
{
    participant_->tie_object(object_id);
}

Topic::~Topic()
{
    participant_->untie_object(get_id());
    get_middleware().delete_topic(get_raw_id(), participant_->get_raw_id());
}

void Topic::release(ObjectContainer& root_objects)
{
    while (!tied_objects_.empty())
    {
        auto obj = tied_objects_.begin();
        root_objects.at(*obj)->release(root_objects);
        root_objects.erase(*obj);
    }
}

bool Topic::matched(const dds::xrce::ObjectVariant& new_object_rep) const
{
    /* Check ObjectKind. */
    if ((get_id().at(1) & 0x0F) != new_object_rep._d())
    {
        return false;
    }

    bool rv = false;
    switch (new_object_rep.topic().representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = new_object_rep.topic().representation().object_reference();
            rv = get_middleware().matched_topic_from_ref(get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = new_object_rep.topic().representation().xml_string_representation();
            rv = get_middleware().matched_topic_from_xml(get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
}

Middleware& Topic::get_middleware() const
{
    return participant_->get_middleware();
}

} // namespace uxr
} // namespace eprosima
