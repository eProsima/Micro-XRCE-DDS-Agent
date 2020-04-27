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

#include <uxr/agent/transport/tcp/TCPv4AgentWindows.hpp>
#include <uxr/agent/transport/util/InterfaceWindows.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <string.h>

namespace eprosima {
namespace uxr {

const uint8_t max_attemps = 16;

extern template class DiscoveryServer<IPv4EndPoint>; // Explicit instantiation declaration.
extern template class DiscoveryServerWindows<IPv4EndPoint>; // Explicit instantiation declaration.

TCPv4Agent::TCPv4Agent(
        uint16_t agent_port,
        Middleware::Kind middleware_kind)
    : Server<IPv4EndPoint>{middleware_kind}
    , TCPServerBase{}
    , connections_{}
    , active_connections_{}
    , free_connections_{}
    , listener_poll_{}
    , poll_fds_{}
    , buffer_{0}
    , agent_port_{agent_port}
    , listener_thread_{}
    , running_cond_{false}
    , messages_queue_{}
#ifdef UAGENT_DISCOVERY_PROFILE
    , discovery_server_(*processor_)
#endif
{}

TCPv4Agent::~TCPv4Agent()
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

bool TCPv4Agent::init()
{
    bool rv = false;

    /* Listener socket initialization. */
    listener_poll_.fd = socket(PF_INET, SOCK_STREAM, 0);

    if (INVALID_SOCKET != listener_poll_.fd)
    {
        /* IP and Port setup. */
        struct sockaddr_in address;

        ZeroMemory(&address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(agent_port_);
        address.sin_addr.s_addr = INADDR_ANY;

        if (SOCKET_ERROR != bind(listener_poll_.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)))
        {
            /* Log. */
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_GREEN("port opened"),
                "port: {}",
                agent_port_);

            /* Setup listener poll. */
            listener_poll_.events = POLLIN;

            /* Setup connections. */
            for (size_t i = 0; i < poll_fds_.size(); ++i)
            {
                poll_fds_[i].fd = INVALID_SOCKET;
                poll_fds_[i].events = POLLIN;
                connections_[i].poll_fd = &poll_fds_[i];
                connections_[i].id = uint32_t(i);
                connections_[i].active = false;
                init_input_buffer(connections_[i].input_buffer);
                free_connections_.push_back(connections_[i].id);
            }

            /* Init listener. */
            if (SOCKET_ERROR != listen(listener_poll_.fd, TCP_MAX_BACKLOG_CONNECTIONS))
            {
                running_cond_ = true;
                listener_thread_ = std::thread(&TCPv4Agent::listener_loop, this);
                rv = true;

                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("running..."),
                    "port: {}",
                    agent_port_);
            }
            else
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("listen error"),
                    "port: {}, errno: {}",
                    agent_port_, errno);
            }
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("bind error"),
                "port: {}, errno: {}",
                agent_port_, errno);
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "port: {}, errno: {}",
            agent_port_, errno);
    }
    return rv;
}

bool TCPv4Agent::fini()
{
    /* Stop listener thread. */
    running_cond_ = false;
    if (listener_thread_.joinable())
    {
        listener_thread_.join();
    }

    /* Close listener. */
    if (INVALID_SOCKET != listener_poll_.fd)
    {
        if (0 == closesocket(listener_poll_.fd))
        {
            listener_poll_.fd = INVALID_SOCKET;
        }
    }

    /* Disconnect clients. */
    for (auto& conn : connections_)
    {
        close_connection(conn);
    }

    std::lock_guard<std::mutex> lock(connections_mtx_);

    bool rv = false;
    if ((INVALID_SOCKET == listener_poll_.fd) && (active_connections_.empty()))
    {
        rv = true;
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("server stopped"),
            "port: {}",
            agent_port_);
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("socket error"),
            "port: {}, errno: {}",
            agent_port_, errno);
    }
    return rv;
}

#ifdef UAGENT_DISCOVERY_PROFILE
bool TCPv4Agent::init_discovery(uint16_t discovery_port)
{
    std::vector<dds::xrce::TransportAddress> transport_addresses;
    util::get_transport_interfaces<IPv4EndPoint>(this->agent_port_, transport_addresses);
    return discovery_server_.run(discovery_port, transport_addresses);
}

bool TCPv4Agent::fini_discovery()
{
    return discovery_server_.stop();
}
#endif

bool TCPv4Agent::recv_message(
        InputPacket<IPv4EndPoint>& input_packet,
        int timeout,
        TransportRc& transport_rc)
{
    bool rv = true;
    if (messages_queue_.empty() && !read_message(timeout, transport_rc))
    {
        rv = false;
    }
    else
    {
        input_packet = std::move(messages_queue_.front());
        messages_queue_.pop();

        uint32_t raw_client_key = 0u;
        Server<IPv4EndPoint>::get_client_key(input_packet.source, raw_client_key);
        UXR_AGENT_LOG_MESSAGE(
            UXR_DECORATE_YELLOW("[==>> TCP <<==]"),
            raw_client_key,
            input_packet.message->get_buf(),
            input_packet.message->get_len());
    }
    return rv;
}

bool TCPv4Agent::send_message(
        OutputPacket<IPv4EndPoint> output_packet,
        TransportRc& transport_rc)
{
    bool rv = false;
    uint8_t msg_size_buf[2];
    transport_rc = TransportRc::connection_error;

    std::unique_lock<std::mutex> lock(connections_mtx_);
    auto it = endpoint_to_connection_map_.find(output_packet.destination);
    if (it != endpoint_to_connection_map_.end())
    {
        TCPv4ConnectionWindows& connection = connections_.at(it->second);
        lock.unlock();

        msg_size_buf[0] = uint8_t(0x00FF & output_packet.message->get_len());
        msg_size_buf[1] = uint8_t((0xFF00 & output_packet.message->get_len()) >> 8);
        uint8_t n_attemps = 0;
        uint16_t bytes_sent = 0;

        /* Send message size. */
        bool size_sent = false;
        do
        {
            size_t send_rv = send_data(connection, msg_size_buf, 2, transport_rc);
            if (0 < send_rv)
            {
                bytes_sent += uint16_t(send_rv);
                size_sent = (bytes_sent == 2);
            }
            else
            {
                if (TransportRc::ok != transport_rc)
                {
                    break;
                }
            }
            ++n_attemps;
        }
        while (!size_sent && n_attemps < max_attemps);

        /* Send message payload. */
        bool payload_sent = false;
        if (size_sent)
        {
            n_attemps = 0;
            bytes_sent = 0;
            do
            {
                size_t send_rv =
                    send_data(
                        connection,
                        (output_packet.message->get_buf() + bytes_sent),
                        size_t(output_packet.message->get_len() - bytes_sent),
                        transport_rc);
                if (0 < send_rv)
                {
                    bytes_sent += uint16_t(send_rv);
                    payload_sent = (bytes_sent == uint16_t(output_packet.message->get_len()));
                }
                else
                {
                    if (TransportRc::ok != transport_rc)
                    {
                        break;
                    }
                }
                ++n_attemps;
            }
            while (!payload_sent && n_attemps < max_attemps);
        }

        if (payload_sent)
        {
            rv = true;

            uint32_t raw_client_key = 0u;
            Server<IPv4EndPoint>::get_client_key(output_packet.destination, raw_client_key);
            UXR_AGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[** <<TCP>> **]"),
                raw_client_key,
                output_packet.message->get_buf(),
                output_packet.message->get_len());
        }

        if (TransportRc::connection_error == transport_rc)
        {
            close_connection(connection);
        }
    }

    return rv;
}

bool TCPv4Agent::handle_error(
        TransportRc /*transport_rc*/)
{
    return fini() && init();
}

bool TCPv4Agent::open_connection(
        SOCKET fd,
        struct sockaddr_in& sockaddr)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(connections_mtx_);
    if (!free_connections_.empty())
    {
        uint32_t id = free_connections_.front();
        TCPv4ConnectionWindows& connection = connections_[id];
        connection.poll_fd->fd = fd;
        connection.endpoint = IPv4EndPoint(sockaddr.sin_addr.s_addr, sockaddr.sin_port);
        connection.active = true;
        init_input_buffer(connection.input_buffer);

        endpoint_to_connection_map_[connection.endpoint] = connection.id;
        active_connections_.insert(id);
        free_connections_.pop_front();
        rv = true;
    }
    return rv;
}

bool TCPv4Agent::close_connection(
        TCPv4ConnectionWindows& connection)
{
    bool rv = false;
    std::unique_lock<std::mutex> lock(connections_mtx_);
    auto it_conn = active_connections_.find(connection.id);
    if (it_conn != active_connections_.end())
    {
        lock.unlock();
        /* Add lock for close. */
        std::unique_lock<std::mutex> conn_lock(connection.mtx);
        if (0 == closesocket(connection.poll_fd->fd))
        {
            connection.poll_fd->fd = INVALID_SOCKET;
            connection.active = false;
            conn_lock.unlock();

            /* Clear connections map and lists. */
            lock.lock();
            endpoint_to_connection_map_.erase(connection.endpoint);
            active_connections_.erase(it_conn);
            free_connections_.push_back(connection.id);
            lock.unlock();

            rv = true;
        }
    }
    return rv;
}

void TCPv4Agent::init_input_buffer(TCPInputBuffer& buffer)
{
    buffer.state = TCP_BUFFER_EMPTY;
    buffer.msg_size = 0;
}

bool TCPv4Agent::read_message(
        int timeout,
        TransportRc& transport_rc)
{
    std::unique_lock<std::mutex> lock(connections_mtx_);
    if (active_connections_.empty())
    {
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        return false;
    }
    lock.unlock();

    bool rv = false;
    int poll_rv = WSAPoll(poll_fds_.data(), ULONG(poll_fds_.size()), timeout);
    if (0 < poll_rv)
    {
        for (auto& conn : connections_)
        {
            if (0 < (POLLIN & conn.poll_fd->revents))
            {
                uint16_t bytes_read = read_data(conn, transport_rc);
                if (TransportRc::ok == transport_rc)
                {
                    if (0 < bytes_read)
                    {
                        InputPacket<IPv4EndPoint> input_packet;
                        input_packet.message.reset(new InputMessage(conn.input_buffer.buffer.data(), bytes_read));
                        input_packet.source = conn.endpoint;
                        messages_queue_.push(std::move(input_packet));
                        rv = true;
                    }
                }
                else
                {
                    if (TransportRc::connection_error == transport_rc)
                    {
                        close_connection(conn);
                    }
                }
            }
            else
            {
                transport_rc = TransportRc::connection_error;
            }
        }
    }
    else
    {
        transport_rc = (0 == poll_rv) ? TransportRc::timeout_error : TransportRc::server_error;
    }
    return rv;
}

void TCPv4Agent::listener_loop()
{
    while (running_cond_)
    {
        int poll_rv = WSAPoll(&listener_poll_, 1, 100);
        if (0 < poll_rv)
        {
            if (0 < (POLLIN & listener_poll_.revents))
            {
                if (connection_available())
                {
                    struct sockaddr_in client_addr{};
                    int client_addr_len = sizeof(client_addr);
                    SOCKET incoming_fd =
                        accept(
                            listener_poll_.fd,
                            reinterpret_cast<struct sockaddr*>(&client_addr),
                            &client_addr_len);
                    if (INVALID_SOCKET != incoming_fd)
                    {
                        open_connection(incoming_fd, client_addr);
                    }
                }
            }
        }
    }
}

bool TCPv4Agent::connection_available()
{
    std::lock_guard<std::mutex> lock(connections_mtx_);
    return !free_connections_.empty();
}

size_t TCPv4Agent::recv_data(
        TCPv4ConnectionWindows& connection,
        uint8_t* buffer,
        size_t len,
        TransportRc& transport_rc)
{
    size_t rv = 0;
    std::lock_guard<std::mutex> lock(connection.mtx);
    if (connection.active)
    {
        int poll_rv = WSAPoll(connection.poll_fd, 1, 0);
        if (0 < poll_rv)
        {
            int bytes_received = recv(connection.poll_fd->fd, reinterpret_cast<char*>(buffer), int(len), 0);
            if (SOCKET_ERROR != bytes_received)
            {
                rv = size_t(bytes_received);
                transport_rc = TransportRc::ok;
            }
            else
            {
                transport_rc = TransportRc::connection_error;
            }
        }
        else
        {
            transport_rc = (0 == poll_rv)
                ? TransportRc::timeout_error
                : TransportRc::connection_error;
        }
    }
    else
    {
        transport_rc = TransportRc::connection_error;
    }
    return rv;
}

size_t TCPv4Agent::send_data(
        TCPv4ConnectionWindows& connection,
        uint8_t* buffer,
        size_t len,
        TransportRc& transport_rc)
{
    size_t rv = 0;
    std::lock_guard<std::mutex> lock(connection.mtx);
    if (connection.active)
    {
        int bytes_sent = send(connection.poll_fd->fd, reinterpret_cast<char*>(buffer), int(len), 0);
        if (SOCKET_ERROR != bytes_sent)
        {
            rv = size_t(bytes_sent);
            transport_rc = TransportRc::ok;
        }
        else
        {
            transport_rc = TransportRc::connection_error;
        }
    }
    else
    {
        transport_rc = TransportRc::connection_error;
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
