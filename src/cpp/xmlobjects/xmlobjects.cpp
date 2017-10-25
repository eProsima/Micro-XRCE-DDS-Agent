#include "xmlobjects.h"

#include <fastrtps/attributes/all_attributes.h>
#include <fastrtps/xmlparser/XMLParser.h>
#include <fastrtps/xmlparser/XMLTree.h>

using eprosima::fastrtps::ParticipantAttributes;
using eprosima::fastrtps::PublisherAttributes;
using eprosima::fastrtps::SubscriberAttributes;
using eprosima::fastrtps::xmlparser::BaseNode;
using eprosima::fastrtps::xmlparser::DataNode;
using eprosima::fastrtps::xmlparser::NodeType;
using eprosima::fastrtps::xmlparser::XMLP_ret;
using eprosima::fastrtps::xmlparser::XMLParser;

bool eprosima::micrortps::xmlobjects::parse_participant(const char* source, std::size_t source_size, ParticipantAttributes& participant)
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

bool eprosima::micrortps::xmlobjects::parse_publisher(const char* source, size_t source_size, PublisherAttributes& publisher)
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

bool eprosima::micrortps::xmlobjects::parse_subscriber(const char* source, size_t source_size, SubscriberAttributes& subscriber)
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