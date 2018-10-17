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

#ifndef _UXR_AGENT_PARTICIPANT_HPP_
#define _UXR_AGENT_PARTICIPANT_HPP_

#include <uxr/agent/object/XRCEObject.hpp>
#include <unordered_map>
#include <string>
#include <set>

namespace eprosima {
namespace fastrtps {
class Participant;
}
}

#include <fastrtps/participant/ParticipantDiscoveryInfo.h>
#include <fastrtps/participant/ParticipantListener.h>

namespace eprosima {
namespace uxr {

class Participant : public XRCEObject, public fastrtps::ParticipantListener
{
public:
    Participant(const dds::xrce::ObjectId& id);
    virtual ~Participant();

    bool init(const dds::xrce::OBJK_PARTICIPANT_Representation& representation);
    fastrtps::Participant* get_rtps_participant() { return rtps_participant_; }
    void register_topic(const std::string& topic_name, const dds::xrce::ObjectId& object_id);
    void unregister_topic(const std::string& topic_name);
    bool check_register_topic(const std::string& topic_name, dds::xrce::ObjectId& object_id);
    void release(ObjectContainer& root_objects) override;
    void tie_object(const dds::xrce::ObjectId& object_id) { tied_objects_.insert(object_id); }
    void untie_object(const dds::xrce::ObjectId& object_id) { tied_objects_.erase(object_id); }
    bool matched(const dds::xrce::ObjectVariant& new_object_rep) const override;

private:
    void onParticipantDiscovery(fastrtps::Participant* p, fastrtps::ParticipantDiscoveryInfo info) override;
    fastrtps::Participant* rtps_participant_ = nullptr;
    std::unordered_map<std::string, dds::xrce::ObjectId> registered_topics_;
    std::set<dds::xrce::ObjectId> tied_objects_;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_PARTICIPANT_HPP_
