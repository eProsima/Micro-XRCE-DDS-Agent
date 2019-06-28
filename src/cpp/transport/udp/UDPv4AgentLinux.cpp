// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/transport/udp/UDPv4AgentLinux.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

namespace eprosima {
namespace uxr {

UDPv4Agent::UDPv4Agent(
        uint16_t agent_port,
        Middleware::Kind middleware_kind)
    : Server<IPv4EndPoint>{middleware_kind}
    , poll_fd_{-1, 0, 0}
    , buffer_{0}
#ifdef UAGENT_DISCOVERY_PROFILE
    , discovery_server_{*processor_}
#endif
#ifdef UAGENT_P2P_PROFILE
    , agent_discoverer_{*this}
#endif
{
    dds::xrce::TransportAddressMedium medium_locator;
    medium_locator.port(agent_port);
    transport_address_.medium_locator(medium_locator);
}

UDPv4Agent::~UDPv4Agent()
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

bool UDPv4Agent::init()
{
    bool rv = false;

    /* Socker initialization. */
    poll_fd_.fd = socket(PF_INET, SOCK_DGRAM, 0);

    if (-1 != poll_fd_.fd)
    {
        /* IP and Port setup. */
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(transport_address_.medium_locator().port());
        address.sin_addr.s_addr = INADDR_ANY;
        memset(address.sin_zero, '\0', sizeof(address.sin_zero));
        if (-1 != bind(poll_fd_.fd, (struct sockaddr*)&address, sizeof(address)))
        {
            /* Log. */
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_GREEN("port opened"),
                "port: {}",
                transport_address_.medium_locator().port()
                );

            /* Poll setup. */
            poll_fd_.events = POLLIN;

            /* Get local address. */
            int fd = socket(PF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in temp_addr;
            temp_addr.sin_family = AF_INET;
            temp_addr.sin_port = htons(80);
            temp_addr.sin_addr.s_addr = inet_addr("1.2.3.4");
            int connected = connect(fd, (struct sockaddr *)&temp_addr, sizeof(temp_addr));
            if (0 == connected)
            {
                struct sockaddr local_addr;
                socklen_t local_addr_len = sizeof(local_addr);
                if (-1 != getsockname(fd, &local_addr, &local_addr_len))
                {
                    transport_address_.medium_locator().address({uint8_t(local_addr.sa_data[2]),
                                                                 uint8_t(local_addr.sa_data[3]),
                                                                 uint8_t(local_addr.sa_data[4]),
                                                                 uint8_t(local_addr.sa_data[5])});
                    rv = true;
                    UXR_AGENT_LOG_INFO(
                        UXR_DECORATE_GREEN("running..."),
                        "port: {}",
                        transport_address_.medium_locator().port());
                }
                ::close(fd);
            }
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("bind error"),
                "port: {}",
                transport_address_.medium_locator().port());
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "port: {}",
            transport_address_.medium_locator().port());
    }

    return rv;
}

bool UDPv4Agent::close()
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
            "port: {}",
            transport_address_.medium_locator().port());
        poll_fd_.fd = -1;
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "port: {}",
            transport_address_.medium_locator().port());
    }
    return rv;
}

#ifdef UAGENT_DISCOVERY_PROFILE
bool UDPv4Agent::init_discovery(uint16_t discovery_port)
{
    return discovery_server_.run(discovery_port, transport_address_);
}

bool UDPv4Agent::close_discovery()
{
    return discovery_server_.stop();
}
#endif

#ifdef UAGENT_P2P_PROFILE
bool UDPv4Agent::init_p2p(uint16_t p2p_port)
{
#ifdef UAGENT_DISCOVERY_PROFILE
    discovery_server_.set_filter_port(p2p_port);
#endif
    return agent_discoverer_.run(p2p_port, transport_address_);
}

bool UDPv4Agent::close_p2p()
{
#ifdef UAGENT_DISCOVERY_PROFILE
    discovery_server_.set_filter_port(0);
#endif
    return agent_discoverer_.stop();
}
#endif

bool UDPv4Agent::recv_message(
        InputPacket<IPv4EndPoint>& input_packet,
        int timeout)
{
    bool rv = false;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int poll_rv = poll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_received = recvfrom(poll_fd_.fd,
                                          buffer_,
                                          sizeof(buffer_),
                                          0,
                                          reinterpret_cast<sockaddr*>(&client_addr),
                                          &client_addr_len);
        if (-1 != bytes_received)
        {
            input_packet.message.reset(new InputMessage(buffer_, static_cast<size_t>(bytes_received)));
            uint32_t addr = client_addr.sin_addr.s_addr;
            uint16_t port = client_addr.sin_port;
            input_packet.source = IPv4EndPoint(addr, port);
//            UXR_AGENT_LOG_MESSAGE(
//                UXR_DECORATE_YELLOW("[==>> UDP <<==]"),
//                conversion::clientkey_to_raw(get_client_key(input_packet.source.get())),
//                input_packet.message->get_buf(),
//                input_packet.message->get_len());
            rv = true;
        }
    }
    else
    {
        if (0 == poll_rv)
        {
            errno = ETIME;
        }
    }

    return rv;
}

bool UDPv4Agent::send_message(
        OutputPacket<IPv4EndPoint> output_packet)
{
    bool rv = false;
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = output_packet.destination.get_port();
    client_addr.sin_addr.s_addr = output_packet.destination.get_addr();
    ssize_t bytes_sent = sendto(poll_fd_.fd,
                                output_packet.message->get_buf(),
                                output_packet.message->get_len(),
                                0,
                                (struct sockaddr*)&client_addr,
                                sizeof(client_addr));
    if (-1 != bytes_sent)
    {
        if (size_t(bytes_sent) == output_packet.message->get_len())
        {
//            UXR_AGENT_LOG_MESSAGE(
//                UXR_DECORATE_YELLOW("[** <<UDP>> **]"),
//                conversion::clientkey_to_raw(get_client_key(output_packet.destination.get())),
//                output_packet.message->get_buf(),
//                output_packet.message->get_len());
            rv = true;
        }
    }

    return rv;
}

int UDPv4Agent::get_error()
{
    return errno;
}

} // namespace uxr
} // namespace eprosima
