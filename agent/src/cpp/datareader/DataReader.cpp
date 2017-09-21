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

/*
 * Subscriber.cpp
 *
 */

#include <fastrtps/Domain.h>
#include <fastrtps/subscriber/SampleInfo.h>

#include <agent/datareader/DataReader.h>

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"
#define DEFAULT_XRCE_SUBSCRIBER_PROFILE "default_xrce_subscriber_profile"



namespace eprosima {
namespace micrortps {

DataReader::DataReader(const ReaderListener* read_list):
        mp_reader_listener(read_list),
        mp_rtps_participant(nullptr),
        mp_rtps_subscriber(nullptr),
        m_rtps_subscriber_prof("")
{

}

DataReader::~DataReader()
{
    if (nullptr != mp_rtps_subscriber)
    {
        fastrtps::Domain::removeSubscriber(mp_rtps_subscriber);
    }

    // TODO: remove participant?
    if (nullptr != mp_rtps_participant)
    {
        fastrtps::Domain::removeParticipant(mp_rtps_participant);
    }
}

bool DataReader::init()
{
    if (nullptr == mp_rtps_participant &&
        nullptr == (mp_rtps_participant = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)))
    {
        return false;
    }

    fastrtps::Domain::registerType(mp_rtps_participant,(fastrtps::TopicDataType*) &m_shape_type);

    if (!m_rtps_subscriber_prof.empty())
    {
        mp_rtps_subscriber = fastrtps::Domain::createSubscriber(mp_rtps_participant, m_rtps_subscriber_prof, nullptr);
    }
    else
    {
        mp_rtps_subscriber = fastrtps::Domain::createSubscriber(mp_rtps_participant, DEFAULT_XRCE_SUBSCRIBER_PROFILE, nullptr);
    }

    if(mp_rtps_subscriber == nullptr)
    {
        return false;
    }
    return true;
}

int DataReader::read(const READ_DATA_PAYLOAD &read_data)
{
    switch(read_data.read_mode())
    {
        case READM_DATA:
        case READM_SAMPLE:
            read_sample();
        break;
        case READM_DATA_SEQ: break;
        case READM_SAMPLE_SEQ: break;
        case READM_PACKED_SAMPLE_SEQ: break;
        default: break;
    }

    return 0;
}

int DataReader::read_sample()
{
    return 0;
}

bool DataReader::readNextData(void* data)
{
    if (nullptr == mp_rtps_subscriber)
    {
        return false;
    }
    fastrtps::SampleInfo_t info;
    return mp_rtps_subscriber->readNextData(data, &info);
}

bool DataReader::takeNextData(void* data)
{
    if (nullptr == mp_rtps_subscriber)
    {
        return false;
    }
    fastrtps::SampleInfo_t info;
    return mp_rtps_subscriber->takeNextData(data, &info);
}

} /* namespace micrortps */
} /* namespace eprosima */


