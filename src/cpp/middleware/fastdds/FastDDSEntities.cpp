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
#include <fastrtps/xmlparser/XMLProfileManager.h>
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
            rv = true;
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
        rv = ReturnCode_t::RETCODE_OK == ptr_->unregister_type(topic_name);
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
    fastrtps::TopicAttributes attrs;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillTopicAttributes(ref, attrs))
    {   
        rv = create_by_attributes(attrs, topic_id);
    }
    return rv;
}

bool FastDDSTopic::create_by_xml(
        const std::string& xml,
        uint16_t topic_id)
{
    bool rv = false;
    fastrtps::TopicAttributes attrs;
    if (xmlobjects::parse_topic(xml.data(), xml.size(), attrs))
    {   
        rv = create_by_attributes(attrs, topic_id);
    }
    return rv;
}

bool FastDDSTopic::create_by_attributes(
        const fastrtps::TopicAttributes& attrs,
        uint16_t topic_id)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        topic_id_ = topic_id;
        if (participant_->register_topic(attrs, type_, shared_from_this()))
        {   
            fastdds::dds::TopicQos qos;
            set_qos_from_attributes(qos, attrs);
            
            ptr_ = participant_->ptr_->create_topic(attrs.getTopicName().to_string(), 
                    attrs.getTopicDataType().to_string(), qos);

            rv = (nullptr != ptr_);
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

FastDDSDataWriter::~FastDDSDataWriter()
{
    publisher_->ptr_->delete_datawriter(ptr_);
}

bool FastDDSDataWriter::create_by_ref(const std::string& ref)
{   
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::PublisherAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(ref, attrs))
        {   
            std::shared_ptr<FastDDSTopic> topic;
            if(publisher_->participant_->find_topic(attrs.topic.getTopicDataType().c_str(), topic)){
                fastdds::dds::DataWriterQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = publisher_->ptr_->create_datawriter(topic.get()->ptr_, qos);
                rv = (nullptr != ptr_);
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
            std::shared_ptr<FastDDSTopic> topic;
            if(publisher_->participant_->find_topic(attrs.topic.getTopicDataType().c_str(), topic)){
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

FastDDSDataReader::~FastDDSDataReader()
{
    // TODO
}

bool FastDDSDataReader::create_by_ref(const std::string& ref)
{   
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::SubscriberAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(ref, attrs))
        {   
            std::shared_ptr<FastDDSTopic> topic;
            if(subscriber_->participant_->find_topic(attrs.topic.getTopicDataType().c_str(), topic)){
                fastdds::dds::DataReaderQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = subscriber_->ptr_->create_datareader(topic.get()->ptr_, qos);
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
            std::shared_ptr<FastDDSTopic> topic;
            if(subscriber_->participant_->find_topic(attrs.topic.getTopicDataType().c_str(), topic)){
                fastdds::dds::DataReaderQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = subscriber_->ptr_->create_datareader(topic.get()->ptr_, qos);
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

FastDDSRequester::~FastDDSRequester()
{
    // TODO
}

bool FastDDSRequester::create_by_xml(
        const std::string& xml)
{
    bool rv = false;
    fastrtps::RequesterAttributes attrs;
    if (xmlobjects::parse_requester(xml.data(), xml.size(), attrs))
    {   
        rv = create_by_attributes(attrs);
    }
    return rv;
}

bool FastDDSRequester::create_by_ref(
        const std::string& ref)
{
    bool rv = false;
    fastrtps::RequesterAttributes attrs;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillRequesterAttributes(ref, attrs))
    {   
        rv = create_by_attributes(attrs);
    }
    return rv;
}

bool FastDDSRequester::create_by_attributes(
        const fastrtps::RequesterAttributes& attrs)
{
    bool rv = false;

    request_topic_ = std::make_shared<FastDDSTopic>(participant_);
    const fastrtps::TopicAttributes& request_topic_attrs = attrs.publisher.topic;
    if (!participant_->register_topic(request_topic_attrs, request_type_, request_topic_))
    {
        return false;
    }

    fastdds::dds::TopicQos qos_request_topic;
    set_qos_from_attributes(qos_request_topic, request_topic_attrs);
    if (!request_topic_->create_by_attributes(request_topic_attrs, 0))
    {
        return false;
    }

    reply_topic_ = std::make_shared<FastDDSTopic>(participant_);
    const fastrtps::TopicAttributes& reply_topic_attrs = attrs.subscriber.topic;
    if (!participant_->register_topic(reply_topic_attrs, reply_type_, reply_topic_))
    {
        return false;
    }

    fastdds::dds::TopicQos qos_reply_topic;
    set_qos_from_attributes(qos_reply_topic, reply_topic_attrs);
    if (!reply_topic_->create_by_attributes(reply_topic_attrs, 0))
    {
        return false;
    }
    
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
    bool rv = false;
    if ((0 == std::strcmp(request_type_->getName(), attrs.publisher.topic.getTopicDataType().c_str())) &&
         (  request_type_->m_isGetKeyDefined ==
            (attrs.publisher.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY)) &&
        (0 == std::strcmp(reply_type_->getName(), attrs.subscriber.topic.getTopicDataType().c_str())) &&
         (  reply_type_->m_isGetKeyDefined ==
            (attrs.subscriber.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY)))
    {   
        fastdds::dds::PublisherQos qos_publisher;
        set_qos_from_attributes(qos_publisher, attrs.publisher);
        fastdds::dds::SubscriberQos qos_subscriber;
        set_qos_from_attributes(qos_subscriber, attrs.subscriber);

        rv = (publisher_ptr_->get_qos() == qos_publisher && 
              subscriber_ptr_->get_qos() == qos_subscriber);
    }
    return rv;
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

FastDDSReplier::~FastDDSReplier()
{
    // TODO
}

bool FastDDSReplier::create_by_xml(
        const std::string& xml)
{
    bool rv = false;
    fastrtps::ReplierAttributes attrs;
    if (xmlobjects::parse_replier(xml.data(), xml.size(), attrs))
    {   
        rv = create_by_attributes(attrs);
    }
    return rv;
}

bool FastDDSReplier::create_by_ref(
        const std::string& ref)
{
    bool rv = false;
    fastrtps::ReplierAttributes attrs;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillReplierAttributes(ref, attrs))
    {   
        rv = create_by_attributes(attrs);
    }
    return rv;
}


bool FastDDSReplier::create_by_attributes(
        const fastrtps::ReplierAttributes& attrs)
{
    bool rv = false;

    request_topic_ = std::make_shared<FastDDSTopic>(participant_);
    const fastrtps::TopicAttributes& request_topic_attrs = attrs.subscriber.topic;
    if (!participant_->register_topic(request_topic_attrs, request_type_, request_topic_))
    {
        return false;
    }

    fastdds::dds::TopicQos qos_request_topic;
    set_qos_from_attributes(qos_request_topic, request_topic_attrs);
    if (!request_topic_->create_by_attributes(request_topic_attrs, 0))
    {
        return false;
    }

    reply_topic_ = std::make_shared<FastDDSTopic>(participant_);
    const fastrtps::TopicAttributes& reply_topic_attrs = attrs.publisher.topic;
    if (!participant_->register_topic(reply_topic_attrs, reply_type_, reply_topic_))
    {
        return false;
    }

    fastdds::dds::TopicQos qos_reply_topic;
    set_qos_from_attributes(qos_reply_topic, reply_topic_attrs);
    if (!reply_topic_->create_by_attributes(reply_topic_attrs, 0))
    {
        return false;
    }
    
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
    bool rv = false;
    if ((0 == std::strcmp(reply_type_->getName(), attrs.publisher.topic.getTopicDataType().c_str())) &&
         (  reply_type_->m_isGetKeyDefined ==
            (attrs.publisher.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY)) &&
        (0 == std::strcmp(request_type_->getName(), attrs.subscriber.topic.getTopicDataType().c_str())) &&
         (  request_type_->m_isGetKeyDefined ==
            (attrs.subscriber.topic.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY)))
    {   
        fastdds::dds::PublisherQos qos_publisher;
        set_qos_from_attributes(qos_publisher, attrs.publisher);
        fastdds::dds::SubscriberQos qos_subscriber;
        set_qos_from_attributes(qos_subscriber, attrs.subscriber);

        rv = (publisher_ptr_->get_qos() == qos_publisher && 
              subscriber_ptr_->get_qos() == qos_subscriber);
    }
    return rv;
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