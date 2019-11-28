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

#ifndef UXR_AGENT_REQUESTER_REQUESTER_HPP_
#define UXR_AGENT_REQUESTER_REQUESTER_HPP_

#include <uxr/agent/object/XRCEObject.hpp>

namespace eprosima {
namespace uxr {

class Participant;
class Middleware;

class ReadCallbackArgs; // TODO: remove.
typedef const std::function<bool (const ReadCallbackArgs&, std::vector<uint8_t>, std::chrono::milliseconds)> read_callback; // TODO: remove.

class Requester : public XRCEObject
{
public:
    static std::unique_ptr<Requester> create(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant,
        const dds::xrce::REQUESTER_Representation& representation,
        const ObjectContainer& root_objects);

    ~Requester() override;

    Requester(Requester&&) = delete;
    Requester(const Requester&) = delete;
    Requester& operator=(Requester&&) = delete;
    Requester& operator=(const Requester&) = delete;

    bool write(
        dds::xrce::WRITE_DATA_Payload_Data& write_data);

    bool write(
        const std::vector<uint8_t>& data);

    bool read(
        const dds::xrce::READ_DATA_Payload& read_data,
        read_callback read_cb,
        const ReadCallbackArgs& cb_args);

private:
    Requester(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<Participant>& participant);

private:
    std::shared_ptr<Participant> participant_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_REQUESTER_REQUESTER_HPP_