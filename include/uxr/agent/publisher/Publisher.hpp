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

#ifndef UXR_AGENT_PUBLISHER_PUBLISHER_HPP_
#define UXR_AGENT_PUBLISHER_PUBLISHER_HPP_

#include <uxr/agent/object/XRCEObject.hpp>
#include <set>

namespace eprosima {
namespace uxr {

class Participant;
class Middleware;

class Publisher : public XRCEObject
{
public:
    static Publisher* create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant,
        const dds::xrce::OBJK_PUBLISHER_Representation& representation);

    virtual ~Publisher() override;

    Publisher(Publisher&&) = delete;
    Publisher(const Publisher&) = delete;
    Publisher& operator=(Publisher&&) = delete;
    Publisher& operator=(const Publisher&) = delete;

    void release(ObjectContainer& root_objects) override;
    void tie_object(const dds::xrce::ObjectId& object_id) { tied_objects_.insert(object_id); }
    void untie_object(const dds::xrce::ObjectId& object_id) { tied_objects_.erase(object_id); }
    bool matched(const dds::xrce::ObjectVariant& ) const override { return true; }
    Middleware* get_middleware() const override;

    const std::shared_ptr<Participant>& get_participant() { return participant_; }

private:
    Publisher(const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant);

private:
    std::shared_ptr<Participant> participant_;
    std::set<dds::xrce::ObjectId> tied_objects_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_PUBLISHER_PUBLISHER_HPP_
