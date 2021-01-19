// Copyright 2021-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/transport/custom/CustomAgent.hpp>
#include <uxr/agent/transport/endpoint/IPv4EndPoint.hpp>

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * This custom XRCE Agent example attempts to show how easy is for the user to define a custom
 * Micro XRCE-DDS Agent behaviour, in terms of transport initialization and closing, and also
 * regarding read and write operations.
 * For this simple case, an UDP socket is opened on port 8888. Additionally, some information
 * messages are being printed to demonstrate the custom behaviour.
 * As the endpoint is already defined, we are using the provided
 * `eprosima::uxr::IPv4EndPoint` by the library.
 * Other transport protocols might need to implement their own endpoint struct.
 */

int main(int argc, char** argv)
{
    eprosima::uxr::Middleware::Kind mw_kind(eprosima::uxr::Middleware::Kind::FASTDDS);
    uint16_t agent_port(8888);
    struct pollfd poll_fd;

    /**
     * @brief Agent's initialization behaviour description.
     */
    auto init_function = [&](
            eprosima::uxr::CustomAgent<eprosima::uxr::IPv4EndPoint>* /*agent*/) -> bool
    {
        bool rv = false;
        poll_fd.fd = socket(PF_INET, SOCK_DGRAM, 0);

        if (-1 != poll_fd.fd)
        {
            struct sockaddr_in address{};

            address.sin_family = AF_INET;
            address.sin_port = htons(agent_port);
            address.sin_addr.s_addr = INADDR_ANY;
            memset(address.sin_zero, '\0', sizeof(address.sin_zero));

            if (-1 != bind(poll_fd.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)))
            {
                poll_fd.events = POLLIN;
                rv = true;

                UXR_AGENT_LOG_DEBUG(
                    UXR_DECORATE_GREEN("Custom agent opened"),
                    "port: {}",
                    agent_port);

                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("Custom agent running..."),
                    "port: {}",
                    agent_port);
            }
            else
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("Custom agent bind error"),
                    "port: {}, errno: {}",
                    agent_port, errno);
            }
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("Custom agent socket error"),
                "port: {}, errno: {}",
                agent_port, errno);
        }

        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("This is an example of a custom Micro XRCE-DDS Agent INIT function"),
            "port: {}",
            agent_port);

        return rv;
    };

    /**
     * @brief Agent's destruction actions.
     */
    auto fini_function = [&](
            eprosima::uxr::CustomAgent<eprosima::uxr::IPv4EndPoint>* /*agent*/) -> bool
    {
        if (-1 == poll_fd.fd)
        {
            return true;
        }

        bool rv = false;
        if (0 == ::close(poll_fd.fd))
        {
            poll_fd.fd = -1;
            rv = true;
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("Custom agent server stopped"),
                "port: {}",
                agent_port);
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("Custom agent socket error"),
                "port: {}, errno: {}",
                agent_port, errno);
        }

        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("This is an example of a custom Micro XRCE-DDS Agent FINI function"),
            "port: {}",
            agent_port);

        return rv;
    };

    /**
     * @brief Agent's incoming data functionality.
     */
    auto recv_msg_function = [&](
            eprosima::uxr::CustomAgent<eprosima::uxr::IPv4EndPoint>* agent,
            uint8_t* buffer,
            eprosima::uxr::InputPacket<eprosima::uxr::IPv4EndPoint>& input_packet,
            int timeout,
            eprosima::uxr::TransportRc& transport_rc) -> bool
    {
        bool rv = false;
        struct sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(struct sockaddr_in);

        int poll_rv = poll(&poll_fd, 1, timeout);
        if (0 < poll_rv)
        {
            ssize_t bytes_received =
                    recvfrom(poll_fd.fd,
                            buffer,
                            agent->buffer_size(),
                            0,
                            reinterpret_cast<struct sockaddr*>(&client_addr),
                            &client_addr_len);
            if (-1 != bytes_received)
            {
                input_packet.message.reset(new eprosima::uxr::InputMessage(
                    buffer, size_t(bytes_received)));
                uint32_t addr = client_addr.sin_addr.s_addr;
                uint16_t port = client_addr.sin_port;
                input_packet.source = eprosima::uxr::IPv4EndPoint(addr, port);
                rv = true;

                uint32_t raw_client_key = 0u;
                agent->get_client_key(
                    input_packet.source, raw_client_key);
                UXR_AGENT_LOG_MESSAGE(
                    UXR_DECORATE_YELLOW("[==>> UDP <<==]"),
                    raw_client_key,
                    input_packet.message->get_buf(),
                    input_packet.message->get_len());
            }
            else
            {
                transport_rc = eprosima::uxr::TransportRc::server_error;
            }
        }
        else
        {
            transport_rc = (0 == poll_rv) ?
                eprosima::uxr::TransportRc::timeout_error : eprosima::uxr::TransportRc::server_error;
        }

        if (rv)
        {
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN("This is an example of a custom Micro XRCE-DDS Agent RECV_MSG function"),
                "port: {}",
                agent_port);
        }

        return rv;
    };

    /**
     * @brief Agent's outcoming data flow definition.
     */
    auto send_msg_function = [&](
        eprosima::uxr::CustomAgent<eprosima::uxr::IPv4EndPoint>* agent,
        uint8_t* /*buffer*/,
        eprosima::uxr::OutputPacket<eprosima::uxr::IPv4EndPoint> output_packet,
        eprosima::uxr::TransportRc& transport_rc) -> bool
    {
        bool rv = false;
        struct sockaddr_in client_addr{};

        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = output_packet.destination.get_port();
        client_addr.sin_addr.s_addr = output_packet.destination.get_addr();

        ssize_t bytes_sent =
            sendto(
                poll_fd.fd,
                output_packet.message->get_buf(),
                output_packet.message->get_len(),
                0,
                reinterpret_cast<struct sockaddr*>(&client_addr),
                sizeof(client_addr));
        if (-1 != bytes_sent)
        {
            if (size_t(bytes_sent) == output_packet.message->get_len())
            {
                rv = true;
                uint32_t raw_client_key = 0u;
                agent->get_client_key(output_packet.destination, raw_client_key);
                UXR_AGENT_LOG_MESSAGE(
                    UXR_DECORATE_YELLOW("[** <<UDP>> **]"),
                    raw_client_key,
                    output_packet.message->get_buf(),
                    output_packet.message->get_len());
            }
        }
        else
        {
            transport_rc = eprosima::uxr::TransportRc::server_error;
        }

        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("This is an example of a custom Micro XRCE-DDS Agent SEND_MSG function"),
            "port: {}",
            agent_port);

        return rv;
    };

    /**
     * @brief Agent's error handling. This will be called, additionally, to init() and fini() methods.
     */
    auto handle_error_function = [&](
            eprosima::uxr::CustomAgent<eprosima::uxr::IPv4EndPoint>* /*agent*/,
            eprosima::uxr::TransportRc /*transport_rc*/) -> bool
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("This is an example of a custom Micro XRCE-DDS Agent HANDLE_ERROR function"),
            "port: {}",
            agent_port);

        return true;
    };

    /**
     * Create a custom agent instance.
     */
    eprosima::uxr::CustomAgent<eprosima::uxr::IPv4EndPoint> custom_agent(
        mw_kind,
        std::move(init_function),
        std::move(fini_function),
        std::move(recv_msg_function),
        std::move(send_msg_function),
        std::move(handle_error_function));

    /**
     * Set verbosity level
     */
    custom_agent.set_verbose_level(6);

    /**
     * Run agent and wait until receiving an stop signal.
     */
    custom_agent.start();

    int n_signal = 0;
    sigset_t signals;
    sigwait(&signals, &n_signal);

    /**
     * Stop agent, and exit.
     */
    custom_agent.stop();
    return 0;
}