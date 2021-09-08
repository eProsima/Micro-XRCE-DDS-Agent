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

#include <uxr/agent/middleware/ced/CedMiddleware.hpp>
#include <uxr/agent/utils/Conversion.hpp>

namespace eprosima {
namespace uxr {

CedMiddleware::CedMiddleware(uint32_t client_key)
    : participants_{}
    , topics_{}
    , publishers_{}
    , subscribers_{}
    , datawriters_{}
    , datareaders_{}
    , topics_src_{}
    , write_access_{}
    , read_access_{}
{
    if (INTERNAL_CLIENT_KEY == client_key)
    {
        topics_src_ = TopicSource::EXTERNAL;
        write_access_ = WriteAccess::EXTERNAL;
        read_access_ = ReadAccess::NONE;
    }
    else
    {
        if (EXTERNAL_CLIENT_KEY_PREFIX == (client_key >> 24))
        {
            topics_src_ = TopicSource::EXTERNAL;
            write_access_ = WriteAccess::NONE;
            read_access_ = ReadAccess::INTERNAL;
        }
        else
        {
            topics_src_ = TopicSource::INTERNAL;
            write_access_ = WriteAccess::COMPLETE;
            read_access_ = ReadAccess::COMPLETE;
        }
    }
}

std::string remove_suffix_form_topic_ref(std::string const & ref)
{
    return ref.substr(0, ref.find("__t"));
}

std::string remove_suffix_form_datawriter_ref(std::string const & ref)
{
    return ref.substr(0, ref.find("__dw"));
}

std::string remove_suffix_form_datareader_ref(std::string const & ref)
{
    return ref.substr(0, ref.find("__dr"));
}

/**********************************************************************************************************************
 * Create functions.
 **********************************************************************************************************************/
bool CedMiddleware::create_participant_by_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& /*ref*/)
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() == it)
    {
        participants_.emplace(participant_id, std::make_shared<CedParticipant>(domain_id));
        rv = true;
    }
    return rv;
}

bool CedMiddleware::create_participant_by_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml)
{
    return create_participant_by_ref(participant_id, domain_id, xml);
}

bool CedMiddleware::create_participant_by_bin(
            uint16_t participant_id,
            const dds::xrce::OBJK_DomainParticipant_Binary& participant_xrce)
{
    return create_participant_by_ref(participant_id, (int16_t) participant_xrce.domain_id(), std::string());
}

static
std::shared_ptr<CedTopic> create_topic(
        std::shared_ptr<CedParticipant>& participant,
        const std::string& topic_name)
{
    std::shared_ptr<CedTopic> topic;
    topic = participant->find_topic(topic_name);
    if (!topic)
    {
        std::shared_ptr<CedGlobalTopic> global_topic;
        if (CedTopicManager::register_topic(topic_name, participant->get_domain_id(), global_topic))
        {
            topic = std::make_shared<CedTopic>(participant, global_topic);
            if (!participant->register_topic(topic))
            {
                topic.reset();
            }
        }
    }
    return topic;
}

bool CedMiddleware::create_topic_by_ref(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        auto it_topic = topics_.find(topic_id);
        if (topics_.end() == it_topic)
        {
            std::shared_ptr<CedTopic> topic =
                create_topic(it_participant->second, remove_suffix_form_topic_ref(ref));
            rv = topic && topics_.emplace(topic_id, std::move(topic)).second;
        }
    }
    return rv;
}

bool CedMiddleware::create_topic_by_xml(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& xml)
{
    return create_topic_by_ref(topic_id, participant_id, xml);
}

bool CedMiddleware::create_topic_by_bin(
        uint16_t topic_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Topic_Binary& topic_xrce)
{
    return create_topic_by_ref(topic_id, participant_id, topic_xrce.topic_name());
}

bool CedMiddleware::create_publisher_by_xml(
        uint16_t publisher_id,
        uint16_t participant_id,
        const std::string &)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        auto it_publisher = publishers_.find(publisher_id);
        if (publishers_.end() == it_publisher)
        {
            publishers_.emplace(publisher_id, std::make_shared<CedPublisher>(it_participant->second));
            rv = true;
        }
    }
    return rv;
}

bool CedMiddleware::create_publisher_by_bin(
        uint16_t publisher_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Publisher_Binary& /* publisher_xrce */)
{
    return create_publisher_by_xml(publisher_id, participant_id, std::string());
}

bool CedMiddleware::create_subscriber_by_xml(
        uint16_t subscirber_id,
        uint16_t participant_id,
        const std::string &)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        auto it_subscriber = subscribers_.find(subscirber_id);
        if (subscribers_.end() == it_subscriber)
        {
            subscribers_.emplace(subscirber_id, std::make_shared<CedSubscriber>(it_participant->second));
            rv = true;
        }
    }
    return rv;
}

bool CedMiddleware::create_subscriber_by_bin(
        uint16_t subscriber_id,
        uint16_t participant_id,
        const dds::xrce::OBJK_Subscriber_Binary& /* subscriber_xrce */)
{
    return create_subscriber_by_xml(subscriber_id, participant_id, std::string());
}

bool CedMiddleware::create_datawriter_by_ref(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        auto it_datawriter = datawriters_.find(datawriter_id);
        if (datawriters_.end() == it_datawriter)
        {
            std::shared_ptr<CedTopic> topic =
                it_publisher->second->get_participant()->find_topic(remove_suffix_form_datawriter_ref(ref));
            rv = topic
                && datawriters_.emplace(
                    datawriter_id,
                    std::make_shared<CedDataWriter>(
                        it_publisher->second,
                        topic,
                        write_access_,
                        topics_src_)).second;
        }
    }
    return rv;
}

bool CedMiddleware::create_datawriter_by_xml(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& xml)
{
    return create_datawriter_by_ref(datawriter_id, publisher_id, xml);
}

bool CedMiddleware::create_datawriter_by_bin(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce)
{
    bool rv = false;
    auto it = topics_.find(eprosima::uxr::conversion::objectid_to_raw(datawriter_xrce.topic_id()));

     if (topics_.end() != it)
    {
        rv = create_datawriter_by_ref(datawriter_id, publisher_id, it->second->get_name());
    }

    return rv;
}

bool CedMiddleware::create_datareader_by_ref(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        auto it_datareader = datareaders_.find(datareader_id);
        if (datareaders_.end() == it_datareader)
        {
            std::shared_ptr<CedTopic> topic =
                it_subscriber->second->get_participant()->find_topic(remove_suffix_form_datareader_ref(ref));
            rv = topic
                && datareaders_.emplace(
                    datareader_id,
                    std::make_shared<CedDataReader>(
                        it_subscriber->second,
                        topic,
                        read_access_)).second;
        }
    }
    return rv;
}

bool CedMiddleware::create_datareader_by_xml(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& xml)
{
    return create_datareader_by_ref(datareader_id, subscriber_id, xml);
}

bool CedMiddleware::create_datareader_by_bin(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const dds::xrce::OBJK_DataReader_Binary& datareader_xrce)
{
    bool rv = false;
    auto it = topics_.find(eprosima::uxr::conversion::objectid_to_raw(datareader_xrce.topic_id()));

     if (topics_.end() != it)
    {
        rv = create_datareader_by_ref(datareader_id, subscriber_id, it->second->get_name());
    }

    return rv;
}

/**********************************************************************************************************************
 * Delete functions.
 **********************************************************************************************************************/
bool CedMiddleware::delete_participant(uint16_t participant_id)
{
    return (0 != participants_.erase(participant_id));
}

bool CedMiddleware::delete_topic(uint16_t topic_id)
{
    return (0 != topics_.erase(topic_id));
}

bool CedMiddleware::delete_publisher(uint16_t publisher_id)
{
    return (0 != publishers_.erase(publisher_id));
}

bool CedMiddleware::delete_subscriber(uint16_t subscriber_id)
{
    return (0 != subscribers_.erase(subscriber_id));
}

bool CedMiddleware::delete_datawriter(uint16_t datawriter_id)
{
    return (0 != datawriters_.erase(datawriter_id));
}

bool CedMiddleware::delete_datareader(uint16_t datareader_id)
{
    return (0 != datareaders_.erase(datareader_id));
}

/**********************************************************************************************************************
 * Write/Read functions.
 **********************************************************************************************************************/
bool CedMiddleware::write_data(
        uint16_t datawriter_id,
        const std::vector<uint8_t>& data)
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        uint8_t errcode;
        rv = it->second->write(data, errcode);
    }
    return rv;
}

bool CedMiddleware::read_data(
        uint16_t datareader_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        uint8_t errcode;
        rv = it->second->read(data, timeout, errcode);
    }
    return rv;
}

/**********************************************************************************************************************
 * Matched functions.
 **********************************************************************************************************************/
bool CedMiddleware::matched_participant_from_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& /*ref*/) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        rv = (domain_id == it->second->get_domain_id());
    }
    return rv;
}

bool CedMiddleware::matched_participant_from_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml) const
{
    return matched_participant_from_ref(participant_id, domain_id, xml);
}

bool CedMiddleware::matched_participant_from_bin(
        uint16_t participant_id,
        int16_t domain_id,
        const dds::xrce::OBJK_DomainParticipant_Binary& /* participant_xrce */) const
{
    return matched_participant_from_ref(participant_id, domain_id, std::string());
}

bool CedMiddleware::matched_topic_from_ref(
        uint16_t topic_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        rv = (remove_suffix_form_topic_ref(ref) == it->second->get_name());
    }
    return rv;
}

bool CedMiddleware::matched_topic_from_xml(
        uint16_t topic_id,
        const std::string& xml) const
{
    return matched_topic_from_ref(topic_id, xml);
}

bool CedMiddleware::matched_topic_from_bin(
            uint16_t topic_id,
            const dds::xrce::OBJK_Topic_Binary& topic_xrce) const
{
    return matched_topic_from_ref(topic_id, topic_xrce.topic_name());
}

bool CedMiddleware::matched_datawriter_from_ref(
        uint16_t datawriter_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        rv = (remove_suffix_form_datawriter_ref(ref) == it->second->topic_name());
    }
    return rv;
}

bool CedMiddleware::matched_datawriter_from_xml(
        uint16_t datawriter_id,
        const std::string& xml) const
{
    return matched_datawriter_from_ref(datawriter_id, xml);
}

bool CedMiddleware::matched_datawriter_from_bin(
            uint16_t datawriter_id,
            const dds::xrce::OBJK_DataWriter_Binary& datawriter_xrce) const
{
    bool rv = false;
    auto it = topics_.find(eprosima::uxr::conversion::objectid_to_raw(datawriter_xrce.topic_id()));

    if (topics_.end() != it)
    {
        rv = matched_datawriter_from_ref(datawriter_id, it->second->get_name());
    }

    return rv;
}

bool CedMiddleware::matched_datareader_from_ref(
        uint16_t datareader_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        rv = (remove_suffix_form_datareader_ref(ref) == it->second->topic_name());
    }
    return rv;
}

bool CedMiddleware::matched_datareader_from_xml(
        uint16_t datareader_id,
        const std::string& xml) const
{
    return matched_datareader_from_ref(datareader_id, xml);
}

bool CedMiddleware::matched_datareader_from_bin(
            uint16_t datareader_id,
            const dds::xrce::OBJK_DataReader_Binary&  datareader_xrce) const
{
    bool rv = false;
    auto it = topics_.find(eprosima::uxr::conversion::objectid_to_raw(datareader_xrce.topic_id()));

    if (topics_.end() != it)
    {
        rv = matched_datareader_from_ref(datareader_id, it->second->get_name());
    }

    return rv;
}

} // namespace uxr
} // namespace eprosima
