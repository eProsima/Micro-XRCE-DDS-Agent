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

#ifndef UXR_AGENT_TOPIC_TOPIC_HPP_
#define UXR_AGENT_TOPIC_TOPIC_HPP_

#include <uxr/agent/object/XRCEObject.hpp>
#include <string>
#include <memory>
#include <set>

namespace eprosima {
namespace uxr {

class Participant;
class ProxyClient;
class Middleware;

class Topic : public XRCEObject
{
public:
    static std::unique_ptr<Topic> create(const dds::xrce::ObjectId& object_id,
        uint16_t participant_id,
        const std::shared_ptr<ProxyClient>& proxy_client,
        const dds::xrce::OBJK_TOPIC_Representation& representation);

    ~Topic() override;

    Topic(Topic&&) = delete;
    Topic(const Topic&) = delete;
    Topic& operator=(Topic&&) = delete;
    Topic& operator=(const Topic&) = delete;

    bool matched(const dds::xrce::ObjectVariant& new_object_rep) const final;

private:
    Topic(
        const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<ProxyClient>& proxy_client);

private:
    std::shared_ptr<ProxyClient> proxy_client_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TOPIC_TOPIC_HPP_
