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
#include <agent/datawriter/DataWriter.h>

#include <DDSXRCETypes.h>
#include <Payloads.h>
#include <xmlobjects/xmlobjects.h>

#include <fastrtps/Domain.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"
#define DEFAULT_XRCE_PUBLISHER_PROFILE "default_xrce_publisher_profile"

namespace eprosima {
namespace micrortps {

DataWriter::DataWriter(const ObjectId& id, fastrtps::Participant* rtps_participant, const std::string& profile_name)
    : XRCEObject{id}, mp_rtps_participant(rtps_participant), mp_rtps_publisher(nullptr),
      m_rtps_publisher_prof(profile_name), topic_type_(false)

{
}

DataWriter::~DataWriter()
{
    if (nullptr != mp_rtps_publisher)
    {
        fastrtps::Domain::removePublisher(mp_rtps_publisher);
    }

    // TODO(borja): remove participant?
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

    PublisherAttributes attributes;
    if (m_rtps_publisher_prof.empty() ||
        (fastrtps::xmlparser::XMLP_ret::XML_ERROR ==
         fastrtps::xmlparser::XMLProfileManager::fillPublisherAttributes(m_rtps_publisher_prof, attributes)))
    {
        fastrtps::xmlparser::XMLProfileManager::getDefaultPublisherAttributes(attributes);
    }
    // topic_type_.setName(attributes.topic.getTopicDataType().data());
    // topic_type_.m_isGetKeyDefined = (attributes.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
    // fastrtps::Domain::registerType(mp_rtps_participant, &topic_type_);

    if (!m_rtps_publisher_prof.empty())
    {
        mp_rtps_publisher = fastrtps::Domain::createPublisher(mp_rtps_participant, m_rtps_publisher_prof, nullptr);
    }
    else
    {
        // std::cout << "init DataWriter RTPS default publisher" << std::endl;
        mp_rtps_publisher =
            fastrtps::Domain::createPublisher(mp_rtps_participant, DEFAULT_XRCE_PUBLISHER_PROFILE, nullptr);
    }

    if (mp_rtps_publisher == nullptr)
    {
        std::cout << "init publisher error" << std::endl;
        return false;
    }
    return true;
}

bool DataWriter::init(const std::string& xmlrep)
{
    if (nullptr == mp_rtps_participant &&
        nullptr == (mp_rtps_participant = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)))
    {
        return false;
    }

    PublisherAttributes attributes;
    if (xmlobjects::parse_publisher(xmlrep.data(), xmlrep.size(), attributes))
    {
        // topic_type_.setName(attributes.topic.getTopicDataType().data());
        // topic_type_.m_isGetKeyDefined = (attributes.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
        // fastrtps::Domain::registerType(mp_rtps_participant, &topic_type_);
        mp_rtps_publisher = fastrtps::Domain::createPublisher(mp_rtps_participant, attributes, nullptr);
    }
    else
    {
        fastrtps::xmlparser::XMLProfileManager::getDefaultPublisherAttributes(attributes);
        // topic_type_.setName(attributes.topic.getTopicDataType().data());
        // topic_type_.m_isGetKeyDefined = (attributes.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
        // fastrtps::Domain::registerType(mp_rtps_participant, &topic_type_);

        mp_rtps_publisher =
            fastrtps::Domain::createPublisher(mp_rtps_participant, DEFAULT_XRCE_PUBLISHER_PROFILE, nullptr);
    }

    if (mp_rtps_publisher == nullptr)
    {
        std::cout << "init publisher error" << std::endl;
        return false;
    }
    return true;
}

bool DataWriter::write(const WRITE_DATA_Payload& write_data)
{
    switch (write_data.data_to_write()._d())
    {
        case FORMAT_DATA:
        case FORMAT_DATA_SEQ:
        case FORMAT_SAMPLE:
        case FORMAT_SAMPLE_SEQ:
        case FORMAT_PACKED_SAMPLES:
        default:
            break;
    }

    if (nullptr == mp_rtps_publisher)
    {
        return false;
    }

    std::vector<uint8_t> serialized_data = write_data.data_to_write().data().serialized_data();
    return mp_rtps_publisher->write(&serialized_data);
}

} /* namespace micrortps */
} /* namespace eprosima */
