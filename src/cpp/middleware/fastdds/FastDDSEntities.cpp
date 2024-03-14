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

static void set_qos_from_xrce_object(
        fastdds::dds::DomainParticipantQos& qos,
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce)
{
    if (participant_xrce.has_qos_profile())
    {
        qos.name() = participant_xrce.qos_profile();
    }
    else
    {
        qos.name() = "MicroXRCEParticipant";
    }
}

static void set_qos_from_xrce_object(
        fastdds::dds::PublisherQos& /* qos */,
        const dds::xrce::OBJK_Publisher_Binary& /* publisher_xrce */)
{
    // TODO copy group_data
    // TODO copy partition
    return;
}

static void set_qos_from_xrce_object(
        fastdds::dds::SubscriberQos& /* qos */,
        const dds::xrce::OBJK_Subscriber_Binary& /* subscriber_xrce */)
{
    // TODO copy group_data
    // TODO copy partition
    return;
}

static void set_qos_from_xrce_object(
        fastdds::dds::DataWriterQos& qos,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce)
{
    qos.endpoint().history_memory_policy =
        fastrtps::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    if (datawriter_xrce.has_qos())
    {
        ReliabilityQosPolicy reliability;
        reliability.kind =
            (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_reliable) ?
            ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS :
            ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
        qos.reliability() = reliability;

        // TODO set is_ownership_exclusive
        // TODO set user data

        DurabilityQosPolicy durability;
        if (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_transient_local)
        {
            durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        }
        else if (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_transient)
        {
            durability.kind = TRANSIENT_DURABILITY_QOS;
        }
        else if (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_persistent)
        {
            durability.kind = PERSISTENT_DURABILITY_QOS;
        }
        else
        {
            durability.kind = VOLATILE_DURABILITY_QOS;
        }

        qos.durability() = durability;

        HistoryQosPolicy history;
        history.kind =
            (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_history_keep_last) ?
            HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS :
            HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;

        if (datawriter_xrce.qos().base().has_history_depth())
        {
            history.depth = datawriter_xrce.qos().base().history_depth();
        }
        qos.history() = history;

        if (datawriter_xrce.qos().base().has_deadline_msec())
        {
            DeadlineQosPolicy deadline;
            deadline.period = Duration_t(static_cast<long double>(datawriter_xrce.qos().base().deadline_msec()/1000.0));
            qos.deadline() = deadline;
        }

        if (datawriter_xrce.qos().base().has_lifespan_msec())
        {
            LifespanQosPolicy lifespan;
            lifespan.duration = Duration_t(static_cast<long double>(datawriter_xrce.qos().base().lifespan_msec()/1000.0));
            qos.lifespan() = lifespan;
        }

        if (datawriter_xrce.qos().has_ownership_strength())
        {
            OwnershipStrengthQosPolicy ownership_strength;
            ownership_strength.value = datawriter_xrce.qos().ownership_strength();
            qos.ownership_strength() = ownership_strength;
        }
    }
}

static void set_qos_from_xrce_object(
        fastdds::dds::DataReaderQos& qos,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce)
{
    qos.endpoint().history_memory_policy =
        fastrtps::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    if (datareader_xrce.has_qos())
    {
        ReliabilityQosPolicy reliability;
        reliability.kind =
            (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_reliable) ?
            ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS :
            ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
        qos.reliability() = reliability;

        // TODO set is_ownership_exclusive
        // TODO set user data
        // TODO set m_contentbased_filter

        DurabilityQosPolicy durability;
        if (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_transient_local)
        {
            durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
        }
        else if (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_transient)
        {
            durability.kind = TRANSIENT_DURABILITY_QOS;
        }
        else if (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_persistent)
        {
            durability.kind = PERSISTENT_DURABILITY_QOS;
        }
        else
        {
            durability.kind = VOLATILE_DURABILITY_QOS;
        }
        qos.durability() = durability;

        HistoryQosPolicy history;
        history.kind =
            (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_history_keep_last) ?
            HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS :
            HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;

        if (datareader_xrce.qos().base().has_history_depth())
        {
            history.depth = datareader_xrce.qos().base().history_depth();
        }
        qos.history() = history;

        if (datareader_xrce.qos().base().has_deadline_msec())
        {
            DeadlineQosPolicy deadline;
            deadline.period = Duration_t(static_cast<long double>(datareader_xrce.qos().base().deadline_msec()/1000.0));
            qos.deadline() = deadline;
        }

        if (datareader_xrce.qos().base().has_lifespan_msec())
        {
            LifespanQosPolicy lifespan;
            lifespan.duration = Duration_t(static_cast<long double>(datareader_xrce.qos().base().lifespan_msec()/1000.0));
            qos.lifespan() = lifespan;
        }

        if (datareader_xrce.qos().has_timebasedfilter_msec())
        {
            TimeBasedFilterQosPolicy timebased_filter;
            timebased_filter.minimum_separation = Duration_t(static_cast<long double>(datareader_xrce.qos().timebasedfilter_msec()/1000.0));;
            qos.time_based_filter() = timebased_filter;
        }

    }
}

static void set_qos_from_xrce_object(
        fastdds::dds::TopicQos& /* qos */,
        const dds::xrce::OBJK_Topic_Binary& /* topic_xrce */)
{
    return;
}

static void set_qos_from_xrce_object(
        fastdds::dds::PublisherQos& /* qos */,
        const dds::xrce::OBJK_Requester_Binary& /* requester_xrce */)
{
    return;
}

static void set_qos_from_xrce_object(
        fastdds::dds::SubscriberQos& /* qos */,
        const dds::xrce::OBJK_Requester_Binary& /* requester_xrce */)
{
    return;
}

static void set_qos_from_xrce_object(
        fastdds::dds::PublisherQos& /* qos */,
        const dds::xrce::OBJK_Replier_Binary& /* replier_xrce */)
{
    return;
}

static void set_qos_from_xrce_object(
        fastdds::dds::SubscriberQos& /* qos */,
        const dds::xrce::OBJK_Replier_Binary& /* replier_xrce */)
{
    return;
}

/**********************************************************************************************************************
 * FastDDSParticipant
 **********************************************************************************************************************/
FastDDSParticipant::~FastDDSParticipant()
{
    if (ptr_)
    {
        // TODO: Not available on foxy (Need FastDDS >= 2.2.0 for declaration and FastDDS >= 2.4.1 for implementation)
        // if (ptr_->has_active_entities())
        {
            // ptr_->delete_contained_entities();
        }

        factory_->delete_participant(ptr_);
    }
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
            fastdds::dds::DomainParticipantQos qos = factory_->get_default_participant_qos();
            set_qos_from_attributes(qos, attrs.rtps);
            ptr_ = factory_->create_participant(domain_id_, qos);
        }
        rv = (nullptr != ptr_);
    }
    return rv;
}

bool FastDDSParticipant::create_by_bin(
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastdds::dds::DomainParticipantQos qos = factory_->get_default_participant_qos();
        set_qos_from_xrce_object(qos, participant_xrce);
        ptr_ = factory_->create_participant(domain_id_, qos);
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
            rv = (ptr_->get_qos().name() == qos.name());
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
            rv = (ptr_->get_qos().name() == qos.name());
        }
    }
    return rv;
}

bool FastDDSParticipant::match_from_bin(
        const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastdds::dds::DomainParticipantQos qos;
        set_qos_from_xrce_object(qos, participant_xrce);
        rv = (ptr_->get_qos().name() == qos.name());
    }
    return rv;
}

// Proxy methods

ReturnCode_t FastDDSParticipant::unregister_type(
            const std::string& typeName)
{
    return ptr_->unregister_type(typeName);
}

fastdds::dds::Topic* FastDDSParticipant::create_topic(
        const std::string& topic_name,
        const std::string& type_name,
        const fastdds::dds::TopicQos& qos,
        fastdds::dds::TopicListener* listener,
        const fastdds::dds::StatusMask& mask)
{
    return ptr_->create_topic(topic_name, type_name, qos, listener, mask);
}

ReturnCode_t FastDDSParticipant::delete_topic(
    fastdds::dds::Topic* topic)
{
    return ptr_->delete_topic(topic);
}

fastdds::dds::Publisher* FastDDSParticipant::create_publisher(
        const fastdds::dds::PublisherQos& qos,
        fastdds::dds::PublisherListener* listener,
        const fastdds::dds::StatusMask& mask)
{
    return ptr_->create_publisher(qos, listener, mask);
}

ReturnCode_t FastDDSParticipant::delete_publisher(
        fastdds::dds::Publisher* publisher)
{
    if (NULL == publisher)
    {
        return ReturnCode_t::RETCODE_ALREADY_DELETED;
    }

    if (publisher->has_datawriters())
    {
        ReturnCode_t ret = ReturnCode_t::RETCODE_OK;

        // TODO: Not available on foxy (Need FastDDS >= 2.2.0 for declaration and FastDDS >= 2.4.1 for implementation)
        // ret = publisher->delete_contained_entities();

        //if (ReturnCode_t::RETCODE_UNSUPPORTED == ret)
        {
            std::vector<eprosima::fastdds::dds::DataWriter*> writers;
            publisher->get_datawriters(writers);

            for (auto datawriter : writers)
            {
                ret = publisher->delete_datawriter(datawriter);

                if (ReturnCode_t::RETCODE_OK != ret)
                {
                    return ret;
                }
            }
        }
    }

    return ptr_->delete_publisher(publisher);
}

fastdds::dds::Subscriber* FastDDSParticipant::create_subscriber(
        const fastdds::dds::SubscriberQos& qos,
        fastdds::dds::SubscriberListener* listener,
        const fastdds::dds::StatusMask& mask)
{
    return ptr_->create_subscriber(qos, listener, mask);
}

ReturnCode_t FastDDSParticipant::delete_subscriber(
        fastdds::dds::Subscriber* subscriber)
{
    if (NULL == subscriber)
    {
        return ReturnCode_t::RETCODE_ALREADY_DELETED;
    }

    if (subscriber->has_datareaders())
    {
        ReturnCode_t ret = ReturnCode_t::RETCODE_OK;

        // TODO: Not available on foxy (Need FastDDS >= 2.2.0 for declaration and FastDDS >= 2.4.1 for implementation)
        // ret = subscriber->delete_contained_entities();

        // if (ReturnCode_t::RETCODE_UNSUPPORTED == ret)
        {
            std::vector<eprosima::fastdds::dds::DataReader*> readers;
            subscriber->get_datareaders(readers);

            for (auto datareader : readers)
            {
                ret = subscriber->delete_datareader(datareader);

                if (ReturnCode_t::RETCODE_OK != ret)
                {
                    return ret;
                }
            }
        }
    }

    return ptr_->delete_subscriber(subscriber);
}

// Types and topics registration

bool FastDDSParticipant::register_local_type(
        const std::shared_ptr<FastDDSType>& type)
{
    fastdds::dds::TypeSupport& type_support = type->get_type_support();
    return ReturnCode_t::RETCODE_OK == ptr_->register_type(type_support, type_support->getName())
        && type_register_.emplace(type_support->getName(), type).second;
}

bool FastDDSParticipant::unregister_local_type(
        const std::string& type_name)
{
    return (1 == type_register_.erase(type_name));
}

std::shared_ptr<FastDDSType> FastDDSParticipant::find_local_type(
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

bool FastDDSParticipant::register_local_topic(
            const std::shared_ptr<FastDDSTopic>& topic)
{
    return topic_register_.emplace(topic->get_name(), topic).second;
}

bool FastDDSParticipant::unregister_local_topic(
        const std::string& topic_name)
{
    ptr_->unregister_type(topic_name);
    return (1 == topic_register_.erase(topic_name));
}

std::shared_ptr<FastDDSTopic> FastDDSParticipant::find_local_topic(
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

const fastdds::dds::DomainParticipant* FastDDSParticipant::operator * () const
{
    return ptr_;
}

fastdds::dds::DomainParticipant* FastDDSParticipant::operator * ()
{
    return ptr_;
}

/**********************************************************************************************************************
 * FastDDSTopic
 **********************************************************************************************************************/
FastDDSType::~FastDDSType()
{
    participant_->unregister_local_type(type_support_->getName());
    participant_->unregister_type(type_support_->getName());
}

FastDDSTopic::~FastDDSTopic()
{
    participant_->unregister_local_topic(ptr_->get_name());
    participant_->delete_topic(ptr_);
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

        ptr_ = participant_->create_topic(attrs.getTopicName().to_string(),
                attrs.getTopicDataType().to_string(), qos);

        rv = (nullptr != ptr_);
    }
    return rv;
}

bool FastDDSTopic::create_by_name_type(
    const std::string& name,
    const std::shared_ptr<FastDDSType>& type)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastdds::dds::TopicQos qos;

        ptr_ = participant_->create_topic(name, type->get_type_support()->getName(), qos);

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

bool FastDDSTopic::match_from_bin(
        const dds::xrce::OBJK_Topic_Binary& topic_xrce) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastdds::dds::TopicQos qos;
        set_qos_from_xrce_object(qos, topic_xrce);
        rv = (ptr_->get_qos() == qos);
    }
    return rv;
}

bool FastDDSTopic::match(const fastrtps::TopicAttributes& attrs) const
{
    return (attrs.getTopicName() == ptr_->get_name())
        && (0 == std::strcmp(type_->get_type_support()->getName(), attrs.getTopicDataType().c_str()))
        && (type_->get_type_support()->m_isGetKeyDefined == (attrs.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY));
}

/**********************************************************************************************************************
 * FastDDSPublisher
 **********************************************************************************************************************/
FastDDSPublisher::~FastDDSPublisher()
{
    if (ptr_)
    {
        participant_->delete_publisher(ptr_);
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
        ptr_ = participant_->create_publisher(qos);
        rv = (nullptr != ptr_);    }
    return rv;
}

bool FastDDSPublisher::create_by_bin(
        const dds::xrce::OBJK_Publisher_Binary& publisher_xrce)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastdds::dds::PublisherQos qos = fastdds::dds::PUBLISHER_QOS_DEFAULT;
        set_qos_from_xrce_object(qos, publisher_xrce);
        ptr_ = participant_->create_publisher(qos);
        rv = (nullptr != ptr_);    }
    return rv;
}

fastdds::dds::DataWriter* FastDDSPublisher::create_datawriter(
        fastdds::dds::Topic* topic,
        const fastdds::dds::DataWriterQos& qos,
        fastdds::dds::DataWriterListener* listener,
        const fastdds::dds::StatusMask& mask)
{
    return ptr_->create_datawriter(topic, qos, listener, mask);
}

ReturnCode_t FastDDSPublisher::delete_datawriter(
    fastdds::dds::DataWriter* writer)
{
    if (NULL == writer)
    {
        return ReturnCode_t::RETCODE_ALREADY_DELETED;
    }

    return ptr_->delete_datawriter(writer);
}


/**********************************************************************************************************************
 * FastDDSSubscriber
 **********************************************************************************************************************/
FastDDSSubscriber::~FastDDSSubscriber()
{
    if (ptr_)
    {
        participant_->delete_subscriber(ptr_);
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
        ptr_ = participant_->create_subscriber(qos);
        rv = (nullptr != ptr_);
    }
    return rv;
}

bool FastDDSSubscriber::create_by_bin(
        const dds::xrce::OBJK_Subscriber_Binary& subscriber_xrce)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        fastdds::dds::SubscriberQos qos = fastdds::dds::SUBSCRIBER_QOS_DEFAULT;
        set_qos_from_xrce_object(qos, subscriber_xrce);
        ptr_ = participant_->create_subscriber(qos);
        rv = (nullptr != ptr_);    }
    return rv;
}

fastdds::dds::DataReader* FastDDSSubscriber::create_datareader(
        fastdds::dds::TopicDescription* topic,
        const fastdds::dds::DataReaderQos& reader_qos,
        fastdds::dds::DataReaderListener* listener,
        const fastdds::dds::StatusMask& mask)
{
    return ptr_->create_datareader(topic, reader_qos, listener, mask);
}

ReturnCode_t FastDDSSubscriber::delete_datareader(
        fastdds::dds::DataReader* reader)
{
    if (NULL == reader)
    {
        return ReturnCode_t::RETCODE_ALREADY_DELETED;
    }

    return ptr_->delete_datareader(reader);
}

/**********************************************************************************************************************
 * FastDDSDataWriter
 **********************************************************************************************************************/
FastDDSDataWriter::~FastDDSDataWriter()
{
    if (ptr_)
    {
        publisher_->delete_datawriter(ptr_);
    }
}

bool FastDDSDataWriter::create_by_ref(const std::string& ref)
{
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::PublisherAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillPublisherAttributes(ref, attrs))
        {
            topic_ = publisher_->get_participant()->find_local_topic(attrs.topic.topicName.c_str());
            if(topic_){
                fastdds::dds::DataWriterQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = publisher_->create_datawriter(topic_->get_ptr(), qos);
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
            topic_ = publisher_->get_participant()->find_local_topic(attrs.topic.topicName.c_str());
            if(topic_){
                fastdds::dds::DataWriterQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = publisher_->create_datawriter(topic_->get_ptr(), qos);
                rv = (nullptr != ptr_) && bool(topic_);
            }
        }
    }
    return rv;
}

bool FastDDSDataWriter::create_by_bin(
    const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce,
    std::shared_ptr<eprosima::uxr::FastDDSTopic> topic)
{
    bool rv = false;
    topic_ = topic;
    if (nullptr == ptr_){
        if(topic_){
            fastdds::dds::DataWriterQos qos = fastdds::dds::DATAWRITER_QOS_DEFAULT;
            set_qos_from_xrce_object(qos, datawriter_xrce);
            ptr_ = publisher_->create_datawriter(topic_->get_ptr(), qos);
            rv = (nullptr != ptr_) && bool(topic_);
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


bool FastDDSDataWriter::match_from_bin(const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) const
{
    fastdds::dds::DataWriterQos qos;
    set_qos_from_xrce_object(qos, datawriter_xrce);
    return (ptr_->get_qos() == qos);
}

bool FastDDSDataWriter::write(const std::vector<uint8_t>& data)
{
    return ptr_->write(&const_cast<std::vector<uint8_t>&>(data));
}

const fastdds::dds::DataWriter* FastDDSDataWriter::ptr() const
{
    return ptr_;
}

const fastdds::dds::DomainParticipant* FastDDSDataWriter::participant() const
{
    return publisher_->get_participant()->get_ptr();
}

/**********************************************************************************************************************
 * FastDDSDataReader
 **********************************************************************************************************************/
FastDDSDataReader::~FastDDSDataReader()
{
    if (ptr_)
    {
        subscriber_->delete_datareader(ptr_);
    }
}

bool FastDDSDataReader::create_by_ref(const std::string& ref)
{
    bool rv = false;
    if (nullptr == ptr_){
        fastrtps::SubscriberAttributes attrs;
        if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(ref, attrs))
        {
            topic_ = subscriber_->get_participant()->find_local_topic(attrs.topic.topicName.c_str());
            if(topic_){
                fastdds::dds::DataReaderQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = subscriber_->create_datareader(topic_->get_ptr(), qos);
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
            topic_ = subscriber_->get_participant()->find_local_topic(attrs.topic.topicName.c_str());
            if(topic_){
                fastdds::dds::DataReaderQos qos;
                set_qos_from_attributes(qos, attrs);

                ptr_ = subscriber_->create_datareader(topic_->get_ptr(), qos);
                rv = (nullptr != ptr_);
            }
        }
    }
    return rv;
}

bool FastDDSDataReader::create_by_bin(
    const dds::xrce::OBJK_DataReader_Binary& datareader_xrce,
    std::shared_ptr<eprosima::uxr::FastDDSTopic> topic)
{
    bool rv = false;
    topic_ = topic;
    if (nullptr == ptr_){
        if(topic_){
            fastdds::dds::DataReaderQos qos = fastdds::dds::DATAREADER_QOS_DEFAULT;
            set_qos_from_xrce_object(qos, datareader_xrce);
            ptr_ = subscriber_->create_datareader(topic_->get_ptr(), qos);
            rv = (nullptr != ptr_) && bool(topic_);
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

bool FastDDSDataReader::match_from_bin(
        const dds::xrce::OBJK_DataReader_Binary& datawriter_xrce) const
{
    bool rv = false;
    if (nullptr != ptr_)
    {
        fastdds::dds::DataReaderQos qos;
        set_qos_from_xrce_object(qos, datawriter_xrce);
        rv = (ptr_->get_qos() == qos);
    }
    return rv;
}

bool FastDDSDataReader::read(
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout,
        fastdds::dds::SampleInfo& sample_info)
{

    bool rv = false;

    fastrtps::Duration_t d((long double) timeout.count()/1000.0);

    if(ptr_->wait_for_unread_message(d)){
        rv = ReturnCode_t::RETCODE_OK == ptr_->take_next_sample(&data, &sample_info);
    }

    return rv;
}

const fastdds::dds::DataReader* FastDDSDataReader::ptr() const
{
    return ptr_;
}

const fastdds::dds::DomainParticipant* FastDDSDataReader::participant() const
{
    return subscriber_->get_participant()->get_ptr();
}

/**********************************************************************************************************************
 * FastDDSRequester
 **********************************************************************************************************************/
FastDDSRequester::~FastDDSRequester()
{
    if (publisher_ptr_)
    {
        participant_->delete_publisher(publisher_ptr_);
    }
    if (subscriber_ptr_)
    {
        participant_->delete_subscriber(subscriber_ptr_);
    }
}

bool FastDDSRequester::create_by_attributes(
        const fastrtps::RequesterAttributes& attrs)
{
    bool rv = false;

    fastdds::dds::PublisherQos qos_publisher;
    set_qos_from_attributes(qos_publisher, attrs.publisher);
    publisher_ptr_ = participant_->create_publisher(qos_publisher);

    fastdds::dds::DataWriterQos qos_datawriter;
    set_qos_from_attributes(qos_datawriter, attrs.publisher);
    datawriter_ptr_ = publisher_ptr_->create_datawriter(request_topic_->get_ptr(), qos_datawriter);

    fastdds::dds::SubscriberQos qos_subscriber;
    set_qos_from_attributes(qos_subscriber, attrs.subscriber);
    subscriber_ptr_ = participant_->create_subscriber(qos_subscriber);

    fastdds::dds::DataReaderQos qos_datareader;
    set_qos_from_attributes(qos_datareader, attrs.subscriber);
    datareader_ptr_ = subscriber_ptr_->create_datareader(reply_topic_->get_ptr(), qos_datareader);

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

bool FastDDSRequester::match_from_bin(const dds::xrce::OBJK_Requester_Binary& requester_xrce) const
{
    fastdds::dds::PublisherQos qos_publisher;
    set_qos_from_xrce_object(qos_publisher, requester_xrce);
    fastdds::dds::SubscriberQos qos_subscriber;
    set_qos_from_xrce_object(qos_subscriber, requester_xrce);

    return publisher_ptr_->get_qos() == qos_publisher
        && subscriber_ptr_->get_qos() == qos_subscriber;
        // TODO Check topic names
}

bool FastDDSRequester::match(const fastrtps::RequesterAttributes& attrs) const
{
    fastdds::dds::PublisherQos qos_publisher;
    set_qos_from_attributes(qos_publisher, attrs.publisher);
    fastdds::dds::SubscriberQos qos_subscriber;
    set_qos_from_attributes(qos_subscriber, attrs.subscriber);

    return reply_topic_->match(attrs.subscriber.topic)
        && request_topic_->match(attrs.publisher.topic)
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
        std::chrono::milliseconds timeout,
        fastdds::dds::SampleInfo& info)
{
    bool rv = false;

    fastrtps::Duration_t d((long double) timeout.count()/1000.0);

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

const fastdds::dds::DomainParticipant* FastDDSRequester::get_participant() const
{
    return participant_->get_ptr();
}

const fastdds::dds::DataWriter* FastDDSRequester::get_request_datawriter() const
{
    return datawriter_ptr_;
}

const fastdds::dds::DataReader* FastDDSRequester::get_reply_datareader() const
{
    return datareader_ptr_;
}

/**********************************************************************************************************************
 * FastDDSReplier
 **********************************************************************************************************************/
FastDDSReplier::~FastDDSReplier()
{
    if (publisher_ptr_)
    {
        participant_->delete_publisher(publisher_ptr_);
    }
    if (subscriber_ptr_)
    {
        participant_->delete_subscriber(subscriber_ptr_);
    }
}

bool FastDDSReplier::create_by_attributes(
        const fastrtps::ReplierAttributes& attrs)
{
    bool rv = false;

    fastdds::dds::PublisherQos qos_publisher;
    set_qos_from_attributes(qos_publisher, attrs.publisher);
    publisher_ptr_ = participant_->create_publisher(qos_publisher);

    fastdds::dds::DataWriterQos qos_datawriter;
    set_qos_from_attributes(qos_datawriter, attrs.publisher);
    datawriter_ptr_ = publisher_ptr_->create_datawriter(reply_topic_->get_ptr(), qos_datawriter);

    fastdds::dds::SubscriberQos qos_subscriber;
    set_qos_from_attributes(qos_subscriber, attrs.subscriber);
    subscriber_ptr_ = participant_->create_subscriber(qos_subscriber);

    fastdds::dds::DataReaderQos qos_datareader;
    set_qos_from_attributes(qos_datareader, attrs.subscriber);
    datareader_ptr_ = subscriber_ptr_->create_datareader(request_topic_->get_ptr(), qos_datareader);

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

bool FastDDSReplier::match_from_bin(const dds::xrce::OBJK_Replier_Binary& replier_xrce) const
{
    fastdds::dds::PublisherQos qos_publisher;
    set_qos_from_xrce_object(qos_publisher, replier_xrce);
    fastdds::dds::SubscriberQos qos_subscriber;
    set_qos_from_xrce_object(qos_subscriber, replier_xrce);

    return publisher_ptr_->get_qos() == qos_publisher
        && subscriber_ptr_->get_qos() == qos_subscriber;
        // TODO Check topic names
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
    fastcdr::Cdr deserializer(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::CdrVersion::XCDRv1);

    dds::SampleIdentity sample_identity;
    sample_identity.deserialize(deserializer);

    fastrtps::rtps::WriteParams wparams;
    transport_sample_identity(sample_identity, wparams.related_sample_identity());

    std::vector<uint8_t> output_data(data.size() - deserializer.get_serialized_data_length());
    deserializer.deserialize_array(output_data.data(), output_data.size());

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
        std::chrono::milliseconds timeout,
        fastdds::dds::SampleInfo& info)
{
    std::vector<uint8_t> temp_data;

    bool rv = false;

    fastrtps::Duration_t d((long double) timeout.count()/1000.0);

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
        fastcdr::Cdr serializer(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::CdrVersion::XCDRv1);

        try
        {
            sample_identity.serialize(serializer);
            serializer.serialize_array(temp_data.data(), temp_data.size());
        }
        catch(const std::exception&)
        {
            rv = false;
        }
    }

    return rv;
}

const fastdds::dds::DomainParticipant* FastDDSReplier::get_participant() const
{
    return participant_->get_ptr();
}

const fastdds::dds::DataReader* FastDDSReplier::get_request_datareader() const
{
    return datareader_ptr_;
}

const fastdds::dds::DataWriter* FastDDSReplier::get_reply_datawriter() const
{
    return datawriter_ptr_;
}

} // namespace uxr
} // namespace eprosima