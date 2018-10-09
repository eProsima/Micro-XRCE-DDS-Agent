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

namespace eprosima {
namespace uxr {

Publisher::Publisher(const dds::xrce::ObjectId& object_id, const std::shared_ptr<Participant>& participant)
    : XRCEObject {object_id},
      participant_(participant)
{
    participant_->tie_object(object_id);
}

Publisher::~Publisher()
{
    participant_->untie_object(get_id());
}

} // namespace uxr
} // namespace eprosima
