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
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>
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

/**********************************************************************************************************************
 * FastDDSParticipant
 **********************************************************************************************************************/
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

bool FastDDSParticipant::register_type(
        const std::shared_ptr<FastDDSType>& type)
{
    fastdds::dds::TypeSupport m_type(type.get());
    return ReturnCode_t::RETCODE_OK == ptr_->register_type(m_type, type->getName())
        && type_register_.emplace(type->getName(), type).second;
}

bool FastDDSParticipant::unregister_type(
        const std::string& type_name)
{
    return (1 == type_register_.erase(type_name))
        && ReturnCode_t::RETCODE_OK == ptr_->unregister_type(type_name);
}

std::shared_ptr<FastDDSType> FastDDSParticipant::find_type(
        const std::string& type_name) const
{
    std::shared_ptr<FastDDSType> type;
    auto it = type_register_.find(type_name);
    if (it != type_register_.end())
    {
        type = it->second.lock();
    }
    return type;
}

bool FastDDSParticipant::register_topic(
            const std::shared_ptr<FastDDSTopic>& topic)
{
    return topic_register_.emplace(topic->get_name(), topic).second;
}

bool FastDDSParticipant::unregister_topic(
        const std::string& topic_name)
{
    return (1 == topic_register_.erase(topic_name));
}

std::shared_ptr<FastDDSTopic> FastDDSParticipant::find_topic(
        const std::string& topic_name) const
{
    std::shared_ptr<FastDDSTopic> topic;
    auto it = topic_register_.find(topic_name);
    if (it != topic_register_.end())
    {
        topic = it->second.lock();
    }
    return topic;
}

/**********************************************************************************************************************
 * FastDDSTopic
 **********************************************************************************************************************/
FastDDSType::~FastDDSType()
{
    participant_->unregister_type(getName());
}

FastDDSTopic::~FastDDSTopic()
{   
    if (ptr_)
    {
        participant_->unregister_topic(ptr_->get_name());
        // participant_->ptr_->delete_topic(ptr_);
    }
}

bool FastDDSTopic::create_by_ref(const std::string& ref)
{
    bool rv = false; 
    fastrtps::TopicAttributes attrs;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillTopicAttributes(ref, attrs))
    {   
        rv = create_by_attributes(attrs);
    }
    return rv;
}

bool FastDDSTopic::create_by_xml(const std::string& xml)
{
    bool rv = false;
    fastrtps::TopicAttributes attrs;
    if (xmlobjects::parse_topic(xml.data(), xml.size(), attrs))
    {   
        rv = create_by_attributes(attrs);
    }
    return rv;
}

bool FastDDSTopic::create_by_attributes(const fastrtps::TopicAttributes& attrs)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastdds::dds::TopicQos qos;
        set_qos_from_attributes(qos, attrs);
        
        ptr_ = participant_->ptr_->create_topic(attrs.getTopicName().to_string(), 
                attrs.getTopicDataType().to_string(), qos);

        rv = (nullptr != ptr_);
    }
    return rv;
}

bool FastDDSTopic::create_by_name_type(const std::string& name, 
    const std::shared_ptr<FastDDSType>& type)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastdds::dds::TopicQos qos;

        ptr_ = participant_->ptr_->create_topic(name, type->getName(), qos);

        rv = (nullptr != ptr_);

        if (rv)
        {
            type_ = type;
        }
        
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

bool FastDDSTopic::match(const fastrtps::TopicAttributes& attrs) const
{
    return (attrs.getTopicName() == ptr_->get_name())
        && (0 == std::strcmp(type_->getName(), attrs.getTopicDataType().c_str()))
        && (type_->m_isGetKeyDefined == (attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY));
}

/**********************************************************************************************************************
 * FastDDSPublisher
 **********************************************************************************************************************/
FastDDSPublisher::~FastDDSPublisher()
{   
    if (ptr_)
    {
        participant_->ptr_->delete_publisher(ptr_);
    }
}

bool FastDDSPublisher::create_by_xml(
        const std::string& xml)
{
    bool rv = false;
    if (nullptr == ptr_)
    {   
        fastdds::dds::PublisherQos qos;
        fastrtps::PublisherAttributes attrs;
        if (0 != xml.size() && xmlobjects::parse_publisher(xml.data(), xml.size(), attrs))
        {   
            set_qos_from_attributes(qos, attrs);
        }
        ptr_ = participant_->ptr_->create_publisher(qos);
        rv = (nullptr != ptr_);    }
    return rv;
}


/**********************************************************************************************************************
 * FastDDSSubscriber
 **********************************************************************************************************************/
FastDDSSubscriber::~FastDDSSubscriber()
{   
    if (ptr_)
    {
        participant_->ptr_->delete_subscriber(ptr_);
    }
}


bool FastDDSSubscriber::create_by_xml(
        const std::string& xml)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastrtps::SubscriberAttributes attrs;
        fastdds::dds::SubscriberQos qos;
        if (0 != xml.size() && xmlobjects::parse_subscriber(xml.data(), xml.size(), attrs))
        {   
            set_qos_from_attributes(qos, attrs);
        }
        ptr_ = participant_->ptr_->create_subscriber(qos);
        rv = (nullptr != ptr_);    
    }
    return rv;
}

/**********************************************************************************************************************
 * FastDDSDataWriter
 **********************************************************************************************************************/
FastDDSDataWriter::~FastDDSDataWriter()
{
    if (ptr_)
    {
        publisher_->ptr_->delete_datawriter(ptr_);
    }  
}

bool FastDDSDataWriter::create_by_ref(const std::string& ref)
{   
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::PublisherAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(ref, attrs))
        {   
            topic_ = publisher_->participant_->find_topic(attrs.topic.getTopicDataType().c_str());
            if(topic_){
                fastdds::dds::DataWriterQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = publisher_->ptr_->create_datawriter(topic_.get()->ptr_, qos);
                rv = (nullptr != ptr_) && bool(topic_);
            }
        }
    }
    return rv;
}

bool FastDDSDataWriter::create_by_xml(const std::string& xml)
{   
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::PublisherAttributes attrs;
        if (xmlobjects::parse_publisher(xml.data(), xml.size(), attrs))
        {   
            topic_ = publisher_->participant_->find_topic(attrs.topic.getTopicDataType().c_str());
            if(topic_){
                fastdds::dds::DataWriterQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = publisher_->ptr_->create_datawriter(topic_.get()->ptr_, qos);
                rv = (nullptr != ptr_) && bool(topic_);
            }
        }
    }
    return rv;
}

bool FastDDSDataWriter::match(const fastrtps::PublisherAttributes& attrs) const
{   
    fastdds::dds::DataWriterQos qos;
    set_qos_from_attributes(qos, attrs);
    return (ptr_->get_qos() == qos);
}


bool FastDDSDataWriter::write(const std::vector<uint8_t>& data)
{
    return ptr_->write(&const_cast<std::vector<uint8_t>&>(data));
}

/**********************************************************************************************************************
 * FastDDSDataReader
 **********************************************************************************************************************/
FastDDSDataReader::~FastDDSDataReader()
{
    if (ptr_)
    {
        subscriber_->ptr_->delete_datareader(ptr_);
    }  
}

bool FastDDSDataReader::create_by_ref(const std::string& ref)
{   
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::SubscriberAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(ref, attrs))
        {   
            topic_ = subscriber_->participant_->find_topic(attrs.topic.getTopicDataType().c_str());
            if(topic_){
                fastdds::dds::DataReaderQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = subscriber_->ptr_->create_datareader(topic_.get()->ptr_, qos);
                rv = (nullptr != ptr_);
            }
        }
    }
    return rv;
}

bool FastDDSDataReader::create_by_xml(const std::string& xml)
{   
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::SubscriberAttributes attrs;
        if (xmlobjects::parse_subscriber(xml.data(), xml.size(), attrs))
        {   
            topic_ = subscriber_->participant_->find_topic(attrs.topic.getTopicDataType().c_str());
            if(topic_){
                fastdds::dds::DataReaderQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = subscriber_->ptr_->create_datareader(topic_.get()->ptr_, qos);
                rv = (nullptr != ptr_);
            }
        }
    }
    return rv;
}

bool FastDDSDataReader::match_from_ref(
        const std::string& ref) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastrtps::SubscriberAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(ref, attrs))
        {
            fastdds::dds::DataReaderQos qos;
            set_qos_from_attributes(qos, attrs);
            rv = (ptr_->get_qos() == qos);
        }
    }
    return rv;
}

bool FastDDSDataReader::match_from_xml(
        const std::string& xml) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastrtps::SubscriberAttributes attrs;
        if (xmlobjects::parse_subscriber(xml.data(), xml.size(), attrs))
        {
            fastdds::dds::DataReaderQos qos;
            set_qos_from_attributes(qos, attrs);
            rv = (ptr_->get_qos() == qos);
        }
    }
    return rv;
}

bool FastDDSDataReader::read(
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{   

    bool rv = false;

    fastrtps::Duration_t d((long double) timeout.count()/1000.0);
    
    if(ptr_->wait_for_unread_message(d)){
        fastdds::dds::SampleInfo info;
        rv = ReturnCode_t::RETCODE_OK == ptr_->take_next_sample(&data, &info);
    }

    return rv;
}

/**********************************************************************************************************************
 * FastDDSRequester
 **********************************************************************************************************************/
FastDDSRequester::~FastDDSRequester()
{
    if (publisher_ptr_)
    {
        participant_->ptr_->delete_publisher(publisher_ptr_);
    }
    if (subscriber_ptr_)
    {
        participant_->ptr_->delete_subscriber(subscriber_ptr_);
    }
}

bool FastDDSRequester::create_by_attributes(
        const fastrtps::RequesterAttributes& attrs)
{
    bool rv = false;
    
    fastdds::dds::PublisherQos qos_publisher;
    set_qos_from_attributes(qos_publisher, attrs.publisher);
    publisher_ptr_ = participant_->ptr_->create_publisher(qos_publisher);

    fastdds::dds::DataWriterQos qos_datawriter;
    set_qos_from_attributes(qos_datawriter, attrs.publisher);
    datawriter_ptr_ = publisher_ptr_->create_datawriter(request_topic_.get()->ptr_, qos_datawriter);

    fastdds::dds::SubscriberQos qos_subscriber;
    set_qos_from_attributes(qos_subscriber, attrs.subscriber);
    subscriber_ptr_ = participant_->ptr_->create_subscriber(qos_subscriber);

    fastdds::dds::DataReaderQos qos_datareader;
    set_qos_from_attributes(qos_datareader, attrs.subscriber);
    datareader_ptr_ = subscriber_ptr_->create_datareader(reply_topic_.get()->ptr_, qos_datareader);

    rv = (nullptr != publisher_ptr_) && (nullptr != datawriter_ptr_) &&
         (nullptr != subscriber_ptr_) && (nullptr != datareader_ptr_);

    if (rv)
    {
        std::copy(
            std::begin(datawriter_ptr_->guid().guidPrefix.value),
            std::end(datawriter_ptr_->guid().guidPrefix.value),
            publisher_id_.guidPrefix().begin());
        std::copy(
            std::begin(datawriter_ptr_->guid().entityId.value),
            std::begin(datawriter_ptr_->guid().entityId.value) + 3,
            publisher_id_.entityId().entityKey().begin());
        publisher_id_.entityId().entityKind() = datawriter_ptr_->guid().entityId.value[3];
    }

    return rv;
}

bool FastDDSRequester::match_from_ref(const std::string& ref) const
{
    bool rv = false;
    fastrtps::RequesterAttributes new_attributes;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillRequesterAttributes(ref, new_attributes))
    {
        rv = match(new_attributes);
    }
    return rv;
}

bool FastDDSRequester::match_from_xml(const std::string& xml) const
{
    bool rv = false;
    fastrtps::RequesterAttributes new_attributes;
    if (xmlobjects::parse_requester(xml.data(), xml.size(), new_attributes))
    {
        rv = match(new_attributes);
    }
    return rv;
}

bool FastDDSRequester::match(const fastrtps::RequesterAttributes& attrs) const
{
    fastdds::dds::PublisherQos qos_publisher;
    set_qos_from_attributes(qos_publisher, attrs.publisher);
    fastdds::dds::SubscriberQos qos_subscriber;
    set_qos_from_attributes(qos_subscriber, attrs.subscriber);

    return reply_topic_->match(attrs.publisher.topic)
        && request_topic_->match(attrs.subscriber.topic)
        && publisher_ptr_->get_qos() == qos_publisher
        && subscriber_ptr_->get_qos() == qos_subscriber;
}

bool FastDDSRequester::write(
        uint32_t sequence_number,
        const std::vector<uint8_t>& data)
{
    bool rv = true;
    try
    {
        fastrtps::rtps::WriteParams wparams;
        rv = datawriter_ptr_->write(&const_cast<std::vector<uint8_t>&>(data), wparams);
        if (rv)
        {
            int64_t sequence = (int64_t)wparams.sample_identity().sequence_number().high << 32;
            sequence += wparams.sample_identity().sequence_number().low;
            sequence_to_sequence_.emplace(sequence, sequence_number);
        }
    }
    catch(const std::exception&)
    {
        rv = false;
    }

    return rv;
}


bool FastDDSRequester::read(
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;

    fastrtps::Duration_t d((long double) timeout.count()/1000.0);
    fastdds::dds::SampleInfo info;
    
    if(datareader_ptr_->wait_for_unread_message(d)){
        rv = ReturnCode_t::RETCODE_OK == datareader_ptr_->take_next_sample(&data, &info);
    }

    if (rv)
    {
        try
        {
            if (info.related_sample_identity.writer_guid() == datawriter_ptr_->guid())
            {
                int64_t sequence = (int64_t)info.related_sample_identity.sequence_number().high << 32;
                sequence += info.related_sample_identity.sequence_number().low;
                auto it = sequence_to_sequence_.find(sequence);
                if (it != sequence_to_sequence_.end())
                {
                    sequence_number = it->second;
                    sequence_to_sequence_.erase(it);
                }
                else
                {
                    rv = false;
                }
            }
            else
            {
                rv = false;
            }
        }
        catch(const std::exception&)
        {
            rv = false;
        }
    }

    return rv;
}

/**********************************************************************************************************************
 * FastDDSReplier
 **********************************************************************************************************************/
FastDDSReplier::~FastDDSReplier()
{
    if (publisher_ptr_)
    {
        participant_->ptr_->delete_publisher(publisher_ptr_);
    }
    if (subscriber_ptr_)
    {
        participant_->ptr_->delete_subscriber(subscriber_ptr_);
    }
}

bool FastDDSReplier::create_by_attributes(
        const fastrtps::ReplierAttributes& attrs)
{
    bool rv = false;

    fastdds::dds::PublisherQos qos_publisher;
    set_qos_from_attributes(qos_publisher, attrs.publisher);
    publisher_ptr_ = participant_->ptr_->create_publisher(qos_publisher);

    fastdds::dds::DataWriterQos qos_datawriter;
    set_qos_from_attributes(qos_datawriter, attrs.publisher);
    datawriter_ptr_ = publisher_ptr_->create_datawriter(reply_topic_.get()->ptr_, qos_datawriter);

    fastdds::dds::SubscriberQos qos_subscriber;
    set_qos_from_attributes(qos_subscriber, attrs.subscriber);
    subscriber_ptr_ = participant_->ptr_->create_subscriber(qos_subscriber);

    fastdds::dds::DataReaderQos qos_datareader;
    set_qos_from_attributes(qos_datareader, attrs.subscriber);
    datareader_ptr_ = subscriber_ptr_->create_datareader(request_topic_.get()->ptr_, qos_datareader);

    rv = (nullptr != publisher_ptr_) && (nullptr != datawriter_ptr_) &&
         (nullptr != subscriber_ptr_) && (nullptr != datareader_ptr_);

    return rv;
}


bool FastDDSReplier::match_from_ref(const std::string& ref) const
{
    bool rv = false;
    fastrtps::ReplierAttributes new_attributes;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillReplierAttributes(ref, new_attributes))
    {
        rv = match(new_attributes);
    }
    return rv;
}

bool FastDDSReplier::match_from_xml(const std::string& xml) const
{
    bool rv = false;
    fastrtps::ReplierAttributes new_attributes;
    if (xmlobjects::parse_replier(xml.data(), xml.size(), new_attributes))
    {
        rv = match(new_attributes);
    }
    return rv;
}

bool FastDDSReplier::match(const fastrtps::ReplierAttributes& attrs) const
{   
    fastdds::dds::PublisherQos qos_publisher;
    set_qos_from_attributes(qos_publisher, attrs.publisher);
    fastdds::dds::SubscriberQos qos_subscriber;
    set_qos_from_attributes(qos_subscriber, attrs.subscriber);

    return reply_topic_->match(attrs.publisher.topic)
        && request_topic_->match(attrs.subscriber.topic)
        && publisher_ptr_->get_qos() == qos_publisher
        && subscriber_ptr_->get_qos() == qos_subscriber;
}

void FastDDSReplier::transport_sample_identity(
        const dds::SampleIdentity& dds_identity,
        fastrtps::rtps::SampleIdentity& fast_identity)
{
    std::copy(
        dds_identity.writer_guid().guidPrefix().begin(),
        dds_identity.writer_guid().guidPrefix().end(),
        std::begin(fast_identity.writer_guid().guidPrefix.value));
    std::copy(
        dds_identity.writer_guid().entityId().entityKey().begin(),
        dds_identity.writer_guid().entityId().entityKey().end(),
        std::begin(fast_identity.writer_guid().entityId.value));
    fast_identity.writer_guid().entityId.value[3] = dds_identity.writer_guid().entityId().entityKind();

    fast_identity.sequence_number().high = dds_identity.sequence_number().high();
    fast_identity.sequence_number().low = dds_identity.sequence_number().low();
}

bool FastDDSReplier::write(
        const std::vector<uint8_t>& data)
{
    fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(const_cast<uint8_t*>(data.data())), data.size()};
    fastcdr::Cdr deserializer(fastbuffer);

    dds::SampleIdentity sample_identity;
    sample_identity.deserialize(deserializer);

    fastrtps::rtps::WriteParams wparams;
    transport_sample_identity(sample_identity, wparams.related_sample_identity());

    std::vector<uint8_t> output_data(data.size() - deserializer.getSerializedDataLength());
    deserializer.deserializeArray(output_data.data(), output_data.size());

    return datawriter_ptr_->write(&const_cast<std::vector<uint8_t>&>(output_data), wparams);
}

void FastDDSReplier::transform_sample_identity(
        const fastrtps::rtps::SampleIdentity& fast_identity,
        dds::SampleIdentity& dds_identity)
{
    std::copy(
        std::begin(fast_identity.writer_guid().guidPrefix.value),
        std::end(fast_identity.writer_guid().guidPrefix.value),
        dds_identity.writer_guid().guidPrefix().begin());
    std::copy(
        std::begin(fast_identity.writer_guid().entityId.value),
        std::begin(fast_identity.writer_guid().entityId.value) + 3,
        dds_identity.writer_guid().entityId().entityKey().begin());
    dds_identity.writer_guid().entityId().entityKind() = fast_identity.writer_guid().entityId.value[3];

    dds_identity.sequence_number().high() = fast_identity.sequence_number().high;
    dds_identity.sequence_number().low() = fast_identity.sequence_number().low;
}

bool FastDDSReplier::read(
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    std::vector<uint8_t> temp_data;

    bool rv = false;

    fastrtps::Duration_t d((long double) timeout.count()/1000.0);
    fastdds::dds::SampleInfo info;
    
    if(datareader_ptr_->wait_for_unread_message(d)){
        rv = ReturnCode_t::RETCODE_OK == datareader_ptr_->take_next_sample(&temp_data, &info);
    }

    if (rv)
    {
        dds::SampleIdentity sample_identity;
        transform_sample_identity(info.sample_identity, sample_identity);

        data.clear();
        data.resize(sample_identity.getCdrSerializedSize() + temp_data.size());

        fastcdr::FastBuffer fastbuffer{reinterpret_cast<char*>(data.data()), data.size()};
        fastcdr::Cdr serializer(fastbuffer);

        try
        {
            sample_identity.serialize(serializer);
            serializer.serializeArray(temp_data.data(), temp_data.size());
        }
        catch(const std::exception&)
        {
            rv = false;
        }
    }

    return rv;
}

} // namespace uxr
} // namespace eprosima