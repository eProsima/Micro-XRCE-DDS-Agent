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
        char const* dev,
        int open_flags,
        termios const& termios_attrs,
        uint8_t addr,
        Middleware::Kind middleware_kind)
    : SerialAgent(addr, middleware_kind)
    , dev_{dev}
    , open_flags_{open_flags}
    , termios_attrs_{termios_attrs}
{
}

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

    // Check if serial port exist
    std::chrono::steady_clock::time_point begin;
    int serial_exist = 0;
    int error_count = 0;

    do
    {
        if (serial_exist != 0)
        {
            std::this_thread::sleep_for((std::chrono::milliseconds) 10);

            if (EACCES == errno || EBUSY == errno)
            {
                // Increase error count
                error_count++;

                if (error_count > 10)
                {
                    // Resource busy or superuser privileges required
                    break;
                }
            }
            else if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - begin).count())
            {
                begin = std::chrono::steady_clock::now();
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_YELLOW("Serial port not found."),
                    "device: {}, error {}, waiting for connection...",
                    dev_, errno);
            }
        }

        serial_exist = access(dev_.c_str(), W_OK );
    }
    while (serial_exist != 0);

    poll_fd_.fd = open(dev_.c_str(), open_flags_);
    if (0 < poll_fd_.fd)
    {
        struct termios new_attrs;
        memset(&new_attrs, 0, sizeof(new_attrs));
        if (0 == tcgetattr(poll_fd_.fd, &new_attrs))
        {
            new_attrs.c_cflag = termios_attrs_.c_cflag;
            new_attrs.c_lflag = termios_attrs_.c_lflag;
            new_attrs.c_iflag = termios_attrs_.c_iflag;
            new_attrs.c_oflag = termios_attrs_.c_oflag;
            new_attrs.c_cc[VMIN] = termios_attrs_.c_cc[VMIN];
            new_attrs.c_cc[VTIME] = termios_attrs_.c_cc[VTIME];

#if _HAVE_STRUCT_TERMIOS_C_ISPEED || __APPLE__
            cfsetispeed(&new_attrs, termios_attrs_.c_ispeed);
#endif
#if _HAVE_STRUCT_TERMIOS_C_OSPEED || __APPLE__
            cfsetospeed(&new_attrs, termios_attrs_.c_ospeed);
#endif

            if (0 == tcsetattr(poll_fd_.fd, TCSANOW, &new_attrs))
            {
                rv = true;
                poll_fd_.events = POLLIN;

                tcflush(poll_fd_.fd, TCIOFLUSH);

                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("running..."),
                    "fd: {}",
                    poll_fd_.fd);
            }
            else
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("set termios attributes error"),
                    "errno: {}",
                    errno);
            }
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("get termios attributes error"),
                "errno: {}",
                errno);
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("open device error"),
            "device: {}, errno: {}{}",
            dev_, errno,
            (EACCES == errno) ? ". Please re-run with superuser privileges." : "");
    }
    return rv;
}

bool TermiosAgent::fini()
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

    poll_fd_.fd = -1;
    return rv;
}

bool TermiosAgent::handle_error(
        TransportRc /*transport_rc*/)
{
    return fini() && init();
}

} // namespace uxr
} // namespace eprosima
