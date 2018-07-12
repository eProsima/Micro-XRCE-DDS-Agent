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

#ifndef _MICRORTPS_AGENT_TOPIC_TOPIC_HPP_
#define _MICRORTPS_AGENT_TOPIC_TOPIC_HPP_

#include <micrortps/agent/object/XRCEObject.hpp>
#include <micrortps/agent/types/TopicPubSubType.hpp>

#include <string>

namespace eprosima {

namespace fastrtps {
class Participant;
} // namespace fastrtps

namespace micrortps {

class Topic : public XRCEObject
{
  public:
    Topic(const dds::xrce::ObjectId& id, fastrtps::Participant& rtps_participant);
    Topic(Topic&&)      = default;
    Topic(const Topic&) = default;
    Topic& operator=(Topic&&) = default;
    Topic& operator=(const Topic&) = default;
    ~Topic() override;

    bool init(const std::string& xmlrep);

  private:
    std::string name;
    std::string type_name;

    fastrtps::Participant& rtps_participant_;
    TopicPubSubType generic_type_;
};
} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TOPIC_TOPIC_HPP_
