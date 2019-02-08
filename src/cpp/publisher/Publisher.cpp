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

#include <uxr/agent/publisher/Publisher.hpp>
#include <uxr/agent/participant/Participant.hpp>
#include <uxr/agent/middleware/Middleware.hpp>

namespace eprosima {
namespace uxr {

Publisher* Publisher::create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant,
        const dds::xrce::OBJK_PUBLISHER_Representation& representation)
{
    bool created_entity = false;
    uint16_t raw_object_id = uint16_t((object_id[0] << 8) + object_id[1]);

    Middleware* middleware = participant->get_middleware();
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().string_representation();
            created_entity = middleware->create_publisher_from_xml(raw_object_id, participant->get_raw_id(), xml);
            break;
        }
        case dds::xrce::REPRESENTATION_IN_BINARY:
        {
            // TODO (julian ???)
            break;
        }
    }

    return (created_entity ? new Publisher(object_id, participant) : nullptr);
}

Publisher::Publisher(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant)
    : XRCEObject(object_id)
    , participant_(participant)
{
    participant_->tie_object(object_id);
}

Publisher::~Publisher()
{
    participant_->untie_object(get_id());
    get_middleware()->delete_publisher(get_raw_id(), participant_->get_raw_id());
}

void Publisher::release(ObjectContainer& root_objects)
{
    while (!tied_objects_.empty())
    {
        auto obj = tied_objects_.begin();
        root_objects.at(*obj)->release(root_objects);
        root_objects.erase(*obj);
    }
}

Middleware* Publisher::get_middleware() const
{
    return participant_->get_middleware();
}

} // namespace uxr
} // namespace eprosima
