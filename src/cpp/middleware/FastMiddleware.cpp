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

#include <uxr/agent/middleware/FastMiddleware.hpp>

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>
#include "../xmlobjects/xmlobjects.h"

namespace eprosima {
namespace uxr {

bool FastMiddleware::create_participant_from_ref(uint16_t participant_id, const std::string& ref)
{
    bool rv = false;
    fastrtps::Participant* participant = fastrtps::Domain::createParticipant(ref, this);
    if (nullptr != participant)
    {
        participants_.insert(std::make_pair(participant_id, std::move(participant)));
        rv = true;
    }
    return rv;
}

bool FastMiddleware::create_participant_from_xml(uint16_t participant_id, const std::string& xml)
{
    bool rv = false;
    fastrtps::ParticipantAttributes attributes;
    if (xmlobjects::parse_participant(xml.data(), xml.size(), attributes))
    {
        fastrtps::Participant* participant = fastrtps::Domain::createParticipant(attributes, this);
        if (nullptr != participant)
        {
            participants_.insert(std::make_pair(participant_id, std::move(participant)));
            rv = true;
        }
    }
    return rv;
}

bool FastMiddleware::create_topic_from_ref(uint16_t topic_id, uint16_t participant_id, const std::string& ref)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        fastrtps::TopicAttributes attributes;
        if (fastrtps::xmlparser::XMLP_ret::XML_OK ==
                fastrtps::xmlparser::XMLProfileManager::fillTopicAttributes(ref, attributes))
        {
            TopicPubSubType* topic = new TopicPubSubType(false);
            topic->setName(attributes.getTopicDataType().data());
            topic->m_isGetKeyDefined = (attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
            if (fastrtps::Domain::registerType(it_participant->second, topic))
            {
                topics_.insert(std::make_pair(topic_id, topic));
                register_topic(topic->getName(), topic_id);
                rv = true;
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_topic_from_xml(uint16_t topic_id, uint16_t participant_id, const std::string& xml)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        fastrtps::TopicAttributes attributes;
        if (xmlobjects::parse_topic(xml.data(), xml.size(), attributes))
        {
            TopicPubSubType* topic = new TopicPubSubType(false);
            topic->setName(attributes.getTopicDataType().data());
            topic->m_isGetKeyDefined = (attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
            if (fastrtps::Domain::registerType(it_participant->second, topic))
            {
                topics_.insert(std::make_pair(topic_id, topic));
                register_topic(topic->getName(), topic_id);
                rv = true;
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_publisher_from_ref(uint16_t publisher_id, uint16_t participant_id, const std::string&)
{
    publishers_.emplace(std::make_pair(publisher_id, participant_id));
    return true;
}

bool FastMiddleware::create_publisher_from_xml(uint16_t publisher_id, uint16_t participant_id, const std::string&)
{
    publishers_.emplace(std::make_pair(publisher_id, participant_id));
    return true;
}

bool FastMiddleware::create_subcriber_from_ref(uint16_t subscriber_id, uint16_t participant_id, const std::string&)
{
    subscribers_.emplace(std::make_pair(subscriber_id, participant_id));
    return true;
}

bool FastMiddleware::create_subcriber_from_xml(uint16_t subscriber_id, uint16_t participant_id, const std::string&)
{
    subscribers_.emplace(std::make_pair(subscriber_id, participant_id));
    return true;
}

bool FastMiddleware::create_datawriter_from_ref(uint16_t datawriter_id,
                                                uint16_t publisher_id,
                                                const std::string& ref,
                                                uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        auto it_participant = participants_.find(it_publisher->second.get_participant_id());
        if (participants_.end() != it_participant)
        {
            fastrtps::Publisher* datawriter = fastrtps::Domain::createPublisher(it_participant->second, ref, this);
            if (nullptr != datawriter)
            {
                const std::string& topic_name = datawriter->getAttributes().topic.getTopicDataType();
                if (check_register_topic(topic_name, associated_topic_id))
                {
                    datawriters_.insert(std::make_pair(datawriter_id, datawriter));
                    rv = true;
                }
                else
                {
                    fastrtps::Domain::removePublisher(datawriter);
                }
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_datawriter_from_xml(uint16_t datawriter_id,
                                                uint16_t publisher_id,
                                                const std::string& xml,
                                                uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_publisher = publishers_.find(publisher_id);
    if (publishers_.end() != it_publisher)
    {
        auto it_participant = participants_.find(it_publisher->second.get_participant_id());
        if (participants_.end() != it_participant)
        {
            fastrtps::PublisherAttributes attributes;
            if (xmlobjects::parse_publisher(xml.data(), xml.size(), attributes))
            {
                fastrtps::Publisher* datawriter = fastrtps::Domain::createPublisher(it_participant->second, attributes, this);
                if (nullptr != datawriter)
                {
                    const std::string& topic_name = datawriter->getAttributes().topic.getTopicDataType();
                    if (check_register_topic(topic_name, associated_topic_id))
                    {
                        datawriters_.insert(std::make_pair(datawriter_id, datawriter));
                        rv = true;
                    }
                    else
                    {
                        fastrtps::Domain::removePublisher(datawriter);
                    }
                }
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_datareader_from_ref(uint16_t datareader_id,
                                                uint16_t subscriber_id,
                                                const std::string& ref,
                                                uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        auto it_participant = participants_.find(it_subscriber->second.get_participant_id());
        if (participants_.end() != it_participant)
        {
            fastrtps::Subscriber* datareader = fastrtps::Domain::createSubscriber(it_participant->second, ref, this);
            if (nullptr != datareader)
            {
                const std::string& topic_name = datareader->getAttributes().topic.getTopicDataType();
                if (check_register_topic(topic_name, associated_topic_id))
                {
                    datareaders_.insert(std::make_pair(datareader_id, datareader));
                    rv = true;
                }
                else
                {
                    fastrtps::Domain::removeSubscriber(datareader);
                }
            }
        }
    }
    return rv;
}

bool FastMiddleware::create_datareader_from_xml(uint16_t datareader_id,
                                                uint16_t subscriber_id,
                                                const std::string& xml,
                                                uint16_t& associated_topic_id)
{
    bool rv = false;
    auto it_subscriber = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it_subscriber)
    {
        auto it_participant = participants_.find(it_subscriber->second.get_participant_id());
        if (participants_.end() != it_participant)
        {
            fastrtps::SubscriberAttributes attributes;
            if (xmlobjects::parse_subscriber(xml.data(), xml.size(), attributes))
            {
                fastrtps::Subscriber* datareader = fastrtps::Domain::createSubscriber(it_participant->second, attributes, this);
                if (nullptr != datareader)
                {
                    const std::string& topic_name = datareader->getAttributes().topic.getTopicDataType();
                    if (check_register_topic(topic_name, associated_topic_id))
                    {
                        datareaders_.insert(std::make_pair(datareader_id, datareader));
                        rv = true;
                    }
                    else
                    {
                        fastrtps::Domain::removeSubscriber(datareader);
                    }
                }
            }
        }
    }
    return rv;
}

bool FastMiddleware::delete_participant(uint16_t participant_id)
{
    bool rv = false;
    auto it = participants_.find(participant_id);
    if (participants_.end() != it)
    {
        if (fastrtps::Domain::removeParticipant(it->second))
        {
            participants_.erase(it);
            rv = true;
        }
    }
    return rv;
}

bool FastMiddleware::delete_topic(uint16_t topic_id, uint16_t participant_id)
{
    bool rv = false;
    auto it_participant = participants_.find(participant_id);
    if (participants_.end() != it_participant)
    {
        auto it_topic = topics_.find(topic_id);
        if (topics_.end() != it_topic)
        {
            if (fastrtps::Domain::unregisterType(it_participant->second, it_topic->second->getName()))
            {
                unregister_topic(it_topic->second->getName());
                rv = true;
            }
        }
    }
    return rv;
}

bool FastMiddleware::delete_publisher(uint16_t publisher_id, uint16_t participant_id)
{
    (void) participant_id;
    bool rv = false;
    auto it = publishers_.find(publisher_id);
    if (publishers_.end() != it)
    {
        publishers_.erase(it);
        rv = true;
    }
    return rv;
}

bool FastMiddleware::delete_subcriber(uint16_t subscriber_id, uint16_t participant_id)
{
    (void) participant_id;
    bool rv = false;
    auto it = subscribers_.find(subscriber_id);
    if (subscribers_.end() != it)
    {
        subscribers_.erase(it);
        rv = true;
    }
    return rv;
}

bool FastMiddleware::delete_datawriter(uint16_t datawriter_id, uint16_t publisher_id)
{
    (void) publisher_id;
    bool rv = false;
    auto it = datawriters_.find(datawriter_id);
    if (datawriters_.end() != it)
    {
        if (fastrtps::Domain::removePublisher(it->second))
        {
            datawriters_.erase(it);
        }
    }
    return rv;
}

bool FastMiddleware::delete_datareader(uint16_t datareader_id, uint16_t subscriber_id)
{
    (void) subscriber_id;
    bool rv = false;
    auto it = datareaders_.find(datareader_id);
    if (datareaders_.end() != it)
    {
        if (fastrtps::Domain::removeSubscriber(it->second))
        {
            datareaders_.erase(it);
        }
    }
    return rv;
}

bool FastMiddleware::write_data(uint16_t datawriter_id, uint8_t *buf, size_t len)
{
    (void) datawriter_id;
    (void) buf;
    (void) len;
    return true;
}

bool FastMiddleware::read_data(uint16_t datareader_id)
{
    (void) datareader_id;
    return true;
}

void FastMiddleware::register_topic(const std::string& topic_name, uint16_t topic_id)
{
    registered_topics_[topic_name] = topic_id;
}

void FastMiddleware::unregister_topic(const std::string& topic_name)
{
    registered_topics_.erase(topic_name);
}

bool FastMiddleware::check_register_topic(const std::string& topic_name, uint16_t& topic_id)
{
    bool rv = false;
    auto it = registered_topics_.find(topic_name);
    if (registered_topics_.end() != it)
    {
        topic_id = it->second;
        rv = true;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
