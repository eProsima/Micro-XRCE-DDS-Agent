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

#include <uxr/agent/datawriter/DataWriter.hpp>
#include <uxr/agent/publisher/Publisher.hpp>
#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/topic/Topic.hpp>
#include <fastrtps/Domain.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../xmlobjects/xmlobjects.h"

namespace eprosima {
namespace uxr {

DataWriter::DataWriter(const dds::xrce::ObjectId& object_id,
                       const std::shared_ptr<Publisher>& publisher,
                       const std::string& profile_name)
    : XRCEObject{object_id},
      publisher_(publisher),
      rtps_publisher_(nullptr),
      rtps_publisher_prof_(profile_name)

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

bool DataWriter::init(const dds::xrce::DATAWRITER_Representation& representation, const ObjectContainer& root_objects)
{
    bool rv = false;
    fastrtps::Participant* rtps_participant = publisher_->get_participant()->get_rtps_participant();
    dds::xrce::ObjectId topic_id;
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref_rep = representation.representation().object_reference();
            rtps_publisher_ = fastrtps::Domain::createPublisher(rtps_participant, ref_rep, this);
            if (nullptr != rtps_publisher_)
            {
                const std::string& topic_data_type = rtps_publisher_->getAttributes().topic.getTopicDataType();
                if (publisher_->get_participant()->check_register_topic(topic_data_type, topic_id))
                {
                    topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_id));
                    topic_->tie_object(get_id());
                    rv = true;
                }
                else
                {
                    if (fastrtps::Domain::removePublisher(rtps_publisher_))
                    {
                        rtps_publisher_ = nullptr;
                    }
                }
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml_rep = representation.representation().xml_string_representation();
            fastrtps::PublisherAttributes attributes;
            if (xmlobjects::parse_publisher(xml_rep.data(), xml_rep.size(), attributes))
            {
                rtps_publisher_ = fastrtps::Domain::createPublisher(rtps_participant, attributes, this);
                if (nullptr != rtps_publisher_)
                {
                    if (publisher_->get_participant()->check_register_topic(attributes.topic.getTopicDataType(), topic_id))
                    {
                        topic_ = std::dynamic_pointer_cast<Topic>(root_objects.at(topic_id));
                        topic_->tie_object(get_id());
                        rv = true;
                    }
                    else
                    {
                        if (fastrtps::Domain::removePublisher(rtps_publisher_))
                        {
                            rtps_publisher_ = nullptr;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return rv;
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

bool DataWriter::matched(const dds::xrce::ObjectVariant& new_object_rep) const
{
    /* Check ObjectKind. */
    if ((get_id().at(1) & 0x0F) != new_object_rep._d())
    {
        return false;
    }

    bool parser_cond = false;
    const fastrtps::PublisherAttributes& old_attributes = rtps_publisher_->getAttributes();
    fastrtps::PublisherAttributes new_attributes;

    switch (new_object_rep.data_writer().representation()._d())
    {
        case dds::xrce::REPRESENTATION_BY_REFERENCE:
        {
            const std::string& ref_rep = new_object_rep.data_writer().representation().object_reference();
            if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
                fastrtps::xmlparser::XMLProfileManager::fillPublisherAttributes(ref_rep, new_attributes))
            {
                parser_cond = true;
            }
            break;
        }
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml_rep = new_object_rep.data_writer().representation().xml_string_representation();
            if (xmlobjects::parse_publisher(xml_rep.data(), xml_rep.size(), new_attributes))
            {
                parser_cond = true;
            }
            break;
        }
        default:
            break;
    }

    return parser_cond && (new_attributes == old_attributes);
}

} // namespace uxr
} // namespace eprosima
