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

#include <micrortps/agent/datawriter/DataWriter.hpp>
#include <micrortps/agent/publisher/Publisher.hpp>
#include <micrortps/agent/participant/Participant.hpp>
#include <micrortps/agent/topic/Topic.hpp>
#include <fastrtps/Domain.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../xmlobjects/xmlobjects.h"

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"
#define DEFAULT_XRCE_PUBLISHER_PROFILE "default_xrce_publisher_profile"

namespace eprosima {
namespace micrortps {

DataWriter::DataWriter(const dds::xrce::ObjectId& object_id,
                       const std::shared_ptr<Publisher>& publisher,
                       const std::string& profile_name)
    : XRCEObject{object_id},
      publisher_(publisher),
      rtps_publisher_(nullptr),
      rtps_publisher_prof_(profile_name),
      topic_type_(false)

{
    publisher_->tie_object(object_id);
}

DataWriter::~DataWriter()
{
    if (nullptr != rtps_publisher_)
    {
        fastrtps::Domain::removePublisher(rtps_publisher_);
    }
    publisher_->untie_object(get_id());
    if (topic_)
    {
        topic_->untie_object(get_id());
    }
}

bool DataWriter::init(const ObjectContainer& root_objects)
{
    PublisherAttributes attributes;
    if (rtps_publisher_prof_.empty() ||
        (fastrtps::xmlparser::XMLP_ret::XML_ERROR ==
         fastrtps::xmlparser::XMLProfileManager::fillPublisherAttributes(rtps_publisher_prof_, attributes)))
    {
        fastrtps::xmlparser::XMLProfileManager::getDefaultPublisherAttributes(attributes);
    }

    dds::xrce::ObjectId topic_id;
    if (publisher_->get_participant()->check_register_topic(attributes.topic.getTopicDataType(), topic_id))
    {
        topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_id));
        topic_->tie_object(get_id());
        fastrtps::Participant* rtps_participant = publisher_->get_participant()->get_rtps_participant();
        if (!rtps_publisher_prof_.empty())
        {
            rtps_publisher_ = fastrtps::Domain::createPublisher(rtps_participant, rtps_publisher_prof_, this);
        }
        else
        {
            rtps_publisher_ = fastrtps::Domain::createPublisher(rtps_participant, DEFAULT_XRCE_PUBLISHER_PROFILE, this);
        }
    }

    if (rtps_publisher_ == nullptr)
    {
        std::cout << "DDS ERROR: init publisher error" << std::endl;
        return false;
    }
    return true;
}

bool DataWriter::init_by_ref(const std::string& ref_rep, const ObjectContainer& root_objects)
{
    bool rv = false;
    fastrtps::Participant* rtps_participant = publisher_->get_participant()->get_rtps_participant();

    rtps_publisher_ = fastrtps::Domain::createPublisher(rtps_participant, ref_rep, this);
    if (nullptr != rtps_publisher_)
    {
        dds::xrce::ObjectId topic_id;
        const std::string& topic_data_type = rtps_publisher_->getAttributes().topic.getTopicDataType();
        if (publisher_->get_participant()->check_register_topic(topic_data_type, topic_id))
        {
            topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_id));
            topic_->tie_object(get_id());
            rv = true;
        }
        else
        {
            fastrtps::Domain::removePublisher(rtps_publisher_);
        }
    }

    return rv;
}

bool DataWriter::init_by_xml(const std::string& xml_rep, const ObjectContainer& root_objects)
{
    PublisherAttributes attributes;
    fastrtps::Participant* rtps_participant = publisher_->get_participant()->get_rtps_participant();
    if (xmlobjects::parse_publisher(xml_rep.data(), xml_rep.size(), attributes))
    {
        dds::xrce::ObjectId topic_id;
        if (publisher_->get_participant()->check_register_topic(attributes.topic.getTopicDataType(), topic_id))
        {
            topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_id));
            topic_->tie_object(get_id());
            rtps_publisher_ = fastrtps::Domain::createPublisher(rtps_participant, attributes, this);
        }
    }
    else
    {
        dds::xrce::ObjectId topic_id;
        fastrtps::xmlparser::XMLProfileManager::getDefaultPublisherAttributes(attributes);
        if (publisher_->get_participant()->check_register_topic(attributes.topic.getTopicDataType(), topic_id))
        {
            topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_id));
            topic_->tie_object(get_id());
            rtps_publisher_ =
                fastrtps::Domain::createPublisher(rtps_participant, DEFAULT_XRCE_PUBLISHER_PROFILE, this);
        }
    }

    if (nullptr == rtps_publisher_)
    {
        std::cout << "init publisher error" << std::endl;
        return false;
    }
    return true;
}

const dds::xrce::ResultStatus& DataWriter::write(dds::xrce::DataRepresentation& data)
{
    result_status_.status(dds::xrce::STATUS_OK);
    result_status_.implementation_status(0x00);

    switch (data._d())
    {
        case dds::xrce::FORMAT_DATA:
            if (rtps_publisher_->write(&(data.data().serialized_data())))
            {
                result_status_.status(dds::xrce::STATUS_ERR_DDS_ERROR);
            }
            break;
        case dds::xrce::FORMAT_SAMPLE:
            break;
        case dds::xrce::FORMAT_DATA_SEQ:
            break;
        case dds::xrce::FORMAT_SAMPLE_SEQ:
            break;
        case dds::xrce::FORMAT_PACKED_SAMPLES:
            break;
        default:
            result_status_.status(dds::xrce::STATUS_ERR_INCOMPATIBLE);
            break;
    }

    return result_status_;
}

bool DataWriter::write(dds::xrce::WRITE_DATA_Payload_Data& write_data)
{
    if (nullptr == rtps_publisher_)
    {
        return false;
    }
    return rtps_publisher_->write(&(write_data.data().serialized_data()));
}

void DataWriter::onPublicationMatched(fastrtps::Publisher*, fastrtps::rtps::MatchingInfo& info)
{
    if (info.status == rtps::MATCHED_MATCHING)
    {
        std::cout << "RTPS Subscriber matched " << info.remoteEndpointGuid << std::endl;
    }
    else
    {
        std::cout << "RTPS Subscriber unmatched " << info.remoteEndpointGuid << std::endl;
    }
}

} // namespace micrortps
} // namespace eprosima
