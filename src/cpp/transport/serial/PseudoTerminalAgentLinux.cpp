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

#include <uxr/agent/transport/serial/PseudoTerminalAgentLinux.hpp>
#include <uxr/agent/transport/serial/baud_rate_table_linux.h>

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

namespace eprosima {
namespace uxr {

PseudoTerminalAgent::PseudoTerminalAgent(
        int open_flags,
        char const * baudrate,
        uint8_t addr,
        Middleware::Kind middleware_kind)
    : SerialAgent(addr, middleware_kind)
    , open_flags_{open_flags}
    , baudrate_{getBaudRate(baudrate)}
{}

PseudoTerminalAgent::~PseudoTerminalAgent()
{
    try
    {
        stop();
    }
    catch (std::exception& e)
    {
        UXR_AGENT_LOG_CRITICAL(
            UXR_DECORATE_RED("error stopping server"),
            "exception: {}",
            e.what());
    }
}

bool PseudoTerminalAgent::init()
{
    bool rv = false;
    char* dev = nullptr;

    poll_fd_.fd = posix_openpt(open_flags_);
    if ((-1 != poll_fd_.fd) && (0 == grantpt(poll_fd_.fd)) && (0 == unlockpt(poll_fd_.fd)) && (dev = ptsname(poll_fd_.fd)))
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("Pseudoterminal opened at"),
            "dev: {}",
            dev);

        struct termios attrs;
        tcgetattr(poll_fd_.fd, &attrs);
        cfmakeraw(&attrs);
        tcflush(poll_fd_.fd, TCIOFLUSH);
        cfsetispeed(&attrs, baudrate_);
        cfsetospeed(&attrs, baudrate_);
        tcsetattr(poll_fd_.fd, TCSANOW, &attrs);
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("open pseudoterminal error"),
            "errno: {}",
            errno);
    }

    return rv;
}

bool PseudoTerminalAgent::fini()
{
    if (-1 == poll_fd_.fd)
    {
        return true;
    }

    bool rv = false;
    if (0 == ::close(poll_fd_.fd))
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("server stopped"),
            "fd: {}",
            poll_fd_.fd);
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("close server error"),
            "fd: {}, errno: {}",
            poll_fd_.fd, errno);
    }
    return rv;
}

bool PseudoTerminalAgent::handle_error(
        TransportRc /*transport_rc*/)
{
    return fini() && init();
}

} // namespace uxr
} // namespace eprosima