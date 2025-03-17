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
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/WriteParams.hpp>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

namespace eprosima {
namespace uxr {

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
        fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    if (datawriter_xrce.has_qos())
    {
        fastdds::dds::ReliabilityQosPolicy reliability;
        reliability.kind =
            (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_reliable) ?
            fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS :
            fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
        qos.reliability() = reliability;

        // TODO set is_ownership_exclusive
        // TODO set user data

        fastdds::dds::DurabilityQosPolicy durability;
        if (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_transient_local)
        {
            durability.kind = fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        }
        else if (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_transient)
        {
            durability.kind = fastdds::dds::TRANSIENT_DURABILITY_QOS;
        }
        else if (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_persistent)
        {
            durability.kind = fastdds::dds::PERSISTENT_DURABILITY_QOS;
        }
        else
        {
            durability.kind = fastdds::dds::VOLATILE_DURABILITY_QOS;
        }

        qos.durability() = durability;

        fastdds::dds::HistoryQosPolicy history;
        history.kind =
            (datawriter_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_history_keep_last) ?
            fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS :
            fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;

        if (datawriter_xrce.qos().base().has_history_depth())
        {
            history.depth = datawriter_xrce.qos().base().history_depth();
        }
        qos.history() = history;

        if (datawriter_xrce.qos().base().has_deadline_msec())
        {
            fastdds::dds::DeadlineQosPolicy deadline;
            deadline.period = Duration_t(static_cast<long double>(datawriter_xrce.qos().base().deadline_msec()/1000.0));
            qos.deadline() = deadline;
        }

        if (datawriter_xrce.qos().base().has_lifespan_msec())
        {
            fastdds::dds::LifespanQosPolicy lifespan;
            lifespan.duration = Duration_t(static_cast<long double>(datawriter_xrce.qos().base().lifespan_msec()/1000.0));
            qos.lifespan() = lifespan;
        }

        if (datawriter_xrce.qos().has_ownership_strength())
        {
            fastdds::dds::OwnershipStrengthQosPolicy ownership_strength;
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
        fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    if (datareader_xrce.has_qos())
    {
        fastdds::dds::ReliabilityQosPolicy reliability;
        reliability.kind =
            (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_reliable) ?
            fastdds::dds::ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS :
            fastdds::dds::ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
        qos.reliability() = reliability;

        // TODO set is_ownership_exclusive
        // TODO set user data
        // TODO set m_contentbased_filter

        fastdds::dds::DurabilityQosPolicy durability;
        if (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_transient_local)
        {
            durability.kind = fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
        }
        else if (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_transient)
        {
            durability.kind = fastdds::dds::TRANSIENT_DURABILITY_QOS;
        }
        else if (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_durability_persistent)
        {
            durability.kind = fastdds::dds::PERSISTENT_DURABILITY_QOS;
        }
        else
        {
            durability.kind = fastdds::dds::VOLATILE_DURABILITY_QOS;
        }
        qos.durability() = durability;

        fastdds::dds::HistoryQosPolicy history;
        history.kind =
            (datareader_xrce.qos().base().qos_flags() & dds::xrce::EndpointQosFlags::is_history_keep_last) ?
            fastdds::dds::HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS :
            fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;

        if (datareader_xrce.qos().base().has_history_depth())
        {
            history.depth = datareader_xrce.qos().base().history_depth();
        }
        qos.history() = history;

        if (datareader_xrce.qos().base().has_deadline_msec())
        {
            fastdds::dds::DeadlineQosPolicy deadline;
            deadline.period = Duration_t(static_cast<long double>(datareader_xrce.qos().base().deadline_msec()/1000.0));
            qos.deadline() = deadline;
        }

        if (datareader_xrce.qos().base().has_lifespan_msec())
        {
            fastdds::dds::LifespanQosPolicy lifespan;
            lifespan.duration = Duration_t(static_cast<long double>(datareader_xrce.qos().base().lifespan_msec()/1000.0));
            qos.lifespan() = lifespan;
        }

        if (datareader_xrce.qos().has_timebasedfilter_msec())
        {
            fastdds::dds::TimeBasedFilterQosPolicy timebased_filter;
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
        if (ptr_->has_active_entities())
        {
            ptr_->delete_contained_entities();
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
    fastdds::dds::DomainParticipantQos qos = factory_->get_default_participant_qos();
    if (nullptr == ptr_ && (xml.size() == 0 || fastdds::dds::RETCODE_OK == factory_->get_participant_qos_from_xml(xml, qos)))
    {
        ptr_ = factory_->create_participant(domain_id_, qos);
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
    fastdds::dds::DomainParticipantQos qos = factory_->get_default_participant_qos();
    if (nullptr != ptr_ && fastdds::dds::RETCODE_OK == factory_->get_participant_qos_from_profile(ref, qos))
    {
        rv = (ptr_->get_qos().name() == qos.name());
    }
    return rv;
}

bool FastDDSParticipant::match_from_xml(
        const std::string& xml) const
{
    bool rv = false;
    fastdds::dds::DomainParticipantQos qos = factory_->get_default_participant_qos();
    if (nullptr != ptr_ && (xml.size() == 0 || fastdds::dds::RETCODE_OK == factory_->get_participant_qos_from_xml(xml, qos)))
    {
        rv = (ptr_->get_qos().name() == qos.name());
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

fastdds::dds::ReturnCode_t FastDDSParticipant::unregister_type(
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

fastdds::dds::ReturnCode_t FastDDSParticipant::delete_topic(
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

fastdds::dds::ReturnCode_t FastDDSParticipant::delete_publisher(
        fastdds::dds::Publisher* publisher)
{
    if (NULL == publisher)
    {
        return fastdds::dds::RETCODE_ALREADY_DELETED;
    }

    if (publisher->has_datawriters())
    {
        fastdds::dds::ReturnCode_t ret = fastdds::dds::RETCODE_OK;

        ret = publisher->delete_contained_entities();

        if (fastdds::dds::RETCODE_UNSUPPORTED == ret)
        {
            std::vector<eprosima::fastdds::dds::DataWriter*> writers;
            publisher->get_datawriters(writers);

            for (auto datawriter : writers)
            {
                ret = publisher->delete_datawriter(datawriter);

                if (fastdds::dds::RETCODE_OK != ret)
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

fastdds::dds::ReturnCode_t FastDDSParticipant::delete_subscriber(
        fastdds::dds::Subscriber* subscriber)
{
    if (NULL == subscriber)
    {
        return fastdds::dds::RETCODE_ALREADY_DELETED;
    }

    if (subscriber->has_datareaders())
    {
        fastdds::dds::ReturnCode_t ret = fastdds::dds::RETCODE_OK;

        ret = subscriber->delete_contained_entities();

        if (fastdds::dds::RETCODE_UNSUPPORTED == ret)
        {
            std::vector<eprosima::fastdds::dds::DataReader*> readers;
            subscriber->get_datareaders(readers);

            for (auto datareader : readers)
            {
                ret = subscriber->delete_datareader(datareader);

                if (fastdds::dds::RETCODE_OK != ret)
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
    return fastdds::dds::RETCODE_OK == ptr_->register_type(type_support, type_support->get_name())
        && type_register_.emplace(type_support->get_name(), type).second;
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
    participant_->unregister_local_type(type_support_->get_name());
    participant_->unregister_type(type_support_->get_name());
}

FastDDSTopic::~FastDDSTopic()
{
    participant_->unregister_local_topic(ptr_->get_name());
    participant_->delete_topic(ptr_);
}

bool FastDDSTopic::create_by_name_type(
    const std::string& name,
    const std::shared_ptr<FastDDSType>& type,
    const fastdds::dds::TopicQos& qos)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        ptr_ = participant_->create_topic(name, type->get_type_support()->get_name(), qos);

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
        fastdds::dds::TopicQos qos;
        std::string topic_name;
        std::string type_name;
        if (fastdds::dds::RETCODE_OK == participant_->get_ptr()->get_topic_qos_from_profile(ref, qos, topic_name, type_name))
        {
            rv = (ptr_->get_qos() == qos) && (ptr_->get_name() == topic_name) && (type_->get_type_support()->get_name() == type_name);
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
        fastdds::dds::TopicQos qos = participant_->get_ptr()->get_default_topic_qos();
        std::string topic_name;
        std::string type_name;
        if (xml.size() == 0 || fastdds::dds::RETCODE_OK == participant_->get_ptr()->get_topic_qos_from_xml(xml, qos, topic_name, type_name))
        {
            rv = (ptr_->get_qos() == qos) && (ptr_->get_name() == topic_name) && (type_->get_type_support()->get_name() == type_name);
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
        rv = (ptr_->get_qos() == qos) && (ptr_->get_name() == topic_xrce.topic_name()) && (type_->get_type_support()->get_name() == topic_xrce.type_name());
    }
    return rv;
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
        fastdds::dds::PublisherQos qos = participant_->get_ptr()->get_default_publisher_qos();
        if(xml.size() == 0 || fastdds::dds::RETCODE_OK == participant_->get_ptr()->get_publisher_qos_from_xml(xml, qos))
        {
            ptr_ = participant_->create_publisher(qos);
            rv = (nullptr != ptr_);
        }
    }
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

fastdds::dds::DataWriter* FastDDSPublisher::create_datawriter_with_profile(
        fastdds::dds::Topic* topic,
        const std::string& profile_name)
{
    return ptr_->create_datawriter_with_profile(topic, profile_name);
}

fastdds::dds::ReturnCode_t FastDDSPublisher::delete_datawriter(
    fastdds::dds::DataWriter* writer)
{
    if (NULL == writer)
    {
        return fastdds::dds::RETCODE_ALREADY_DELETED;
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
        fastdds::dds::SubscriberQos qos = participant_->get_ptr()->get_default_subscriber_qos();
        if(xml.size() == 0 || fastdds::dds::RETCODE_OK == participant_->get_ptr()->get_subscriber_qos_from_xml(xml, qos))
        {
            ptr_ = participant_->create_subscriber(qos);
            rv = (nullptr != ptr_);
        }
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

fastdds::dds::DataReader* FastDDSSubscriber::create_datareader_with_profile(
        fastdds::dds::TopicDescription* topic,
        const std::string& profile_name)
{
    return ptr_->create_datareader_with_profile(topic, profile_name);
}

fastdds::dds::ReturnCode_t FastDDSSubscriber::delete_datareader(
        fastdds::dds::DataReader* reader)
{
    if (NULL == reader)
    {
        return fastdds::dds::RETCODE_ALREADY_DELETED;
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
    if (nullptr == ptr_)
    {
        fastdds::dds::DataWriterQos qos;
        std::string topic_name;
        if (fastdds::dds::RETCODE_OK == publisher_->get_ptr()->get_datawriter_qos_from_profile(ref, qos, topic_name))
        {
            topic_ = publisher_->get_participant()->find_local_topic(topic_name);

            if (topic_)
            {
                ptr_ = publisher_->create_datawriter_with_profile(topic_->get_ptr(), ref);
                rv = (nullptr != ptr_);
            }
        }
    }
    return rv;
}

bool FastDDSDataWriter::create_by_xml(const std::string& xml)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        std::string topic_name;
        fastdds::dds::DataWriterQos qos = publisher_->get_ptr()->get_default_datawriter_qos();

        if (xml.size() == 0 || fastdds::dds::RETCODE_OK == publisher_->get_ptr()->get_datawriter_qos_from_xml(xml, qos, topic_name))
        {
            topic_ = publisher_->get_participant()->find_local_topic(topic_name);

            if (topic_)
            {
                ptr_ = publisher_->create_datawriter(topic_->get_ptr(), qos);
                rv = (nullptr != ptr_);
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

bool FastDDSDataWriter::match_from_ref(const std::string& ref) const
{
    bool rv = false;

    if (nullptr != ptr_)
    {
        fastdds::dds::DataWriterQos qos;
        std::string topic_name;
        if (fastdds::dds::RETCODE_OK == publisher_->get_ptr()->get_datawriter_qos_from_profile(ref, qos, topic_name))
        {
            rv = (ptr_->get_qos() == qos) && (topic_->get_name() == topic_name);
        }
    }

    return rv;
}

bool FastDDSDataWriter::match_from_xml(const std::string& xml) const
{
    bool rv = false;

    if (nullptr != ptr_)
    {
        fastdds::dds::DataWriterQos qos = publisher_->get_ptr()->get_default_datawriter_qos();
        std::string topic_name;
        if (xml.size() == 0 || fastdds::dds::RETCODE_OK == publisher_->get_ptr()->get_datawriter_qos_from_xml(xml, qos, topic_name))
        {
            rv = (ptr_->get_qos() == qos) && (topic_->get_name() == topic_name);
        }
    }

    return rv;
}

bool FastDDSDataWriter::match_from_bin(const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) const
{
    fastdds::dds::DataWriterQos qos;
    set_qos_from_xrce_object(qos, datawriter_xrce);
    return (ptr_->get_qos() == qos);
}

bool FastDDSDataWriter::write(const std::vector<uint8_t>& data)
{
    return fastdds::dds::RETCODE_OK == ptr_->write(&const_cast<std::vector<uint8_t>&>(data));
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

    if (nullptr == ptr_)
    {
        fastdds::dds::DataReaderQos qos;
        std::string topic_name;

        if (fastdds::dds::RETCODE_OK == subscriber_->get_ptr()->get_datareader_qos_from_profile(ref, qos, topic_name))
        {
            topic_ = subscriber_->get_participant()->find_local_topic(topic_name);

            if (topic_)
            {
                ptr_ = subscriber_->create_datareader_with_profile(topic_->get_ptr(), ref);
                rv = (nullptr != ptr_);
            }
        }
    }
    return rv;
}

bool FastDDSDataReader::create_by_xml(const std::string& xml)
{
    bool rv = false;
    if (nullptr == ptr_)
    {
        std::string topic_name;
        fastdds::dds::DataReaderQos qos = subscriber_->get_ptr()->get_default_datareader_qos();

        if (xml.size() == 0 || fastdds::dds::RETCODE_OK == subscriber_->get_ptr()->get_datareader_qos_from_xml(xml, qos, topic_name))
        {
            topic_ = subscriber_->get_participant()->find_local_topic(topic_name);

            if (topic_)
            {
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
        fastdds::dds::DataReaderQos qos;
        std::string topic_name;
        if (fastdds::dds::RETCODE_OK == subscriber_->get_ptr()->get_datareader_qos_from_profile(ref, qos, topic_name))
        {
            rv = (ptr_->get_qos() == qos) && (topic_->get_name() == topic_name);
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
        fastdds::dds::DataReaderQos qos = subscriber_->get_ptr()->get_default_datareader_qos();
        std::string topic_name;
        if (xml.size() == 0 || fastdds::dds::RETCODE_OK == subscriber_->get_ptr()->get_datareader_qos_from_xml(xml, qos, topic_name))
        {
            rv = (ptr_->get_qos() == qos) && (topic_->get_name() == topic_name);
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

    fastdds::dds::Duration_t d((long double) timeout.count()/1000.0);

    if(ptr_->wait_for_unread_message(d)){
        rv = fastdds::dds::RETCODE_OK == ptr_->take_next_sample(&data, &sample_info);
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

bool FastDDSRequester::create_by_qos(
        const fastdds::dds::RequesterQos& qos)
{
    publisher_ptr_ = participant_->create_publisher(participant_->get_ptr()->get_default_publisher_qos());
    datawriter_ptr_ = publisher_ptr_->create_datawriter(request_topic_->get_ptr(), qos.writer_qos);
    subscriber_ptr_ = participant_->create_subscriber(participant_->get_ptr()->get_default_subscriber_qos());
    datareader_ptr_ = subscriber_ptr_->create_datareader(reply_topic_->get_ptr(), qos.reader_qos);

    bool rv = (nullptr != publisher_ptr_) && (nullptr != datawriter_ptr_) &&
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

    fastdds::dds::RequesterQos qos;

    if(fastdds::dds::RETCODE_OK == participant_->get_ptr()->get_requester_qos_from_profile(ref, qos))
    {
        rv = datawriter_ptr_->get_qos() == qos.writer_qos
            && datareader_ptr_->get_qos() == qos.reader_qos
            && reply_topic_->get_name() == qos.reply_topic_name
            && request_topic_->get_name() == qos.request_topic_name
            && reply_topic_->get_type()->get_type_support()->get_name() == qos.reply_type
            && request_topic_->get_type()->get_type_support()->get_name() == qos.request_type;
    }

    return rv;
}

bool FastDDSRequester::match_from_xml(const std::string& xml) const
{
    bool rv = false;

    fastdds::dds::RequesterQos qos;
    qos.writer_qos = publisher_ptr_->get_default_datawriter_qos();
    qos.reader_qos = subscriber_ptr_->get_default_datareader_qos();

    if(xml.size() == 0 || fastdds::dds::RETCODE_OK == participant_->get_ptr()->get_requester_qos_from_xml(xml, qos))
    {
        rv = datawriter_ptr_->get_qos() == qos.writer_qos
            && datareader_ptr_->get_qos() == qos.reader_qos
            && reply_topic_->get_name() == qos.reply_topic_name
            && request_topic_->get_name() == qos.request_topic_name
            && reply_topic_->get_type()->get_type_support()->get_name() == qos.reply_type
            && request_topic_->get_type()->get_type_support()->get_name() == qos.request_type;
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
        && subscriber_ptr_->get_qos() == qos_subscriber
        && reply_topic_->get_name() == requester_xrce.reply_topic_name()
        && request_topic_->get_name() == requester_xrce.request_topic_name()
        && reply_topic_->get_type()->get_type_support()->get_name() == requester_xrce.reply_type()
        && request_topic_->get_type()->get_type_support()->get_name() == requester_xrce.request_type();
}

bool FastDDSRequester::write(
        uint32_t sequence_number,
        const std::vector<uint8_t>& data)
{
    bool rv = true;
    try
    {
        fastdds::rtps::WriteParams wparams;
        if (fastdds::dds::RETCODE_OK == datawriter_ptr_->write(&const_cast<std::vector<uint8_t>&>(data), wparams))
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

    fastdds::dds::Duration_t d((long double) timeout.count()/1000.0);

    if(datareader_ptr_->wait_for_unread_message(d)){
        rv = fastdds::dds::RETCODE_OK == datareader_ptr_->take_next_sample(&data, &info);
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

bool FastDDSReplier::create_by_qos(
        const fastdds::dds::ReplierQos& qos)
{
    bool rv = false;
    publisher_ptr_ = participant_->create_publisher(participant_->get_ptr()->get_default_publisher_qos());
    datawriter_ptr_ = publisher_ptr_->create_datawriter(reply_topic_->get_ptr(), qos.writer_qos);
    subscriber_ptr_ = participant_->create_subscriber(participant_->get_ptr()->get_default_subscriber_qos());
    datareader_ptr_ = subscriber_ptr_->create_datareader(request_topic_->get_ptr(), qos.reader_qos);

    rv = (nullptr != publisher_ptr_) && (nullptr != datawriter_ptr_) &&
         (nullptr != subscriber_ptr_) && (nullptr != datareader_ptr_);

    return rv;
}


bool FastDDSReplier::match_from_ref(const std::string& ref) const
{
    bool rv = false;

    fastdds::dds::ReplierQos qos;

    if (fastdds::dds::RETCODE_OK == participant_->get_ptr()->get_replier_qos_from_profile(ref, qos))
    {
        rv = datawriter_ptr_->get_qos() == qos.writer_qos
            && datareader_ptr_->get_qos() == qos.reader_qos
            && reply_topic_->get_name() == qos.reply_topic_name
            && request_topic_->get_name() == qos.request_topic_name
            && reply_topic_->get_type()->get_type_support()->get_name() == qos.reply_type
            && request_topic_->get_type()->get_type_support()->get_name() == qos.request_type;
    }

    return rv;
}

bool FastDDSReplier::match_from_xml(const std::string& xml) const
{
    bool rv = false;

    fastdds::dds::ReplierQos qos;
    qos.writer_qos = publisher_ptr_->get_default_datawriter_qos();
    qos.reader_qos = subscriber_ptr_->get_default_datareader_qos();

    if (xml.size() == 0 || fastdds::dds::RETCODE_OK == participant_->get_ptr()->get_replier_qos_from_xml(xml, qos))
    {
        rv = datawriter_ptr_->get_qos() == qos.writer_qos
            && datareader_ptr_->get_qos() == qos.reader_qos
            && reply_topic_->get_name() == qos.reply_topic_name
            && request_topic_->get_name() == qos.request_topic_name
            && reply_topic_->get_type()->get_type_support()->get_name() == qos.reply_type
            && request_topic_->get_type()->get_type_support()->get_name() == qos.request_type;
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
        && subscriber_ptr_->get_qos() == qos_subscriber
        && reply_topic_->get_name() == replier_xrce.reply_topic_name()
        && request_topic_->get_name() == replier_xrce.request_topic_name()
        && reply_topic_->get_type()->get_type_support()->get_name() == replier_xrce.reply_type()
        && request_topic_->get_type()->get_type_support()->get_name() == replier_xrce.request_type();
}

void FastDDSReplier::transport_sample_identity(
        const dds::SampleIdentity& dds_identity,
        fastdds::rtps::SampleIdentity& fast_identity)
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

    fastdds::rtps::WriteParams wparams;
    transport_sample_identity(sample_identity, wparams.related_sample_identity());

    std::vector<uint8_t> output_data(data.size() - deserializer.get_serialized_data_length());
    deserializer.deserialize_array(output_data.data(), output_data.size());

    return fastdds::dds::RETCODE_OK == datawriter_ptr_->write(&const_cast<std::vector<uint8_t>&>(output_data), wparams);
}

void FastDDSReplier::transform_sample_identity(
        const fastdds::rtps::SampleIdentity& fast_identity,
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

    fastdds::dds::Duration_t d((long double) timeout.count()/1000.0);

    if(datareader_ptr_->wait_for_unread_message(d)){
        rv = fastdds::dds::RETCODE_OK == datareader_ptr_->take_next_sample(&temp_data, &info);
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