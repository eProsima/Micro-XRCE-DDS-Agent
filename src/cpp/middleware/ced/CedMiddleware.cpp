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
        const std::string& /*xml*/)
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
            std::shared_ptr<CedGlobalTopic> global_topic;
            if (it_participant->second->register_topic(ref, topic_id, global_topic)) // TODO: get reference.
            {
                topics_.emplace(topic_id, std::make_shared<CedTopic>(it_participant->second, global_topic));
                rv = true;
            }
        }
    }
    return rv;
}

bool CedMiddleware::create_topic_by_xml(
        uint16_t topic_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        auto it_topic = topics_.find(topic_id);
        if (topics_.end() == it_topic)
        {
            std::shared_ptr<CedGlobalTopic> global_topic;
            if (it_participant->second->register_topic(xml, topic_id, global_topic)) // TODO: parse XML.
            {
                topics_.emplace(topic_id, std::make_shared<CedTopic>(it_participant->second, global_topic));
                rv = true;
            }
        }
    }
    return rv;
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

bool CedMiddleware::create_datawriter_by_ref(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& ref,
        uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        auto it_datawriter = datawriters_.find(datawriter_id);
        if (datawriters_.end() == it_datawriter)
        {
            uint16_t topic_id;
            if (it_publisher->second->get_participant()->find_topic(ref, topic_id)) // TODO: get reference.
            {
                auto it_topic = topics_.find(topic_id);
                if (topics_.end() != it_topic)
                {
                    datawriters_.emplace(
                        datawriter_id,
                        std::make_shared<CedDataWriter>(
                            it_publisher->second,
                            it_topic->second,
                            write_access_,
                            topics_src_));

                    associated_topic_id = topic_id;
                    rv = true;
                }
            }
        }
    }
    return rv;
}

bool CedMiddleware::create_datawriter_by_xml(
        uint16_t datawriter_id,
        uint16_t publisher_id,
        const std::string& xml,
        uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        auto it_datawriter = datawriters_.find(datawriter_id);
        if (datawriters_.end() == it_datawriter)
        {
            uint16_t topic_id;
            if (it_publisher->second->get_participant()->find_topic(xml, topic_id)) // TODO: parse XML.
            {
                auto it_topic = topics_.find(topic_id);
                if (topics_.end() != it_topic)
                {
                    datawriters_.emplace(
                        datawriter_id,
                        std::make_shared<CedDataWriter>(
                            it_publisher->second,
                            it_topic->second,
                            write_access_,
                            topics_src_));

                    associated_topic_id = topic_id;
                    rv = true;
                }
            }
        }
    }
    return rv;
}

bool CedMiddleware::create_datareader_by_ref(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& ref,
        uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        auto it_datareader = datareaders_.find(datareader_id);
        if (datareaders_.end() == it_datareader)
        {
            uint16_t topic_id;
            if (it_subscriber->second->get_participant()->find_topic(ref, topic_id)) // TODO: get reference.
            {
                auto it_topic = topics_.find(topic_id);
                if (topics_.end() != it_topic)
                {
                    datareaders_.emplace(
                        datareader_id,
                        std::make_shared<CedDataReader>(
                            it_subscriber->second,
                            it_topic->second,
                            read_access_));

                    associated_topic_id = topic_id;
                    rv = true;
                }
            }
        }
    }
    return rv;
}

bool CedMiddleware::create_datareader_by_xml(
        uint16_t datareader_id,
        uint16_t subscriber_id,
        const std::string& xml,
        uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        auto it_datareader = datareaders_.find(datareader_id);
        if (datareaders_.end() == it_datareader)
        {
            uint16_t topic_id;
            if (it_subscriber->second->get_participant()->find_topic(xml, topic_id)) // TODO: parse XML.
            {
                auto it_topic = topics_.find(topic_id);
                if (topics_.end() != it_topic)
                {
                    datareaders_.emplace(
                        datareader_id,
                        std::make_shared<CedDataReader>(
                            it_subscriber->second,
                            it_topic->second,
                            read_access_));

                    associated_topic_id = topic_id;
                    rv = true;
                }
            }
        }
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
        const std::string& /*xml*/) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        rv = (domain_id == it->second->get_domain_id());
    }
    return rv;
}

bool CedMiddleware::matched_topic_from_ref(
        uint16_t topic_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        rv = (ref == it->second->name()); // TODO: get reference.
    }
    return rv;
}

bool CedMiddleware::matched_topic_from_xml(
        uint16_t topic_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        rv = (xml == it->second->name()); // TODO: parse XML.
    }
    return rv;
}

bool CedMiddleware::matched_datawriter_from_ref(
        uint16_t datawriter_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        rv = (ref == it->second->topic_name()); // TODO: get reference.
    }
    return rv;
}

bool CedMiddleware::matched_datawriter_from_xml(
        uint16_t datawriter_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        rv = (xml == it->second->topic_name()); // TODO: parse XML.
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
        rv = (ref == it->second->topic_name()); // TODO: parse XML.
    }
    return rv;
}

bool CedMiddleware::matched_datareader_from_xml(
        uint16_t datareader_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        rv = (xml == it->second->topic_name()); // TODO: parse XML.
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
