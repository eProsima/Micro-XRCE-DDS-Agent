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
#include <fastrtps/Domain.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../xmlobjects/xmlobjects.h"

namespace eprosima {
namespace uxr {

Topic::Topic(const dds::xrce::ObjectId& object_id, const std::shared_ptr<Participant>& participant)
    : XRCEObject{object_id},
      participant_(participant),
      generic_type_(false)
{
    participant_->tie_object(object_id);
}

Topic::~Topic()
{
    participant_->untie_object(get_id());
}

bool Topic::init_middleware(Middleware *middleware, const dds::xrce::OBJK_TOPIC_Representation &representation)
{
    bool rv = false;
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref = representation.representation().object_reference();
            rv = middleware->create_topic_from_ref(get_raw_id(), participant_->get_raw_id(), ref);
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().xml_string_representation();
            rv = middleware->create_topic_from_xml(get_raw_id(), participant_->get_raw_id(), xml);
            break;
        }
        default:
            break;
    }
    return rv;
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

    bool parser_cond = false;
    fastrtps::TopicAttributes new_attributes;

    switch (new_object_rep.topic().representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref_rep = new_object_rep.topic().representation().object_reference();
            if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
                fastrtps::xmlparser::XMLProfileManager::fillTopicAttributes(ref_rep, new_attributes))
            {
                parser_cond = true;
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml_rep = new_object_rep.topic().representation().xml_string_representation();
            if (xmlobjects::parse_topic(xml_rep.data(), xml_rep.size(), new_attributes))
            {
                parser_cond = true;
            }
            break;
        }
        default:
            break;
    }

    return parser_cond &&  (0 == std::strcmp(generic_type_.getName(), new_attributes.getTopicDataType().data())) &&
                           (generic_type_.m_isGetKeyDefined == (new_attributes.getTopicKind() ==
                                                                fastrtps::rtps::TopicKind_t::WITH_KEY));
}

} // namespace uxr
} // namespace eprosima
