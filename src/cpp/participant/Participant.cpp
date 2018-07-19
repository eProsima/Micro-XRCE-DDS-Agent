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

#include <micrortps/agent/participant/Participant.hpp>
#include <xmlobjects/xmlobjects.h>
#include <fastrtps/Domain.h>

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"

namespace eprosima {
namespace micrortps {

Participant::Participant(const dds::xrce::ObjectId& id) : XRCEObject{id} {}

Participant::~Participant()
{
    if (nullptr != rtps_participant_)
    {
        fastrtps::Domain::removeParticipant(rtps_participant_);
    }
    std::cout << "Participant deleted!!" << std::endl;
}

bool Participant::init()
{
    return !(nullptr == rtps_participant_ &&
             nullptr == (rtps_participant_ = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)));
}

bool Participant::init(const std::string& xml_rep)
{
    return !(nullptr == rtps_participant_ &&
             nullptr == (rtps_participant_ = fastrtps::Domain::createParticipant(xml_rep)));
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
    for (auto obj : tied_objects_)
    {
        root_objects.at(obj)->release(root_objects);
        root_objects.erase(obj);
    }
}

} // namespace micrortps
} // namespace eprosima
