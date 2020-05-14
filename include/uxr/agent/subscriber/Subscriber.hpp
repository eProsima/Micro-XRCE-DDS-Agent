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

#ifndef UXR_AGENT_SUBSCRIBER_SUBSCRIBER_HPP_
#define UXR_AGENT_SUBSCRIBER_SUBSCRIBER_HPP_

#include <uxr/agent/object/XRCEObject.hpp>
#include <set>

namespace eprosima {
namespace uxr {

class Participant;
class ProxyClient;
class Middleware;

class Subscriber : public XRCEObject
{
public:
    static std::unique_ptr<Subscriber> create(const dds::xrce::ObjectId& object_id,
        uint16_t participant_id,
        const std::shared_ptr<ProxyClient>& proxy_client,
        const dds::xrce::OBJK_SUBSCRIBER_Representation& representation);

    virtual ~Subscriber() override;

    Subscriber(Subscriber&&) = delete;
    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(Subscriber&&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;

    bool matched(
        const dds::xrce::ObjectVariant& ) const final { return true; }

private:
    Subscriber(const dds::xrce::ObjectId& object_id,
        const std::shared_ptr<ProxyClient>& proxy_client);

private:
    std::shared_ptr<ProxyClient> proxy_client_;
    std::set<dds::xrce::ObjectId> tied_objects_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_SUBSCRIBER_SUBSCRIBER_HPP_
