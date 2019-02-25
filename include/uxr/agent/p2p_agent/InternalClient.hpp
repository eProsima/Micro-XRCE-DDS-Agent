// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#ifndef UXR_AGENT_P2P_AGENT_INTERNAL_CLIENT_HPP_
#define UXR_AGENT_P2P_AGENT_INTERNAL_CLIENT_HPP_

#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <thread>

namespace eprosima {
namespace uxr {


class InternalClient
{
public:
    InternalClient(const std::string& ip, int port)
        : ip_(ip)
        , port_(port)
    {}

    virtual ~InternalClient() = default;

    void connect();
    void add_topic(/* topic */);

    std::string get_ip() const { return ip_; }
    int get_port() const { return port_; }

private:
    std::string ip_;
    int port_;
    std::thread main_thread_;
    std::thread subscription_thread_;
    //topi synchronized queue

    /* XRCE stuffs */
    void run();
    void subscribe();
    void read_topic_callback();

    /* Middleware publisher */
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_P2P_AGENT_INTERNAL_CLIENT_HPP_
