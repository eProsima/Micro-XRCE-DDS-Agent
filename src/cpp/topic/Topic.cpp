#include <agent/topic/Topic.hpp>

#include <xmlobjects/xmlobjects.h>

#include <fastrtps/Domain.h>
#include <fastrtps/xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace micrortps {

#define DEFAULT_XRCE_PARTICIPANT_PROFILE "default_xrce_participant_profile"

Topic::Topic(ObjectId id, fastrtps::Participant* rtps_participant) : XRCEObject{id}, rtps_participant_(rtps_participant), generic_type_(false)
{
}

Topic::~Topic()
{
    if (nullptr != rtps_participant_)
    {
        fastrtps::Domain::unregisterType(rtps_participant_, generic_type_.getName());
    }
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
        generic_type_.m_isGetKeyDefined = (attributes.getTopicKind() == fastrtps::rtps::TopicKind_t::WITH_KEY);
        return fastrtps::Domain::registerType(rtps_participant_, &generic_type_);
    }
    else
    {
        return false;
    }
}
} // namespace micrortps
} // namespace eprosima