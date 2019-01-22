// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/middleware/FastEntities.hpp>
#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/subscriber/SampleInfo.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../xmlobjects/xmlobjects.h"

namespace eprosima {
namespace uxr {

FastParticipant::~FastParticipant()
{
    fastrtps::Domain::removeParticipant(ptr_);
}

bool FastParticipant::create_by_ref(const std::string& ref)
{
    ptr_ = fastrtps::Domain::createParticipant(ref, this);
    return (nullptr != ptr_);
}

bool FastParticipant::create_by_attributes(fastrtps::ParticipantAttributes& attrs)
{
    ptr_ = fastrtps::Domain::createParticipant(attrs, this);
    return (nullptr != ptr_);
}

bool FastParticipant::match_from_ref(const std::string& ref)
{
    bool rv = false;
    fastrtps::ParticipantAttributes new_attributes;
    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillParticipantAttributes(ref, new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

bool FastParticipant::match_from_xml(const std::string& xml)
{
    bool rv = false;
    fastrtps::ParticipantAttributes new_attributes;
    if (xmlobjects::parse_participant(xml.data(), xml.size(), new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

void FastParticipant::onParticipantDiscovery(fastrtps::Participant*, fastrtps::rtps::ParticipantDiscoveryInfo&& info)
{
    if (info.status == eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
    {
        std::cout << "RTPS Participant matched " << info.info.m_guid << std::endl;
    }
    else
    {
        std::cout << "RTPS Participant unmatched " << info.info.m_guid << std::endl;
    }
}

FastTopic::~FastTopic()
{
    fastrtps::Domain::unregisterType(participant_->get_ptr(), getName());
}

bool FastTopic::create_by_attributes(const fastrtps::TopicAttributes& attrs, FastParticipant* participant)
{
    bool rv = false;
    setName(attrs.getTopicDataType().data());
    m_isGetKeyDefined = (attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
    if (fastrtps::Domain::registerType(participant->get_ptr(), this))
    {
        participant_ = participant;
        rv = true;
    }
    return rv;
}

bool FastTopic::match_from_ref(const std::string& ref)
{
    bool rv = false;
    fastrtps::TopicAttributes new_attributes;
    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillTopicAttributes(ref, new_attributes))
    {
        rv = (0 == std::strcmp(getName(), new_attributes.getTopicDataType().data())) &&
             (m_isGetKeyDefined == (new_attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY));
    }
    return rv;
}

bool FastTopic::match_from_xml(const std::string& xml)
{
    bool rv = false;
    fastrtps::TopicAttributes new_attributes;
    if (xmlobjects::parse_topic(xml.data(), xml.size(), new_attributes))
    {
        rv = (0 == std::strcmp(getName(), new_attributes.getTopicDataType().data())) &&
             (m_isGetKeyDefined == (new_attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY));
    }
    return rv;
}

FastDataWriter::~FastDataWriter()
{
    fastrtps::Domain::removePublisher(ptr_);
}

bool FastDataWriter::create_by_ref(const std::string& ref, const FastParticipant* participant, std::string& topic_name)
{
    ptr_ = fastrtps::Domain::createPublisher(participant->get_ptr(), ref, this);
    if (nullptr != ptr_)
    {
        topic_name = ptr_->getAttributes().topic.getTopicDataType();
    }
    return (nullptr != ptr_);
}

bool FastDataWriter::create_by_attributes(PublisherAttributes& attrs,
                                          const FastParticipant* participant,
                                          std::string& topic_name)
{
    ptr_ = fastrtps::Domain::createPublisher(participant->get_ptr(), attrs, this);
    if (nullptr != ptr_)
    {
        topic_name = ptr_->getAttributes().topic.getTopicDataType();
    }
    return (nullptr != ptr_);
}

bool FastDataWriter::match_from_ref(const std::string& ref)
{
    bool rv = false;
    fastrtps::PublisherAttributes new_attributes;
    if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
        fastrtps::xmlparser::XMLProfileManager::fillPublisherAttributes(ref, new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

bool FastDataWriter::match_from_xml(const std::string& xml)
{
    bool rv = false;
    fastrtps::PublisherAttributes new_attributes;
    if (xmlobjects::parse_publisher(xml.data(), xml.size(), new_attributes))
    {
        rv = (new_attributes == ptr_->getAttributes());
    }
    return rv;
}

bool FastDataWriter::write(std::vector<uint8_t>& data)
{
    return ptr_->write(&data);
}

void FastDataWriter::onPublicationMatched(fastrtps::Publisher*, fastrtps::rtps::MatchingInfo& info)
{
    if (info.status == fastrtps::rtps::MATCHED_MATCHING)
    {
        std::cout << "RTPS Subscriber matched " << info.remoteEndpointGuid << std::endl;
    }
    else
    {
        std::cout << "RTPS Subscriber unmatched " << info.remoteEndpointGuid << std::endl;
    }
}

FastDataReader::~FastDataReader()
{
    fastrtps::Domain::removeSubscriber(ptr_);
}

bool FastDataReader::create_by_ref(const std::string& ref,
                                   const FastParticipant* participant,
                                   std::string& topic_name,
                                   OnNewData on_new_data_cb)
{
    ptr_ = fastrtps::Domain::createSubscriber(participant->get_ptr(), ref, this);
    if (nullptr != ptr_)
    {
        topic_name = ptr_->getAttributes().topic.getTopicDataType();
        on_new_data_cb_ = on_new_data_cb;
    }
    return (nullptr != ptr_);
}

bool FastDataReader::create_by_attributes(SubscriberAttributes& attrs,
                                          const FastParticipant* participant,
                                          std::string& topic_name,
                                          OnNewData on_new_data_cb)
{
    ptr_ = fastrtps::Domain::createSubscriber(participant->get_ptr(), attrs, this);
    if (nullptr != ptr_)
    {
        topic_name = ptr_->getAttributes().topic.getTopicDataType();
        on_new_data_cb_ = on_new_data_cb;
    }
    return (nullptr != ptr_);
}

bool FastDataReader::read(std::vector<uint8_t>& data)
{
    bool rv = false;
    if (ptr_->getUnreadCount() != 0)
    {
        fastrtps::SampleInfo_t info;
        rv = ptr_->takeNextData(&data, &info);
    }
    return rv;
}

void FastDataReader::onSubscriptionMatched(fastrtps::Subscriber*, fastrtps::rtps::MatchingInfo& info)
{
    if (info.status == fastrtps::rtps::MATCHED_MATCHING)
    {
        std::cout << "RTPS Publisher matched " << info.remoteEndpointGuid << std::endl;
    }
    else
    {
        std::cout << "RTPS Publisher unmatched " << info.remoteEndpointGuid << std::endl;
    }
}

void FastDataReader::onNewDataMessage(fastrtps::Subscriber *)
{
    on_new_data_cb_();
}

} // namespace uxr
} // namespace eprosima
