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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

namespace eprosima {
namespace uxr {

AgentDiscovererLinux::AgentDiscovererLinux()
    : poll_fd_{-1, 0, 0}
{
}

bool AgentDiscovererLinux::init(uint16_t p2p_port)
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
    if (-1 != bind(poll_fd_.fd, (struct sockaddr*)&address, sizeof(address)))
    {
        /* Poll setup. */
        poll_fd_.events = POLLIN;
        rv = true;
    }

    return rv;
}

bool AgentDiscovererLinux::close()
{
    return (-1 == poll_fd_.fd) || ::close(poll_fd_.fd);
}

} // namespace eprosima
} // namespace uxr
