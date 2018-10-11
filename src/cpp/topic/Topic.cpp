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
#include <uxr/agent/participant/Participant.hpp>
#include <fastrtps/Domain.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../xmlobjects/xmlobjects.h"

namespace eprosima {
namespace uxr {

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"

Topic::Topic(const dds::xrce::ObjectId& object_id, const std::shared_ptr<Participant>& participant)
    : XRCEObject{object_id},
      participant_(participant),
      generic_type_(false)
{
    participant_->tie_object(object_id);
}

Topic::~Topic()
{
    fastrtps::Domain::unregisterType(participant_->get_rtps_participant(), generic_type_.getName());
    participant_->unregister_topic(generic_type_.getName());
    participant_->untie_object(get_id());
}

bool Topic::init(const dds::xrce::OBJK_TOPIC_Representation& representation)
{
    bool rv = false;
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref_rep = representation.representation().object_reference();
            fastrtps::TopicAttributes attributes;
            if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
                fastrtps::xmlparser::XMLProfileManager::fillTopicAttributes(ref_rep, attributes))
            {
                generic_type_.setName(attributes.getTopicDataType().data());
                generic_type_.m_isGetKeyDefined = (attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
                if (fastrtps::Domain::registerType(participant_->get_rtps_participant(), &generic_type_))
                {
                    participant_->register_topic(generic_type_.getName(), get_id());
                    rv = true;
                }
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml_rep = representation.representation().xml_string_representation();
            fastrtps::TopicAttributes attributes;
            if (xmlobjects::parse_topic(xml_rep.data(), xml_rep.size(), attributes))
            {
                generic_type_.setName(attributes.getTopicDataType().data());
                generic_type_.m_isGetKeyDefined = (attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
                if (fastrtps::Domain::registerType(participant_->get_rtps_participant(), &generic_type_))
                {
                    participant_->register_topic(generic_type_.getName(), get_id());
                    rv = true;
                }
            }
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

bool Topic::matched(const dds::xrce::OBJK_TOPIC_Representation& representation) const
{
    bool rv = false;
    fastrtps::TopicAttributes new_attributes;

    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref_rep = representation.representation().object_reference();
            if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
                fastrtps::xmlparser::XMLProfileManager::fillTopicAttributes(ref_rep, new_attributes))
            {
                rv = (0 == std::strcmp(generic_type_.getName(), new_attributes.getTopicDataType().data())) &&
                     (generic_type_.m_isGetKeyDefined == (new_attributes.getTopicKind() ==
                                                          fastrtps::rtps::TopicKind_t::WITH_KEY));
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml_rep = representation.representation().xml_string_representation();
            if (xmlobjects::parse_topic(xml_rep.data(), xml_rep.size(), new_attributes))
            {
                rv = (0 == std::strcmp(generic_type_.getName(), new_attributes.getTopicDataType().data())) &&
                     (generic_type_.m_isGetKeyDefined == (new_attributes.getTopicKind() ==
                                                          fastrtps::rtps::TopicKind_t::WITH_KEY));
            }
            break;
        }
        default:
            break;
    }

    return rv;
}

} // namespace uxr
} // namespace eprosima
