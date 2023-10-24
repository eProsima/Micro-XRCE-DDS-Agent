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

#include <uxr/agent/transport/serial/TermiosAgentWindows.hpp>

#include <fcntl.h>

namespace eprosima {
namespace uxr {

TermiosAgent::TermiosAgent(
        char const* dev,
        DCB const& termios_attrs,
        uint8_t addr,
        Middleware::Kind middleware_kind)
    : SerialAgent(addr, middleware_kind)
    , dev_{dev}
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
    /*

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
    */

    serial_handle = CreateFile(dev_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (INVALID_HANDLE_VALUE != serial_handle)
    {
        DCB new_attrs = {};
        new_attrs.DCBlength = sizeof(DCB);

        if (GetCommState(serial_handle, &new_attrs)) {
            new_attrs.BaudRate = termios_attrs_.BaudRate;
            new_attrs.Parity = termios_attrs_.Parity;
            new_attrs.StopBits = termios_attrs_.StopBits;
            new_attrs.ByteSize = termios_attrs_.ByteSize;

            new_attrs.fBinary = termios_attrs_.fBinary;
            new_attrs.fParity = termios_attrs_.fParity;
            new_attrs.fOutxCtsFlow = termios_attrs_.fOutxCtsFlow;
            new_attrs.fOutxDsrFlow = termios_attrs_.fOutxDsrFlow;
            new_attrs.fDtrControl = termios_attrs_.fDtrControl;
            new_attrs.fRtsControl = termios_attrs_.fRtsControl;
            new_attrs.fInX = termios_attrs_.fInX;
            new_attrs.fOutX = termios_attrs_.fOutX;
            new_attrs.fDsrSensitivity = termios_attrs_.fDsrSensitivity;
            new_attrs.fErrorChar = termios_attrs_.fErrorChar;


            if (SetCommState(serial_handle, &new_attrs))
            {
                COMMTIMEOUTS comm_timeouts{};
                comm_timeouts.ReadIntervalTimeout = MAXDWORD;
                comm_timeouts.ReadTotalTimeoutMultiplier = 0;
                comm_timeouts.ReadTotalTimeoutConstant = 0;;
                comm_timeouts.WriteTotalTimeoutConstant = 0,
                comm_timeouts.WriteTotalTimeoutMultiplier = 0;
                if (!SetCommTimeouts(serial_handle, &comm_timeouts)) {
                    UXR_AGENT_LOG_ERROR(
                        UXR_DECORATE_RED("set SetCommTimeouts error"),
                        "errno: {}",
                        errno);
                }
                else {
                    rv = true;

                    UXR_AGENT_LOG_INFO(
                        UXR_DECORATE_GREEN("running..."),
                        "handle: {}",
                        serial_handle);
                }
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
    if (INVALID_HANDLE_VALUE == serial_handle)
    {
        return true;
    }

    bool rv = false;
    if (CloseHandle(serial_handle))
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("server stopped"),
            "handle: {}",
            serial_handle);
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("close server error"),
            "handle: {}, errno: {}",
            serial_handle, errno);
    }

    serial_handle = INVALID_HANDLE_VALUE;
    return rv;
}

bool TermiosAgent::handle_error(
        TransportRc /*transport_rc*/)
{
    return fini() && init();
}

} // namespace uxr
} // namespace eprosima
