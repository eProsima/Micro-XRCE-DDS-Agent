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

#include <uxr/agent/transport/serial/TermiosAgentLinux.hpp>

#include <fcntl.h>
#include <unistd.h>

namespace eprosima {
namespace uxr {

TermiosAgent::TermiosAgent(
        char const * dev,
        int open_flags,
        termios const & termios_attrs,
        uint8_t addr,
        Middleware::Kind middleware_kind)
    : SerialAgent(addr, middleware_kind)
    , dev_{dev}
    , open_flags_{open_flags}
    , termios_attrs_{termios_attrs}
{}

TermiosAgent::~TermiosAgent()
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

bool TermiosAgent::init()
{
    bool rv = false;
    poll_fd_.fd = open(dev_.c_str(), open_flags_);
    if (0 < poll_fd_.fd)
    {
        if (0 == tcsetattr(poll_fd_.fd, TCSANOW, &termios_attrs_))
        {
            /* Init serial IO. */
            uxr_init_serial_io(&serial_io_, addr_);

            /* Poll setup. */
            poll_fd_.events = POLLIN;

            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("running..."),
                "fd: {}",
                poll_fd_.fd);
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("set termios attributes error"),
                "tcsetattr errno: {}",
                errno);
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("open device error"),
            "device: {}, open errno: {}",
            dev_, errno);
    }
    return rv;
}

bool TermiosAgent::close()
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
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("close server error"),
            "fd: {}",
            poll_fd_.fd);
    }
    return rv;
}


} // namespace uxr
} // namespace eprosima