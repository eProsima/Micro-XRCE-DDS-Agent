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

#include <XRCETypes.h>
#include <xmlobjects/xmlobjects.h>

#include <fastrtps/Domain.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"
#define DEFAULT_XRCE_PUBLISHER_PROFILE "default_xrce_publisher_profile"

namespace eprosima {
namespace micrortps {

DataWriter::DataWriter(const dds::xrce::ObjectId& id,
                       Participant& rtps_participant,
                       const std::string& profile_name)
    : XRCEObject{id},
      mp_rtps_participant(rtps_participant),
      mp_rtps_publisher(nullptr),
      m_rtps_publisher_prof(profile_name),
      topic_type_(false)

{
}

DataWriter::~DataWriter()
{
    if (nullptr != mp_rtps_publisher)
    {
        fastrtps::Domain::removePublisher(mp_rtps_publisher);
    }
}

bool DataWriter::init()
{
    PublisherAttributes attributes;
    if (m_rtps_publisher_prof.empty() ||
        (fastrtps::xmlparser::XMLP_ret::XML_ERROR ==
         fastrtps::xmlparser::XMLProfileManager::fillPublisherAttributes(m_rtps_publisher_prof, attributes)))
    {
        fastrtps::xmlparser::XMLProfileManager::getDefaultPublisherAttributes(attributes);
    }

    if (check_registered_topic(attributes.topic.getTopicDataType()))
    {
        if (!m_rtps_publisher_prof.empty())
        {
            mp_rtps_publisher = fastrtps::Domain::createPublisher(&mp_rtps_participant, m_rtps_publisher_prof, nullptr);
        }
        else
        {
            mp_rtps_publisher =
                fastrtps::Domain::createPublisher(&mp_rtps_participant, DEFAULT_XRCE_PUBLISHER_PROFILE, nullptr);
        }
    }

    if (mp_rtps_publisher == nullptr)
    {
        std::cout << "DDS ERROR: init publisher error" << std::endl;
        return false;
    }
    return true;
}

bool DataWriter::init(const std::string& xmlrep)
{
    PublisherAttributes attributes;
    if (xmlobjects::parse_publisher(xmlrep.data(), xmlrep.size(), attributes))
    {
        if (check_registered_topic(attributes.topic.getTopicDataType()))
        {
            mp_rtps_publisher = fastrtps::Domain::createPublisher(&mp_rtps_participant, attributes, nullptr);
        }
    }
    else
    {
        fastrtps::xmlparser::XMLProfileManager::getDefaultPublisherAttributes(attributes);
        if (check_registered_topic(attributes.topic.getTopicDataType()))
        {
            mp_rtps_publisher =
                fastrtps::Domain::createPublisher(&mp_rtps_participant, DEFAULT_XRCE_PUBLISHER_PROFILE, nullptr);
        }
    }

    if (mp_rtps_publisher == nullptr)
    {
        std::cout << "init publisher error" << std::endl;
        return false;
    }
    return true;
}

bool DataWriter::write(dds::xrce::WRITE_DATA_Payload_Data& write_data)
{
    if (nullptr == mp_rtps_publisher)
    {
        return false;
    }
    return mp_rtps_publisher->write(&(write_data.data().serialized_data()));
}

bool DataWriter::check_registered_topic(const std::string& topic_data_type) const
{
    // TODO(Borja) Take this method out to Topic type.
    TopicDataType* p_type = nullptr;
    if (!fastrtps::Domain::getRegisteredType(&mp_rtps_participant, topic_data_type.data(), &p_type))
    {
        std::cout << "DDS ERROR: No registered type" << std::endl;
        return false;
    }
    return true;
}

} /* namespace micrortps */
} /* namespace eprosima */
