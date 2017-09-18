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

/**
 * @file DataWriter.cpp
 *
 */


#include <fastrtps/Domain.h>

#include <agent/datawriter/DataWriter.h>


#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"
#define DEFAULT_XRCE_PUBLISHER_PROFILE "default_xrce_publisher_profile"


namespace eprosima {
namespace micrortps {

DataWriter::DataWriter(fastrtps::Participant* participant, const std::string &rtps_publisher_profile):
        mp_rtps_participant(participant),
        mp_rtps_publisher(nullptr),
        m_rtps_publisher_prof(rtps_publisher_profile)

{
    init();
}

DataWriter::~DataWriter()
{
    if (nullptr != mp_rtps_publisher)
    {
        fastrtps::Domain::removePublisher(mp_rtps_publisher);
    }

    // TODO: remove participant?
    if (nullptr != mp_rtps_participant)
    {
        fastrtps::Domain::removeParticipant(mp_rtps_participant);
    }
}

bool DataWriter::init()
{
    if (nullptr == mp_rtps_participant &&
        nullptr == (mp_rtps_participant = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)))
    {
        return false;
    }

    fastrtps::Domain::registerType(mp_rtps_participant,(fastrtps::TopicDataType*) &m_shape_type);

    if (!m_rtps_publisher_prof.empty())
    {
        mp_rtps_publisher = fastrtps::Domain::createPublisher(mp_rtps_participant, m_rtps_publisher_prof, nullptr);
    }
    else
    {
        mp_rtps_publisher = fastrtps::Domain::createPublisher(mp_rtps_participant, DEFAULT_XRCE_PUBLISHER_PROFILE, nullptr);
    }

    if(mp_rtps_publisher == nullptr)
    {
        return false;
    }
    return true;
}

bool DataWriter::write(void* data)
{
    if (nullptr == mp_rtps_publisher)
    {
        return false;
    }
    return mp_rtps_publisher->write(data);
}


} /* namespace micrortps */
} /* namespace eprosima */
