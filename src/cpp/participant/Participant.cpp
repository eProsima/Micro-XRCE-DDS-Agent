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

XRCEParticipant::XRCEParticipant(const dds::xrce::ObjectId& id) : XRCEObject{id} {}

XRCEParticipant::~XRCEParticipant()
{
    if (nullptr != mp_rtps_participant)
    {
        fastrtps::Domain::removeParticipant(mp_rtps_participant);
    }
}

bool XRCEParticipant::init()
{
    return !(nullptr == mp_rtps_participant &&
             nullptr == (mp_rtps_participant = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)));
}


XRCEObject* XRCEParticipant::create_topic(const dds::xrce::ObjectId& id, const std::string& xmlrep)
{
    auto topic = new Topic(id, *mp_rtps_participant);
    if (topic->init(xmlrep))
    {
        return topic;
    }
    else
    {
        delete topic;
        return nullptr;
    }
}

XRCEObject* XRCEParticipant::create_writer(const dds::xrce::ObjectId& id, const std::string& xmlrep)
{
    auto data_writer = new DataWriter(id, *mp_rtps_participant);
    if (data_writer->init(xmlrep))
    {
        return data_writer;
    }
    else
    {
        delete data_writer;
        return nullptr;
    }
}

XRCEObject* XRCEParticipant::create_reader(const dds::xrce::ObjectId& id,
                                           const std::string& xmlrep)
{
    auto data_reader = new DataReader(id, *mp_rtps_participant);
//    auto data_reader = new DataReader(id, *mp_rtps_participant, message_listener);
    if (data_reader->init(xmlrep))
    {
        return data_reader;
    }
    else
    {
        delete data_reader;
        return nullptr;
    }
}

} /* namespace micrortps */
} /* namespace eprosima */
