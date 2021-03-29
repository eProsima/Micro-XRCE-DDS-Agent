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

#ifndef UXR_AGENT_CLIENT_PROXYCLIENT_HPP_
#define UXR_AGENT_CLIENT_PROXYCLIENT_HPP_

#include <uxr/agent/middleware/Middleware.hpp>
#include <uxr/agent/types/XRCETypes.hpp>
#include <uxr/agent/client/session/Session.hpp>

#include <gmock/gmock.h>

namespace eprosima {
namespace uxr {

class Middleware;

class ProxyClient
{
public:
    explicit ProxyClient(
        const dds::xrce::CLIENT_Representation& /*representation*/,
        Middleware::Kind /*middleware_kind*/,
        std::unordered_map<std::string, std::string>&& properties = {}) {};

    ~ProxyClient() = default;

    ProxyClient(ProxyClient&&) = delete;
    ProxyClient(const ProxyClient&) = delete;
    ProxyClient& operator=(ProxyClient&&) = delete;
    ProxyClient& operator=(const ProxyClient&) = delete;

    MOCK_METHOD0(get_session_id, dds::xrce::SessionId());
    MOCK_METHOD0(session, Session&());

    void release() {}
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_CLIENT_PROXYCLIENT_HPP_
