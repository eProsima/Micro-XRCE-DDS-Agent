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

#ifndef UXR_AGENT_REPLIER_REPLIER_HPP_
#define UXR_AGENT_REPLIER_REPLIER_HPP_

#include <uxr/agent/object/XRCEObject.hpp>
#include <uxr/agent/reader/Reader.hpp>

namespace eprosima {
namespace uxr {

class Participant;
class ProxyClient;
class Middleware;

class Replier : public XRCEObject
{
public:
    static std::unique_ptr<Replier> create(
        const dds::xrce::ObjectId& object_id,
        uint16_t participant_id,
        const std::shared_ptr<ProxyClient>& proxy_client,
        const dds::xrce::REPLIER_Representation& representation);

    ~Replier() override;

    Replier(Replier&&) = delete;
    Replier(const Replier&) = delete;
    Replier& operator=(Replier&&) = delete;
    Replier& operator=(const Replier&) = delete;

    bool write(
        dds::xrce::WRITE_DATA_Payload_Data& write_data);

    bool read(
        const dds::xrce::READ_DATA_Payload& read_data,
        Reader<bool>::WriteFn write_fn,
        WriteFnArgs& write_args);

    bool matched(
        const dds::xrce::ObjectVariant& new_object_rep) const override;

private:
    Replier(
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

#endif // UXR_AGENT_REPLIER_REPLIER_HPP_