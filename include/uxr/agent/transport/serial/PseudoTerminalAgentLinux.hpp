// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_TRANSPORT_SERIAL_PSEUDOTERMINALAGENTLINUX_HPP_
#define UXR_AGENT_TRANSPORT_SERIAL_PSEUDOTERMINALAGENTLINUX_HPP_

#include <uxr/agent/transport/serial/SerialAgentLinux.hpp>

#include <termios.h>

namespace eprosima {
namespace uxr {

class PseudoTerminalAgent : public SerialAgent
{
public:
    PseudoTerminalAgent(
            int open_flags,
            char const * baudrate,
            uint8_t addr,
            Middleware::Kind middleware_kind);

    ~PseudoTerminalAgent();

private:
    bool init() final;
    bool fini() final;
    bool handle_error(
            TransportRc transport_rc);

private:
    int open_flags_;
    speed_t baudrate_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_SERIAL_PSEUDOTERMINALAGENTLINUX_HPP_