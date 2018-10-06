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
#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../xmlobjects/xmlobjects.h"

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"

namespace eprosima {
namespace uxr {

Participant::Participant(const dds::xrce::ObjectId& id) : XRCEObject{id} {}

Participant::~Participant()
{
    if (nullptr != rtps_participant_)
    {
        fastrtps::Domain::removeParticipant(rtps_participant_);
    }
}

bool Participant::init_by_ref(const std::string& ref_rep)
{
    return !(nullptr == rtps_participant_ &&
             nullptr == (rtps_participant_ = fastrtps::Domain::createParticipant(ref_rep, this)));
}

bool Participant::init_by_xml(const std::string& xml_rep)
{
    bool rv = false;
    if (nullptr == rtps_participant_)
    {
        fastrtps::ParticipantAttributes attributes;
        if (xmlobjects::parse_participant(xml_rep.data(), xml_rep.size(), attributes))
        {
            rtps_participant_ = fastrtps::Domain::createParticipant(attributes, this);
            rv = (nullptr != rtps_participant_);
        }
    }
    return rv;
}

void Participant::register_topic(const std::string& topic_name, const dds::xrce::ObjectId& object_id)
{
    registered_topics_[topic_name] = object_id;
}

void Participant::unregister_topic(const std::string& topic_name)
{
    registered_topics_.erase(topic_name);
}

bool Participant::check_register_topic(const std::string& topic_name, dds::xrce::ObjectId& object_id)
{
    bool rv = false;
    auto it = registered_topics_.find(topic_name);
    if (it != registered_topics_.end())
    {
        object_id = it->second;
        rv = true;
    }
    return rv;
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

void Participant::onParticipantDiscovery(eprosima::fastrtps::Participant*, eprosima::fastrtps::ParticipantDiscoveryInfo info)
{
    if(info.rtps.m_status == eprosima::fastrtps::rtps::DISCOVERED_RTPSPARTICIPANT)
    {
        std::cout << "RTPS Participant matched " << info.rtps.m_guid << std::endl;
    }
    else
    {
        std::cout << "RTPS Participant unmatched " << info.rtps.m_guid << std::endl;
    }
}

bool Participant::matched(const dds::xrce::OBJK_PARTICIPANT_Representation& representation) const
{
    bool rv = false;
    fastrtps::ParticipantAttributes old_attributes = rtps_participant_->getAttributes();
    fastrtps::ParticipantAttributes new_attributes;

    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref_rep = representation.representation().object_reference();
            if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
                fastrtps::xmlparser::XMLProfileManager::fillParticipantAttributes(ref_rep, new_attributes))
            {
                rv = (new_attributes == old_attributes);
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml_rep = representation.representation().xml_string_representation();
            if (xmlobjects::parse_participant(xml_rep.data(), xml_rep.size(), new_attributes))
            {
                rv = (new_attributes == old_attributes);
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
