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

#ifndef UXR_AGENT_TRANSPORT_MULTISERIAL_TERMIOSAGENTLINUX_HPP_
#define UXR_AGENT_TRANSPORT_MULTISERIAL_TERMIOSAGENTLINUX_HPP_

#include <uxr/agent/transport/serial/MultiSerialAgentLinux.hpp>

#include <termios.h>
#include <vector>
#include <atomic>

namespace eprosima {
namespace uxr {

class MultiTermiosAgent : public MultiSerialAgent
{
public:
    MultiTermiosAgent(
            std::vector<std::string> devs,
            int open_flags,
            termios const & termios_attrs,
            uint8_t addr,
            Middleware::Kind middleware_kind);

    ~MultiTermiosAgent();

    bool restart_serial(std::map<int, std::string>::iterator initialized_devs_it);

    std::vector<int> getfds(){
        std::vector<int> result;
        std::unique_lock<std::mutex> lk(devs_mtx);
        for (auto & element : initialized_devs_)
        {
            result.push_back(element.first);
        }

        return result;
    }

private:
    void init_multiport();
    bool init() final;
    bool fini() final;
    bool handle_error(
            TransportRc transport_rc) final;

    std::thread init_serial;
    std::condition_variable init_serial_cv;

    std::mutex devs_mtx;
    std::atomic<bool> exitSignal;

    std::vector<std::pair<int, std::string>> devs_;
    std::map<int, std::string> initialized_devs_;

    const int open_flags_;
    const termios termios_attrs_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_MULTISERIAL_TERMIOSAGENTLINUX_HPP_