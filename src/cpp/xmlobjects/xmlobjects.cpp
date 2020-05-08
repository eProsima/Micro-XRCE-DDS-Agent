// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "xmlobjects.h"

#include <fastrtps/attributes/all_attributes.h>
#include <fastrtps/attributes/ReplierAttributes.hpp>
#include <fastrtps/attributes/RequesterAttributes.hpp>
#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/xmlparser/XMLTree.h>

using eprosima::fastrtps::ParticipantAttributes;
using eprosima::fastrtps::PublisherAttributes;
using eprosima::fastrtps::SubscriberAttributes;
using eprosima::fastrtps::TopicAttributes;
using eprosima::fastrtps::RequesterAttributes;
using eprosima::fastrtps::ReplierAttributes;
using eprosima::fastrtps::xmlparser::BaseNode;
using eprosima::fastrtps::xmlparser::DataNode;
using eprosima::fastrtps::xmlparser::NodeType;
using eprosima::fastrtps::xmlparser::XMLP_ret;
using eprosima::fastrtps::xmlparser::XMLParser;

bool eprosima::uxr::xmlobjects::parse_participant(const char* source, std::size_t source_size,
                                                        ParticipantAttributes& participant)
{
    bool ret = false;
    std::unique_ptr<BaseNode> root;
    if (XMLParser::loadXML(source, source_size, root) == XMLP_ret::XML_OK)
    {
        for (const auto& profile : root->getChildren())
        {
            if (profile->getType() == NodeType::PARTICIPANT)
            {
                participant = *(dynamic_cast<DataNode<ParticipantAttributes>*>(profile.get())->get());
                ret         = true;
            }
        }
    }
    return ret;
}

bool eprosima::uxr::xmlobjects::parse_publisher(const char* source, size_t source_size,
                                                      PublisherAttributes& publisher)
{
    bool ret = false;
    std::unique_ptr<BaseNode> root;
    if (XMLParser::loadXML(source, source_size, root) == XMLP_ret::XML_OK)
    {
        for (const auto& profile : root->getChildren())
        {
            if (profile->getType() == NodeType::PUBLISHER)
            {
                publisher = *(dynamic_cast<DataNode<PublisherAttributes>*>(profile.get())->get());
                ret       = true;
            }
        }
    }
    return ret;
}

bool eprosima::uxr::xmlobjects::parse_subscriber(const char* source, size_t source_size,
                                                       SubscriberAttributes& subscriber)
{
    bool ret = false;
    std::unique_ptr<BaseNode> root;
    if (XMLParser::loadXML(source, source_size, root) == XMLP_ret::XML_OK)
    {
        for (const auto& profile : root->getChildren())
        {
            if (profile->getType() == NodeType::SUBSCRIBER)
            {
                subscriber = *(dynamic_cast<DataNode<SubscriberAttributes>*>(profile.get())->get());
                ret        = true;
            }
        }
    }
    return ret;
}

bool eprosima::uxr::xmlobjects::parse_topic(const char* source, std::size_t source_size, TopicAttributes& topic)
{
    bool ret = false;
    std::unique_ptr<BaseNode> root;
    if (XMLParser::loadXML(source, source_size, root) == XMLP_ret::XML_OK)
    {
        for (const auto& profile : root->getChildren())
        {
            if (profile->getType() == NodeType::TOPIC)
            {
                topic = *(dynamic_cast<DataNode<TopicAttributes>*>(profile.get())->get());
                ret   = true;
            }
        }
    }
    return ret;
}

bool eprosima::uxr::xmlobjects::parse_requester(
        const char* source,
        std::size_t source_size,
        RequesterAttributes& requester)
{
    bool ret = false;
    std::unique_ptr<BaseNode> root;
    if (XMLParser::loadXML(source, source_size, root) == XMLP_ret::XML_OK)
    {
        for (const auto& profile : root->getChildren())
        {
            if (profile->getType() == NodeType::REQUESTER)
            {
                requester = *(dynamic_cast<DataNode<RequesterAttributes>*>(profile.get())->get());
                ret   = true;
            }
        }
    }
    return ret;
}

bool eprosima::uxr::xmlobjects::parse_replier(
        const char* source,
        std::size_t source_size,
        ReplierAttributes& replier)
{
    bool ret = false;
    std::unique_ptr<BaseNode> root;
    if (XMLParser::loadXML(source, source_size, root) == XMLP_ret::XML_OK)
    {
        for (const auto& profile : root->getChildren())
        {
            if (profile->getType() == NodeType::REPLIER)
            {
                replier = *(dynamic_cast<DataNode<ReplierAttributes>*>(profile.get())->get());
                ret   = true;
            }
        }
    }
    return ret;
}