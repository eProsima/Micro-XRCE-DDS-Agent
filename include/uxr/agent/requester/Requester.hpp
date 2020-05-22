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
#include <uxr/agent/replier/Replier.hpp>

namespace eprosima {
namespace uxr {

class Participant;
class Middleware;

class Requester : public XRCEObject
{
public:
    static std::unique_ptr<Requester> create(
        const dds::xrce::ObjectId& object_id,
        uint16_t participant_id,
        const std::shared_ptr<ProxyClient>& proxy_client,
        const dds::xrce::REQUESTER_Representation& representation);

    ~Requester() override;

    Requester(Requester&&) = delete;
    Requester(const Requester&) = delete;
    Requester& operator=(Requester&&) = delete;
    Requester& operator=(const Requester&) = delete;

    bool write(
        dds::xrce::WRITE_DATA_Payload_Data& write_data,
        const dds::xrce::RequestId& request_id);

    bool read(
        const dds::xrce::READ_DATA_Payload& read_data,
        Reader<bool>::WriteFn write_fn,
        WriteFnArgs& write_args);

    bool matched(
        const dds::xrce::ObjectVariant& new_object_rep) const override;

private:
    Requester(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<ProxyClient>& proxy_client);

    bool read_fn(
        bool,
        std::vector<uint8_t>& data,
        std::chrono::milliseconds timeout);

private:
    std::shared_ptr<ProxyClient> proxy_client_;
    Reader<bool> reader_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_REQUESTER_REQUESTER_HPP_