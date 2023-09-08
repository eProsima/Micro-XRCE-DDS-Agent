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

#include <uxr/agent/transport/serial/MultiTermiosAgentLinux.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <algorithm>

namespace eprosima {
namespace uxr {

MultiTermiosAgent::MultiTermiosAgent(
        std::vector<std::string> devs,
        int open_flags,
        termios const& termios_attrs,
        uint8_t addr,
        Middleware::Kind middleware_kind)
    : MultiSerialAgent(addr, middleware_kind)
    , exitSignal(false)
    , devs_{}
    , initialized_devs_{}
    , open_flags_{open_flags}
    , termios_attrs_{termios_attrs}
{
    for (auto & element : devs)
    {
        devs_.push_back(std::pair<int, std::string>(0, element));
    }
}

MultiTermiosAgent::~MultiTermiosAgent()
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

void MultiTermiosAgent::init_multiport()
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();;
    bool wake_main = false;
    exitSignal = false;
    
    do
    {
        std::unique_lock<std::mutex> lk(devs_mtx);
        for(auto it = devs_.begin(); it!=devs_.end(); )
        {
            if(access(it->second.c_str(), W_OK | R_OK ) == 0 || it->first > 10)
            {
                pollfd aux_poll_fd;
                aux_poll_fd.fd = open(it->second.c_str(), open_flags_);
                if (0 < aux_poll_fd.fd)
                {
                    struct termios new_attrs;
                    memset(&new_attrs, 0, sizeof(new_attrs));
                    if (0 == tcgetattr(aux_poll_fd.fd, &new_attrs))
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

                        if (0 == tcsetattr(aux_poll_fd.fd, TCSANOW, &new_attrs))
                        {
                            // Add open port to MultiSerialAgent
                            insert_serial(aux_poll_fd.fd);
                            initialized_devs_.insert(std::pair<int, std::string>(aux_poll_fd.fd, it->second));
                            tcflush(aux_poll_fd.fd, TCIOFLUSH);

                            UXR_AGENT_LOG_INFO(
                                UXR_DECORATE_GREEN("Serial port running..."),
                                "device: {}, fd: {}",
                                it->second, aux_poll_fd.fd);
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
                        it->second, errno,
                        (EACCES == errno) ? ". Please re-run with superuser privileges." : "");
                }

                it = devs_.erase(it);
            }
            else
            {
                if (EACCES == errno || EBUSY == errno)
                {
                    // Increase error counter
                    it->first++;
                }

                it++;
            }
        }

        if (devs_.size() > 0)
        {
            if (!wake_main && initialized_devs_.size())
            {
                init_serial_cv.notify_one();
                wake_main = true;
            }
            
            std::this_thread::sleep_for((std::chrono::milliseconds) 10);

            if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - begin).count())
            {
                std::string aux_str;
                for (const auto &port : devs_) aux_str += (" " + port.second);

                begin = std::chrono::steady_clock::now();
                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_YELLOW("Serial ports not found."),
                    "Waiting for devices: {}",
                    aux_str);
            }
        }
        else if (!exitSignal)
        {
            // All ports handled, notify main thread
            init_serial_cv.notify_one();

            // Wait for more ports
            init_serial_cv.wait(lk);
        }

        lk.unlock();

    } while (!exitSignal);
}

bool MultiTermiosAgent::init()
{
    init_serial = std::thread(&MultiTermiosAgent::init_multiport, this);

    std::mutex temp_mtx;
    std::unique_lock<std::mutex> lk(temp_mtx);

    // Wait for initialized port
    init_serial_cv.wait(lk);

    return (framing_io.size() > 0) ? true : false;
}

bool MultiTermiosAgent::fini()
{
    bool rv = true;

    if (init_serial.joinable())
    {
        exitSignal = true;
        init_serial_cv.notify_one();
        init_serial.join();
    }

    for (auto it = initialized_devs_.begin(); it != initialized_devs_.end();)
    {
        if (restart_serial(it))
        {
            it = initialized_devs_.erase(it);
        }
        else
        {
            rv = false;
            it++;
        }
    }

    if (rv)
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("server stopped"),
            "", "");
    }
    else
    {
        // TODO: handle close error
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("close server error"),
            "", "");
    }

    return rv;
}

bool MultiTermiosAgent::handle_error(
        TransportRc)
{
    bool rv = true;

    // Delete duplicates on error_fd
    std::unique_lock<std::mutex> error_lk(error_mtx);
    std::sort( error_fd.begin(), error_fd.end() );
    error_fd.erase( std::unique( error_fd.begin(), error_fd.end() ), error_fd.end() );

    // Close failed serial port and add to open thread
    if (error_fd.size() == initialized_devs_.size())
    {
        rv = fini() && init();

        // TODO: handle unclosed error ports
        error_fd.clear();
    }
    else if (error_fd.size() > 0)
    {
        std::unique_lock<std::mutex> devs_lk(devs_mtx);

        for(auto serial_fd = error_fd.begin(); serial_fd!=error_fd.end(); )
        {
            std::map<int, std::string>::iterator it = initialized_devs_.find(*serial_fd);
            
            if(it != initialized_devs_.end())
            {
                if(restart_serial(it))
                {
                    initialized_devs_.erase(it);
                    serial_fd = error_fd.erase(serial_fd);
                }
                else
                {
                    // Failed to close serial, keep file descriptor on vector
                    serial_fd++;
                    rv = false;
                }
            }
        }

        // Wake serial init thread
        init_serial_cv.notify_one();        
    }

    // TODO: handle close errors
    return rv;
}

bool MultiTermiosAgent::restart_serial(std::map<int, std::string>::iterator initialized_devs_it)
{
    bool rv = remove_serial(initialized_devs_it->first);

    if (rv)
    {
        devs_.push_back(std::pair<int, std::string>(0, initialized_devs_it->second));
    }

    return rv;
}

} // namespace uxr
} // namespace eprosima
