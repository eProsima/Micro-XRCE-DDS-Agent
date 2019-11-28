// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/requester/Requester.hpp>
#include <uxr/agent/participant/Participant.hpp>

namespace eprosima {
namespace uxr {

std::unique_ptr<Requester> Requester::create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant,
        const dds::xrce::REQUESTER_Representation& representation,
        const ObjectContainer& root_objects)
{
    // TODO.
}

Requester::Requester(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant)
    : XRCEObject(object_id)
    , participant_(participant_)
{
    // TODO.
}

Requester::~Requester()
{
    // TODO.
}

bool Requester::write(
        dds::xrce::WRITE_DATA_Payload_Data& write_data)
{
    // TODO.
}

bool Requester::write(
        const std::vector<uint8_t>& data)
{
    // TODO.            
}

bool Requester::read(
        const dds::xrce::READ_DATA_Payload& read_data,
        read_callback read_cb,
        const ReadCallbackArgs& cb_args)
{
    // TODO.
}

} // namespace uxr
} // namespace eprosima