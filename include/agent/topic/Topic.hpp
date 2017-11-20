#ifndef _TOPIC_H
#define _TOPIC_H

#include <agent/XRCEObject.hpp>
#include <agent/types/TopicPubSubType.h>

#include <string>

namespace eprosima {

namespace fastrtps {
class Participant;
} // namespace fastrtps

namespace micrortps {

class Topic : public XRCEObject
{
  public:
    Topic(ObjectId id, fastrtps::Participant* rtps_participant);
    Topic(Topic&&)      = default;
    Topic(const Topic&) = default;
    Topic& operator=(Topic&&) = default;
    Topic& operator=(const Topic&) = default;
    ~Topic() override;

    bool init();
    bool init(const std::string& xmlrep);

  private:
    std::string name;
    std::string type_name;

    fastrtps::Participant* rtps_participant_;
    TopicPubSubType generic_type_;
};
} // namespace micrortps
} // namespace eprosima
#endif // !_TOPIC_H