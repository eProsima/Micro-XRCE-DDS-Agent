#include <agent/topic/Topic.hpp>

#include <xmlobjects/xmlobjects.h>

#include <fastrtps/Domain.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace micrortps {

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"

Topic::Topic(ObjectId id, fastrtps::Participant* rtps_participant) : XRCEObject{id}, rtps_participant_(rtps_participant)
{
}

Topic::~Topic()
{
    if (nullptr != rtps_participant_)
    {
        fastrtps::Domain::unregisterType(rtps_participant_, generic_type_.getName());
    }
}

bool Topic::init()
{
    // if (nullptr == rtps_participant_ &&
    //     nullptr == (rtps_participant_ = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)))
    // {
    //     return false;
    // }


    // topic_type_.setName(attributes.topic.getTopicDataType().data());
    // fastrtps::Domain::registerType(rtps_participant_, &topic_type_);

    // if (!m_rtps_publisher_prof.empty())
    // {
    //     mp_rtps_publisher = fastrtps::Domain::createPublisher(rtps_participant_, m_rtps_publisher_prof, nullptr);
    // }
    // else
    // {
    //     // std::cout << "init DataWriter RTPS default publisher" << std::endl;
    //     mp_rtps_publisher =
    //         fastrtps::Domain::createPublisher(rtps_participant_, DEFAULT_XRCE_PUBLISHER_PROFILE, nullptr);
    // }

    // if (mp_rtps_publisher == nullptr)
    // {
    //     std::cout << "init publisher error" << std::endl;
    //     return false;
    // }
    // return true;
}

bool Topic::init(const std::string& xmlrep)
{
    if (nullptr == rtps_participant_ &&
        nullptr == (rtps_participant_ = fastrtps::Domain::createParticipant(DEFAULT_XRCE_PARTICIPANT_PROFILE)))
    {
        return false;
    }

    TopicAttributes attributes;
    if (xmlobjects::parse_topic(xmlrep.data(), xmlrep.size(), attributes))
    {
        generic_type_.setName(attributes.getTopicDataType().data());
        return fastrtps::Domain::registerType(rtps_participant_, &generic_type_);
    }
    else
    {
        return false;
    }
}
} // namespace micrortps
} // namespace eprosima