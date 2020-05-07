// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/middleware/fastdds/FastDDSEntities.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include <fastrtps/attributes/all_attributes.h>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../../xmlobjects/xmlobjects.h"


namespace eprosima {
namespace uxr {

using namespace fastrtps::xmlparser;
using eprosima::fastrtps::types::ReturnCode_t;

static void set_qos_from_attributes(
        fastdds::dds::DomainParticipantQos& qos,
        const fastrtps::rtps::RTPSParticipantAttributes& attr)
{
    qos.user_data().setValue(attr.userData);
    qos.allocation() = attr.allocation;
    qos.properties() = attr.properties;
    qos.wire_protocol().prefix = attr.prefix;
    qos.wire_protocol().participant_id = attr.participantID;
    qos.wire_protocol().builtin = attr.builtin;
    qos.wire_protocol().port = attr.port;
    qos.wire_protocol().throughput_controller = attr.throughputController;
    qos.wire_protocol().default_unicast_locator_list = attr.defaultUnicastLocatorList;
    qos.wire_protocol().default_multicast_locator_list = attr.defaultMulticastLocatorList;
    qos.transport().user_transports = attr.userTransports;
    qos.transport().use_builtin_transports = attr.useBuiltinTransports;
    qos.transport().send_socket_buffer_size = attr.sendSocketBufferSize;
    qos.transport().listen_socket_buffer_size = attr.listenSocketBufferSize;
    qos.name() = attr.getName();
}

static void set_qos_from_attributes(
        fastdds::dds::TopicQos& qos,
        const fastrtps::TopicAttributes& attr)
{
    qos.history() = attr.historyQos;
    qos.resource_limits() = attr.resourceLimitsQos;
}

static void set_qos_from_attributes(
        fastdds::dds::SubscriberQos& qos,
        const fastrtps::SubscriberAttributes& attr)
{
    qos.group_data().setValue(attr.qos.m_groupData);
    qos.partition() = attr.qos.m_partition;
    qos.presentation() = attr.qos.m_presentation;
}

static void set_qos_from_attributes(
        fastdds::dds::PublisherQos& qos,
        const fastrtps::PublisherAttributes& attr)
{
    qos.group_data().setValue(attr.qos.m_groupData);
    qos.partition() = attr.qos.m_partition;
    qos.presentation() = attr.qos.m_presentation;
}

static void set_qos_from_attributes(
        fastdds::dds::DataWriterQos& qos,
        const fastrtps::PublisherAttributes& attr)
{
    qos.writer_resource_limits().matched_subscriber_allocation = attr.matched_subscriber_allocation;
    qos.properties() = attr.properties;
    qos.throughput_controller() = attr.throughputController;
    qos.endpoint().unicast_locator_list = attr.unicastLocatorList;
    qos.endpoint().multicast_locator_list = attr.multicastLocatorList;
    qos.endpoint().remote_locator_list = attr.remoteLocatorList;
    qos.endpoint().history_memory_policy = attr.historyMemoryPolicy;
    qos.endpoint().user_defined_id = attr.getUserDefinedID();
    qos.endpoint().entity_id = attr.getEntityID();
    qos.reliable_writer_qos().times = attr.times;
    qos.reliable_writer_qos().disable_positive_acks = attr.qos.m_disablePositiveACKs;
    qos.durability() = attr.qos.m_durability;
    qos.durability_service() = attr.qos.m_durabilityService;
    qos.deadline() = attr.qos.m_deadline;
    qos.latency_budget() = attr.qos.m_latencyBudget;
    qos.liveliness() = attr.qos.m_liveliness;
    qos.reliability() = attr.qos.m_reliability;
    qos.lifespan() = attr.qos.m_lifespan;
    qos.user_data().setValue(attr.qos.m_userData);
    qos.ownership() = attr.qos.m_ownership;
    qos.ownership_strength() = attr.qos.m_ownershipStrength;
    qos.destination_order() = attr.qos.m_destinationOrder;
    qos.representation() = attr.qos.representation;
    qos.publish_mode() = attr.qos.m_publishMode;
    qos.history() = attr.topic.historyQos;
    qos.resource_limits() = attr.topic.resourceLimitsQos;
}

static void set_qos_from_attributes(
        fastdds::dds::DataReaderQos& qos,
        const fastrtps::SubscriberAttributes& attr)
{
    qos.reader_resource_limits().matched_publisher_allocation = attr.matched_publisher_allocation;
    qos.properties() = attr.properties;
    qos.expects_inline_qos(attr.expectsInlineQos);
    qos.endpoint().unicast_locator_list = attr.unicastLocatorList;
    qos.endpoint().multicast_locator_list = attr.multicastLocatorList;
    qos.endpoint().remote_locator_list = attr.remoteLocatorList;
    qos.endpoint().history_memory_policy = attr.historyMemoryPolicy;
    qos.endpoint().user_defined_id = attr.getUserDefinedID();
    qos.endpoint().entity_id = attr.getEntityID();
    qos.reliable_reader_qos().times = attr.times;
    qos.reliable_reader_qos().disable_positive_ACKs = attr.qos.m_disablePositiveACKs;
    qos.durability() = attr.qos.m_durability;
    qos.durability_service() = attr.qos.m_durabilityService;
    qos.deadline() = attr.qos.m_deadline;
    qos.latency_budget() = attr.qos.m_latencyBudget;
    qos.liveliness() = attr.qos.m_liveliness;
    qos.reliability() = attr.qos.m_reliability;
    qos.lifespan() = attr.qos.m_lifespan;
    qos.user_data().setValue(attr.qos.m_userData);
    qos.ownership() = attr.qos.m_ownership;
    qos.destination_order() = attr.qos.m_destinationOrder;
    qos.type_consistency().type_consistency = attr.qos.type_consistency;
    qos.type_consistency().representation = attr.qos.representation;
    qos.time_based_filter() = attr.qos.m_timeBasedFilter;
    qos.history() = attr.topic.historyQos;
    qos.resource_limits() = attr.topic.resourceLimitsQos;
}

FastDDSParticipant::~FastDDSParticipant()
{
    factory_->delete_participant(ptr_);
}

bool FastDDSParticipant::create_by_ref(
        const std::string& ref)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        ptr_ = factory_->create_participant_with_profile(domain_id_, ref);
        rv = (nullptr != ptr_);
    }
    return rv;
}

bool FastDDSParticipant::create_by_xml(
        const std::string& xml)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastrtps::ParticipantAttributes attrs;
        if (xmlobjects::parse_participant(xml.data(), xml.size(), attrs))
        {
            fastdds::dds::DomainParticipantQos qos;
            set_qos_from_attributes(qos, attrs.rtps);
            ptr_ = factory_->create_participant(domain_id_, qos);
        }
        rv = (nullptr != ptr_);
    }
    return rv;
}

bool FastDDSParticipant::match_from_ref(
        const std::string& ref) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastrtps::ParticipantAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillParticipantAttributes(ref, attrs))
        {
            fastdds::dds::DomainParticipantQos qos;
            set_qos_from_attributes(qos, attrs.rtps);
            rv = (ptr_->get_qos() == qos);
        }
    }
    return rv;
}

bool FastDDSParticipant::match_from_xml(
        const std::string& xml) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastrtps::ParticipantAttributes attrs;
        if (xmlobjects::parse_participant(xml.data(), xml.size(), attrs))
        {
            fastdds::dds::DomainParticipantQos qos;
            set_qos_from_attributes(qos, attrs.rtps);
            rv = (ptr_->get_qos() == qos);
        }
    }
    return rv;
}


bool FastDDSParticipant::register_topic(
        const fastrtps::TopicAttributes& attrs,
        std::shared_ptr<FastDDSType>& type,
        std::shared_ptr<FastDDSTopic> topic)
{
    // TODO (#5057): allow more than one topic.
    bool rv = false;
    auto it = type_register_.find(attrs.getTopicDataType().c_str());
    if (type_register_.end() == it)
    {
        type = std::make_shared<FastDDSType>(shared_from_this());
        type->setName(attrs.getTopicDataType().c_str());
        type->m_isGetKeyDefined = (attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);

        fastdds::dds::TypeSupport m_type(type.get());
        // Is this neccesary?
        // m_type.get()->auto_fill_type_information(false);
        // m_type.get()->auto_fill_type_object(true);

        if (ReturnCode_t::RETCODE_OK == ptr_->register_type(m_type, attrs.getTopicDataType().c_str()))
        {   
            std::weak_ptr<FastDDSTopic> w_topic;
            w_topic = topic;
            topics_.emplace(type->getName(), w_topic);
            type_register_.emplace(type->getName(), type);
        }
    }
    else
    {
        type = it->second.lock();
        rv = true;
    }

    return rv;
}

bool FastDDSParticipant::unregister_topic(
        const std::string& topic_name)
{
    bool rv = false;
    if ((0 != type_register_.erase(topic_name)) && (0 != topics_.erase(topic_name)))
    {   
        rv = ReturnCode_t::RETCODE_OK == ptr_->unregister_type(topic_name);;
    }
    return rv;
}

bool FastDDSParticipant::find_topic(
        const std::string& topic_name,
        std::shared_ptr<FastDDSTopic>& topic)
{
    bool rv = false;
    auto it = topics_.find(topic_name);
    if (topics_.end() != it)
    {
        topic = it->second.lock();
        rv = true;
    }
    return rv;
}

FastDDSType::~FastDDSType()
{
    participant_->unregister_topic(getName());
}

FastDDSTopic::~FastDDSTopic()
{   
    participant_->ptr_->delete_topic(ptr_);
}

bool FastDDSTopic::create_by_ref(
        const std::string& ref,
        uint16_t topic_id)
{
    bool rv = false;
    if (nullptr == ptr_)
    {   
        fastrtps::TopicAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillTopicAttributes(ref, attrs))
        {   
            topic_id = topic_id;
            if (participant_->register_topic(attrs, type_, shared_from_this()))
            {
                rv = true;
            }
        }
    }
    return rv;
}

bool FastDDSTopic::create_by_xml(
        const std::string& xml,
        uint16_t topic_id)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastrtps::TopicAttributes attrs;
        if (xmlobjects::parse_topic(xml.data(), xml.size(), attrs))
        {   
            topic_id = topic_id;
            if (participant_->register_topic(attrs, type_, shared_from_this()))
            {
                rv = true;
            }
        }
        rv = (nullptr != ptr_);
    }
    return rv;
}

bool FastDDSTopic::match_from_ref(
        const std::string& ref) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastrtps::TopicAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillTopicAttributes(ref, attrs))
        {
            fastdds::dds::TopicQos qos;
            set_qos_from_attributes(qos, attrs);
            rv = (ptr_->get_qos() == qos);
        }
    }
    return rv;
}

bool FastDDSTopic::match_from_xml(
        const std::string& xml) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastrtps::TopicAttributes attrs;
        if (xmlobjects::parse_topic(xml.data(), xml.size(), attrs))
        {
            fastdds::dds::TopicQos qos;
            set_qos_from_attributes(qos, attrs);
            rv = (ptr_->get_qos() == qos);
        }
    }
    return rv;
}

FastDDSPublisher::~FastDDSPublisher()
{   
    participant_->ptr_->delete_publisher(ptr_);
}

bool FastDDSPublisher::create_by_xml(
        const std::string& xml)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastrtps::PublisherAttributes attrs;
        if (xmlobjects::parse_publisher(xml.data(), xml.size(), attrs))
        {   
            fastdds::dds::PublisherQos qos;
            set_qos_from_attributes(qos, attrs);
            ptr_ = participant_->ptr_->create_publisher(qos);
            rv = (nullptr != ptr_);
        }
        rv = (nullptr != ptr_);
    }
    return rv;
}

FastDDSSubscriber::~FastDDSSubscriber()
{   
    participant_->ptr_->delete_subscriber(ptr_);
}


bool FastDDSSubscriber::create_by_xml(
        const std::string& xml)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastrtps::SubscriberAttributes attrs;
        if (xmlobjects::parse_subscriber(xml.data(), xml.size(), attrs))
        {   
            fastdds::dds::SubscriberQos qos;
            set_qos_from_attributes(qos, attrs);
            ptr_ = participant_->ptr_->create_subscriber(qos);
            rv = (nullptr != ptr_);
        }
        rv = (nullptr != ptr_);
    }
    return rv;
}

FastDDSDataWriter::~FastDDSDataWriter()
{
    publisher_->ptr_->delete_datawriter(ptr_);
}

bool FastDDSDataWriter::create_by_ref(
        const std::string& ref,
        uint16_t& topic_id)
{   
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::PublisherAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(ref, attrs))
        {   
            std::shared_ptr<FastDDSTopic> topic;
            if(publisher_->participant_->find_topic(attrs.topic.getTopicDataType().c_str(), topic)){
                topic_id = topic->topic_id;
                fastdds::dds::DataWriterQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = publisher_->ptr_->create_datawriter(topic.get()->ptr_, qos);
                rv = (nullptr != ptr_);
            }
        }
    }
    return rv;
}

bool FastDDSDataWriter::create_by_xml(
        const std::string& xml,
        uint16_t& topic_id)
{   
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::PublisherAttributes attrs;
        if (xmlobjects::parse_publisher(xml.data(), xml.size(), attrs))
        {
            std::shared_ptr<FastDDSTopic> topic;
            if(publisher_->participant_->find_topic(attrs.topic.getTopicDataType().c_str(), topic)){
                topic_id = topic->topic_id;
                fastdds::dds::DataWriterQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = publisher_->ptr_->create_datawriter(topic.get()->ptr_, qos);
                rv = (nullptr != ptr_);
            }
        }
    }
    return rv;
}

bool FastDDSDataWriter::match_from_ref(
        const std::string& ref) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastrtps::PublisherAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(ref, attrs))
        {
            fastdds::dds::DataWriterQos qos;
            set_qos_from_attributes(qos, attrs);
            rv = (ptr_->get_qos() == qos);
        }
    }
    return rv;
}

bool FastDDSDataWriter::match_from_xml(
        const std::string& xml) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastrtps::PublisherAttributes attrs;
        if (xmlobjects::parse_publisher(xml.data(), xml.size(), attrs))
        {
            fastdds::dds::DataWriterQos qos;
            set_qos_from_attributes(qos, attrs);
            rv = (ptr_->get_qos() == qos);
        }
    }
    return rv;
}

bool FastDDSDataWriter::write(const std::vector<uint8_t>& data)
{
    return ptr_->write(&const_cast<std::vector<uint8_t>&>(data));
}

} // namespace uxr
} // namespace eprosima