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

#include <uxr/agent/middleware/fast/FastMiddleware.hpp>

#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../../xmlobjects/xmlobjects.h"

namespace eprosima {
namespace uxr {

FastMiddleware::FastMiddleware()
{
}

/**********************************************************************************************************************
 * Create functions.
 **********************************************************************************************************************/
bool FastMiddleware::create_participant_by_ref(uint16_t participant_id, int16_t domain_id, const std::string& ref)
{
    bool rv = false;
    std::shared_ptr<FastParticipant> participant(new FastParticipant(domain_id));
    if (participant->create_by_ref(ref))
    {
        participants_.emplace(participant_id, std::move(participant));
        rv = true;
    }
    return rv;
}

bool FastMiddleware::create_participant_by_xml(uint16_t participant_id, int16_t domain_id, const std::string& xml)
{
    (void) domain_id;
    bool rv = false;
    fastrtps::ParticipantAttributes attributes;
    if (xmlobjects::parse_participant(xml.data(), xml.size(), attributes))
    {
        attributes.rtps.builtin.domainId = uint32_t(domain_id);
        std::shared_ptr<FastParticipant> participant(new FastParticipant(domain_id));
        if (participant->create_by_attributes(attributes))
        {
            participants_.emplace(participant_id, std::move(participant));
            rv = true;
        }
    }
    return rv;
}

bool FastMiddleware::create_topic_by_ref(uint16_t topic_id, uint16_t participant_id, const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        fastrtps::TopicAttributes attributes;
        if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
                fastrtps::xmlparser::XMLProfileManager::fillTopicAttributes(ref, attributes))
        {
            std::shared_ptr<FastTopic> topic(new FastTopic(it_participant->second));
            if (topic->create_by_attributes(attributes, topic_id))
            {
                topics_.emplace(topic_id, std::move(topic));
                rv = true;
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_topic_by_xml(uint16_t topic_id, uint16_t participant_id, const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        fastrtps::TopicAttributes attributes;
        if (xmlobjects::parse_topic(xml.data(), xml.size(), attributes))
        {
            std::shared_ptr<FastTopic> topic(new FastTopic(it_participant->second));
            if (topic->create_by_attributes(attributes, topic_id))
            {
                topics_.emplace(topic_id, std::move(topic));
                rv = true;
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_publisher_by_xml(uint16_t publisher_id, uint16_t participant_id, const std::string&)
{
    std::shared_ptr<FastPublisher> publisher(new FastPublisher(participant_id));
    publishers_.emplace(publisher_id, std::move(publisher));
    return true;
}

bool FastMiddleware::create_subscriber_by_xml(uint16_t subscriber_id, uint16_t participant_id, const std::string&)
{
    std::shared_ptr<FastSubscriber> subscriber(new FastSubscriber(participant_id));
    subscribers_.emplace(subscriber_id, std::move(subscriber));
    return true;
}

bool FastMiddleware::create_datawriter_by_ref(uint16_t datawriter_id,
                                                uint16_t publisher_id,
                                                const std::string& ref,
                                                uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        auto it_participant = participants_.find(it_publisher->second->get_participant_id());
        if (participants_.end() != it_participant)
        {
            std::shared_ptr<FastDataWriter> datawriter(new FastDataWriter(it_participant->second));
            if (datawriter->create_by_ref(ref, associated_topic_id))
            {
                datawriters_.emplace(datawriter_id, std::move(datawriter));
                rv = true;
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_datawriter_by_xml(uint16_t datawriter_id,
                                                uint16_t publisher_id,
                                                const std::string& xml,
                                                uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        auto it_participant = participants_.find(it_publisher->second->get_participant_id());
        if (participants_.end() != it_participant)
        {
            fastrtps::PublisherAttributes attributes;
            if (xmlobjects::parse_publisher(xml.data(), xml.size(), attributes))
            {
                std::shared_ptr<FastDataWriter> datawriter(new FastDataWriter(it_participant->second));
                if (datawriter->create_by_attributes(attributes, associated_topic_id))
                {
                    datawriters_.emplace(datawriter_id, std::move(datawriter));
                    rv = true;
                }
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_requester_by_ref(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastRequester> requester(new FastRequester(it_participant->second));
        if (requester->create_by_ref(ref))
        {
            requesters_.emplace(requester_id, std::move(requester));
            rv = true;
        }
    }
    return rv;
}

bool FastMiddleware::create_requester_by_xml(
        uint16_t requester_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        fastrtps::RequesterAttributes attrs;
        if (xmlobjects::parse_requester(xml.data(), xml.size(), attrs))
        {
            std::shared_ptr<FastRequester> requester(new FastRequester(it_participant->second));
            if (requester->create_by_attributes(attrs))
            {
                requesters_.emplace(requester_id, std::move(requester));
                rv = true;
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_replier_by_ref(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        std::shared_ptr<FastReplier> replier(new FastReplier(it_participant->second));
        if (replier->create_by_ref(ref))
        {
            repliers_.emplace(replier_id, std::move(replier));
            rv = true;
        }
    }
    return rv;
}

bool FastMiddleware::create_replier_by_xml(
        uint16_t replier_id,
        uint16_t participant_id,
        const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        fastrtps::ReplierAttributes attrs;
        if (xmlobjects::parse_replier(xml.data(), xml.size(), attrs))
        {
            std::shared_ptr<FastReplier> replier(new FastReplier(it_participant->second));
            if (replier->create_by_attributes(attrs))
            {
                repliers_.emplace(replier_id, std::move(replier));
                rv = true;
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_datareader_by_ref(uint16_t datareader_id,
                                                uint16_t subscriber_id,
                                                const std::string& ref,
                                                uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        auto it_participant = participants_.find(it_subscriber->second->get_participant_id());
        if (participants_.end() != it_participant)
        {
            std::shared_ptr<FastDataReader> datareader(new FastDataReader(it_participant->second));
            std::string topic_name;
            if (datareader->create_by_ref(ref, associated_topic_id))
            {
                datareaders_.emplace(datareader_id, std::move(datareader));
                rv = true;
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_datareader_by_xml(uint16_t datareader_id,
                                                uint16_t subscriber_id,
                                                const std::string& xml,
                                                uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        auto it_participant = participants_.find(it_subscriber->second->get_participant_id());
        if (participants_.end() != it_participant)
        {
            fastrtps::SubscriberAttributes attributes;
            if (xmlobjects::parse_subscriber(xml.data(), xml.size(), attributes))
            {
                std::shared_ptr<FastDataReader> datareader(new FastDataReader(it_participant->second));
                if (datareader->create_by_attributes(attributes, associated_topic_id))
                {
                    datareaders_.emplace(datareader_id, std::move(datareader));
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
bool FastMiddleware::delete_participant(uint16_t participant_id)
{
    return (0 != participants_.erase(participant_id));
}

bool FastMiddleware::delete_topic(uint16_t topic_id)
{
    return (0 != topics_.erase(topic_id));
}

bool FastMiddleware::delete_publisher(uint16_t publisher_id)
{
    return (0 != publishers_.erase(publisher_id));
}

bool FastMiddleware::delete_subscriber(uint16_t subscriber_id)
{
    return (0 != subscribers_.erase(subscriber_id));
}

bool FastMiddleware::delete_datawriter(uint16_t datawriter_id)
{
    return (0 != datawriters_.erase(datawriter_id));
}

bool FastMiddleware::delete_datareader(uint16_t datareader_id)
{
    return (0 != datareaders_.erase(datareader_id));
}

bool FastMiddleware::delete_requester(uint16_t requester_id)
{
    return (0 != requesters_.erase(requester_id));
}

bool FastMiddleware::delete_replier(uint16_t replier_id)
{
    return false;
}

/**********************************************************************************************************************
 * Write/Read functions.
 **********************************************************************************************************************/
bool FastMiddleware::write_data(
        uint16_t datawriter_id,
        const std::vector<uint8_t>& data)
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        rv = it->second->write(data);
    }
    return rv;
}

bool FastMiddleware::write_request(
        uint16_t requester_id,
        uint32_t sequence_number,
        const std::vector<uint8_t>& data)
{
    bool rv = false;
    auto it = requesters_.find(requester_id);
    if (requesters_.end() != it)
    {
        rv = it->second->write(sequence_number, data);
    }
    return rv;
}

bool FastMiddleware::write_reply(
        uint16_t replier_id,
        const std::vector<uint8_t>& data)
{
    bool rv = false;
    auto it = repliers_.find(replier_id);
    if (repliers_.end() != it)
    {
        rv = it->second->write(data);
    }
    return rv;
}

bool FastMiddleware::read_data(
        uint16_t datareader_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        rv = it->second->read(data, timeout);
    }
    return rv;
}

bool FastMiddleware::read_request(
        uint16_t replier_id,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    auto it = repliers_.find(replier_id);
    if (repliers_.end() != it)
    {
        rv = it->second->read(data, timeout);
    }
    return rv;
}

bool FastMiddleware::read_reply(
        uint16_t requester_id,
        uint32_t& sequence_number,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout)
{
    bool rv = false;
    auto it = requesters_.find(requester_id);
    if (requesters_.end() != it)
    {
        rv = it->second->read(sequence_number, data, timeout);
    }
    return rv;
}

/**********************************************************************************************************************
 * Matched functions.
 **********************************************************************************************************************/
bool FastMiddleware::matched_participant_from_ref(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& ref) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        rv = (domain_id == it->second->domain_id()) && (it->second->match_from_ref(ref));
    }
    return rv;
}

bool FastMiddleware::matched_participant_from_xml(
        uint16_t participant_id,
        int16_t domain_id,
        const std::string& xml) const
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        rv = (domain_id == it->second->domain_id()) && (it->second->match_from_xml(xml));
    }
    return rv;
}

bool FastMiddleware::matched_topic_from_ref(uint16_t topic_id, const std::string& ref) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        rv = it->second->match_from_ref(ref);
    }
    return rv;
}

bool FastMiddleware::matched_topic_from_xml(uint16_t topic_id, const std::string& xml) const
{
    bool rv = false;
    auto it = topics_.find(topic_id);
    if (topics_.end() != it)
    {
        rv = it->second->match_from_xml(xml);
    }
    return rv;
}

bool FastMiddleware::matched_datawriter_from_ref(uint16_t datawriter_id, const std::string& ref) const
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        rv = it->second->match_from_ref(ref);
    }
    return rv;
}

bool FastMiddleware::matched_datawriter_from_xml(uint16_t datawriter_id, const std::string& xml) const
{
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        rv = it->second->match_from_xml(xml);
    }
    return rv;
}

bool FastMiddleware::matched_datareader_from_ref(uint16_t datareader_id, const std::string& ref) const
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        rv = it->second->match_from_ref(ref);
    }
    return rv;
}

bool FastMiddleware::matched_datareader_from_xml(uint16_t datareader_id, const std::string& xml) const
{
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        rv = it->second->match_from_xml(xml);
    }
    return rv;
}

bool FastMiddleware::matched_requester_from_ref(
        uint16_t requester_id,
        const std::string& ref) const
{
    return false;
}

bool FastMiddleware::matched_requester_from_xml(
        uint16_t requester_id,
        const std::string& xml) const
{
    return false;
}

bool FastMiddleware::matched_replier_from_ref(
        uint16_t requester_id,
        const std::string& ref) const
{
    return false;
}

bool FastMiddleware::matched_replier_from_xml(
        uint16_t requester_id,
        const std::string& xml) const
{
    return false;
}

} // namespace uxr
} // namespace eprosima
