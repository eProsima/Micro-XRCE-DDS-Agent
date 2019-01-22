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

Publisher::Publisher(const dds::xrce::ObjectId& object_id, Middleware* middleware, const std::shared_ptr<Participant>& participant)
    : XRCEObject(object_id, middleware),
      participant_(participant)
{
    participant_->tie_object(object_id);
}

Publisher::~Publisher()
{
    participant_->untie_object(get_id());
}

bool Publisher::init_middleware(const dds::xrce::OBJK_PUBLISHER_Representation &representation)
{
    bool rv = false;
    switch (representation.representation()._d())
    {
        case dds::xrce::REPRESENTATION_AS_XML_STRING:
        {
            const std::string& xml = representation.representation().string_representation();
            rv = middleware_->create_publisher_from_xml(get_raw_id(), participant_->get_raw_id(), xml);
            break;
        }
        case dds::xrce::REPRESENTATION_IN_BINARY:
        {
            // TODO (julian ???)
            break;
        }
    }
    return rv;
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

} // namespace uxr
} // namespace eprosima
