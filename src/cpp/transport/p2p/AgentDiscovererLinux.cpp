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

#include <uxr/agent/transport/p2p/AgentDiscovererLinux.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

namespace eprosima {
namespace uxr {

AgentDiscovererLinux::AgentDiscovererLinux(
        Agent& agent)
    : AgentDiscoverer(agent)
    , poll_fd_{-1, 0, 0}
    , buf_{0}
{
}

bool AgentDiscovererLinux::init(
        uint16_t p2p_port)
{
    bool rv = false;

    /* Socket initialization. */
    poll_fd_.fd = socket(PF_INET, SOCK_DGRAM, 0);

    /* Port setup. */
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(p2p_port);
    address.sin_addr.s_addr = INADDR_ANY;
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));
    if (-1 != bind(poll_fd_.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)))
    {
        /* Log. */
        UXR_AGENT_LOG_DEBUG(
            UXR_DECORATE_WHITE("port opened"),
            "Port: {}",
            p2p_port);
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_WHITE("launched"),
            "Port: {}",
            p2p_port);

        /* Poll setup. */
        poll_fd_.events = POLLIN;
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("bind error"),
            "Port: {}",
            p2p_port);
    }

    return rv;
}

bool AgentDiscovererLinux::fini()
{
    return (-1 == poll_fd_.fd) || (0 == ::close(poll_fd_.fd));
}

bool AgentDiscovererLinux::recv_message(
        InputMessagePtr& input_message,
        int timeout)
{
    bool rv = false;

    struct sockaddr address;
    socklen_t address_len = sizeof(address);
    int poll_rv = poll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_received = recvfrom(poll_fd_.fd, buf_, sizeof(buf_), 0, &address, &address_len);
        if (0 < bytes_received)
        {
            input_message.reset(new InputMessage(buf_, size_t(bytes_received)));
            rv = true;
        }
    }

    return rv;
}

bool AgentDiscovererLinux::send_message(
        const OutputMessage& output_message)
{
    bool rv = false;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(DISCOVERY_IP);
    address.sin_port = htons(DISCOVERY_PORT);
    ssize_t bytes_sent =
            sendto(poll_fd_.fd,
                   output_message.get_buf(),
                   output_message.get_len(),
                   0,
                   reinterpret_cast<struct sockaddr*>(&address),
                   sizeof(address));
    if (0 < bytes_sent)
    {
        rv = (size_t(bytes_sent) == output_message.get_len());
    }

    return rv;
}

} // namespace eprosima
} // namespace uxr
