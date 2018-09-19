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

#ifndef _MICRORTPS_AGENT_SUBSCRIBER_SUBSCRIBER_HPP_
#define _MICRORTPS_AGENT_SUBSCRIBER_SUBSCRIBER_HPP_

#include <micrortps/agent/object/XRCEObject.hpp>
#include <set>

namespace eprosima {
namespace micrortps {

class Participant;

class Subscriber : public XRCEObject
{
public:
    Subscriber(const dds::xrce::ObjectId& object_id, const std::shared_ptr<Participant>& participant);
    virtual ~Subscriber();

    const std::shared_ptr<Participant>& get_participant() { return participant_; }
    virtual void release(ObjectContainer& root_objects) override;
    void tie_object(const dds::xrce::ObjectId& object_id) { tied_objects_.insert(object_id); }
    void untie_object(const dds::xrce::ObjectId& object_id) { tied_objects_.erase(object_id); }

private:
    std::shared_ptr<Participant> participant_;
    std::set<dds::xrce::ObjectId> tied_objects_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_SUBSCRIBER_SUBSCRIBER_HPP_
