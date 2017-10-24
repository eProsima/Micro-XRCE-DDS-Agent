// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <agent/datareader/DataReader.h>
#include <agent/datawriter/DataWriter.h>
#include <agent/participant/Participant.h>
#include <xmlobjects/xmlobjects.h>

#include <fastrtps/Domain.h>

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"

namespace eprosima {
namespace micrortps {

XRCEParticipant::XRCEParticipant() : mp_rtps_participant(nullptr)
{
    init();
}

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

XRCEObject* XRCEParticipant::create_writer()
{
    return new DataWriter(mp_rtps_participant);
}

XRCEObject* XRCEParticipant::create_writer(const char* xmlrep, size_t size)
{
    return new DataWriter(xmlrep, size, mp_rtps_participant);
}

XRCEObject* XRCEParticipant::create_reader(ReaderListener* message_listener)
{
    return new DataReader(mp_rtps_participant, message_listener);
}

XRCEObject* XRCEParticipant::create_reader(const char* xmlrep, size_t size, ReaderListener* message_listener)
{
    return new DataReader(xmlrep, size, mp_rtps_participant, message_listener);
}

} /* namespace micrortps */
} /* namespace eprosima */
