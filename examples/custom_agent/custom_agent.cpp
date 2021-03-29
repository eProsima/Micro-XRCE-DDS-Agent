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
    eprosima::uxr::CustomAgent::InitFunction init_function = [&]() -> bool
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

            if (-1 != bind(poll_fd.fd,
                           reinterpret_cast<struct sockaddr*>(&address),
                           sizeof(address)))
            {
                poll_fd.events = POLLIN;
                rv = true;

                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN(
                        "This is an example of a custom Micro XRCE-DDS Agent INIT function"),
                    "port: {}",
                    agent_port);
            }
        }

        return rv;
    };

    /**
     * @brief Agent's destruction actions.
     */
    eprosima::uxr::CustomAgent::FiniFunction fini_function = [&]() -> bool
    {
        if (-1 == poll_fd.fd)
        {
            return true;
        }

        if (0 == ::close(poll_fd.fd))
        {
            poll_fd.fd = -1;

            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN(
                    "This is an example of a custom Micro XRCE-DDS Agent FINI function"),
                "port: {}",
                agent_port);

            return true;
        }
        else
        {
            return false;
        }
    };

    /**
     * @brief Agent's incoming data functionality.
     */
    eprosima::uxr::CustomAgent::RecvMsgFunction recv_msg_function = [&](
            eprosima::uxr::CustomEndPoint* source_endpoint,
            uint8_t* buffer,
            size_t buffer_length,
            int timeout,
            eprosima::uxr::TransportRc& transport_rc) -> ssize_t
    {
        struct sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(struct sockaddr_in);
        ssize_t bytes_received = -1;

        int poll_rv = poll(&poll_fd, 1, timeout);

        if (0 < poll_rv)
        {
            bytes_received = recvfrom(
                poll_fd.fd,
                buffer,
                buffer_length,
                0,
                reinterpret_cast<struct sockaddr *>(&client_addr),
                &client_addr_len);

            transport_rc = (-1 != bytes_received)
                ? eprosima::uxr::TransportRc::ok
                : eprosima::uxr::TransportRc::server_error;
        }
        else
        {
            transport_rc = (0 == poll_rv)
                ? eprosima::uxr::TransportRc::timeout_error
                : eprosima::uxr::TransportRc::server_error;
            bytes_received = 0;
        }

        if (eprosima::uxr::TransportRc::ok == transport_rc)
        {
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN(
                    "This is an example of a custom Micro XRCE-DDS Agent RECV_MSG function"),
                "port: {}",
                agent_port);
            source_endpoint->set_member_value<uint32_t>("address",
                static_cast<uint32_t>(client_addr.sin_addr.s_addr));
            source_endpoint->set_member_value<uint16_t>("port",
                static_cast<uint16_t>(client_addr.sin_port));
        }


        return bytes_received;
    };

    /**
     * @brief Agent's outcoming data flow definition.
     */
    eprosima::uxr::CustomAgent::SendMsgFunction send_msg_function = [&](
        const eprosima::uxr::CustomEndPoint* destination_endpoint,
        uint8_t* buffer,
        size_t message_length,
        eprosima::uxr::TransportRc& transport_rc) -> ssize_t
    {
        struct sockaddr_in client_addr{};

        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = destination_endpoint->get_member<uint16_t>("port");
        client_addr.sin_addr.s_addr = destination_endpoint->get_member<uint32_t>("address");

        ssize_t bytes_sent =
            sendto(
                poll_fd.fd,
                buffer,
                message_length,
                0,
                reinterpret_cast<struct sockaddr*>(&client_addr),
                sizeof(client_addr));

        transport_rc = (-1 != bytes_sent)
            ? eprosima::uxr::TransportRc::ok
            : eprosima::uxr::TransportRc::server_error;

        if (eprosima::uxr::TransportRc::ok == transport_rc)
        {
            UXR_AGENT_LOG_INFO(
                UXR_DECORATE_GREEN(
                    "This is an example of a custom Micro XRCE-DDS Agent SEND_MSG function"),
                "port: {}",
                agent_port);
        }

        return bytes_sent;
    };

    /**
     * Run the main application.
     */
    try
    {
        /**
         * EndPoint definition for this transport. We define an address and a port.
         */
        eprosima::uxr::CustomEndPoint custom_endpoint;
        custom_endpoint.add_member<uint32_t>("address");
        custom_endpoint.add_member<uint16_t>("port");

        /**
         * Create a custom agent instance.
         */
        eprosima::uxr::CustomAgent custom_agent(
            "UDPv4_CUSTOM",
            &custom_endpoint,
            mw_kind,
            false,
            init_function,
            fini_function,
            send_msg_function,
            recv_msg_function);

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
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
}