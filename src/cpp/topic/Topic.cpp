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

#include <micrortps/agent/topic/Topic.hpp>
#include <xmlobjects/xmlobjects.h>
#include <fastrtps/Domain.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace micrortps {

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"

Topic::Topic(const dds::xrce::ObjectId& id, Participant& rtps_participant)
    : XRCEObject{id},
      rtps_participant_(rtps_participant),
      generic_type_(false)
{
}

Topic::~Topic()
{
    fastrtps::Domain::unregisterType(&rtps_participant_, generic_type_.getName());
}

bool Topic::init(const std::string& xmlrep)
{
    TopicAttributes attributes;
    if (xmlobjects::parse_topic(xmlrep.data(), xmlrep.size(), attributes))
    {
        generic_type_.setName(attributes.getTopicDataType().data());
        generic_type_.m_isGetKeyDefined = (attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
        return fastrtps::Domain::registerType(&rtps_participant_, &generic_type_);
    }
    else
    {
        return false;
    }
}

} // namespace micrortps
} // namespace eprosima
