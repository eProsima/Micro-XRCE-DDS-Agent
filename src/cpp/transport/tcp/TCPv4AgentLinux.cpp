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

#include <uxr/agent/transport/tcp/TCPv4AgentLinux.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <functional>

namespace eprosima {
namespace uxr {

const uint8_t max_attemps = 16;

TCPv4Agent::TCPv4Agent(
        uint16_t agent_port,
        Middleware::Kind middleware_kind)
    : TCPv4ServerBase{agent_port, middleware_kind}
    , connections_{}
    , active_connections_{}
    , free_connections_{}
    , listener_poll_{}
    , poll_fds_{}
    , buffer_{0}
    , listener_thread_{}
    , running_cond_{false}
    , messages_queue_{}
#ifdef UAGENT_DISCOVERY_PROFILE
    , discovery_server_{*processor_}
#endif
#ifdef UAGENT_P2P_PROFILE
    , agent_discoverer_{*this}
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

    /* Ignore SIGPIPE signal. */
    signal(SIGPIPE, sigpipe_handler);

    /* Listener socket initialization. */
    listener_poll_.fd = socket(PF_INET, SOCK_STREAM, 0);

    if (-1 != listener_poll_.fd)
    {
        /* IP and Port setup. */
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(transport_address_.medium_locator().port());
        address.sin_addr.s_addr = INADDR_ANY;
        memset(address.sin_zero, '\0', sizeof(address.sin_zero));
        if (-1 != bind(listener_poll_.fd, (struct sockaddr*)&address, sizeof(address)))
        {
            /* Log. */
            UXR_AGENT_LOG_DEBUG(
                UXR_DECORATE_GREEN("port opened"),
                "port: {}",
                transport_address_.medium_locator().port());

            /* Setup listener poll. */
            listener_poll_.events = POLLIN;

            /* Setup connections. */
            for (size_t i = 0; i < poll_fds_.size(); ++i)
            {
                poll_fds_[i].fd = -1;
                poll_fds_[i].events = POLLIN;
                connections_[i].poll_fd = &poll_fds_[i];
                connections_[i].id = uint32_t(i);
                connections_[i].active = false;
                init_input_buffer(connections_[i].input_buffer);
                free_connections_.push_back(connections_[i].id);
            }

            /* Init listener. */
            if (-1 != listen(listener_poll_.fd, TCP_MAX_BACKLOG_CONNECTIONS))
            {
                running_cond_ = true;
                listener_thread_ = std::thread(&TCPv4Agent::listener_loop, this);

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
                    UXR_DECORATE_RED("listen error"),
                    "port: {}",
                    transport_address_.medium_locator().port());
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

bool TCPv4Agent::close()
{
    /* Stop listener thread. */
    running_cond_ = false;
    if (listener_thread_.joinable())
    {
        listener_thread_.join();
    }

    /* Close listener. */
    if (-1 != listener_poll_.fd)
    {
        if (0 == ::close(listener_poll_.fd))
        {
            listener_poll_.fd = -1;
        }
    }

    /* Disconnect clients. */
    for (auto& conn : connections_)
    {
        close_connection(conn);
    }

    std::lock_guard<std::mutex> lock(connections_mtx_);

    bool rv = false;
    if ((-1 == listener_poll_.fd) && (active_connections_.empty()))
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("server stopped"),
            "port: {}",
            transport_address_.medium_locator().port());
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
bool TCPv4Agent::init_discovery(uint16_t discovery_port)
{
    return discovery_server_.run(discovery_port, transport_address_);
}

bool TCPv4Agent::close_discovery()
{
    return discovery_server_.stop();
}
#endif

#ifdef UAGENT_P2P_PROFILE
bool TCPv4Agent::init_p2p(uint16_t p2p_port)
{
#ifdef UAGENT_DISCOVERY_PROFILE
    discovery_server_.set_filter_port(p2p_port);
#endif
    return agent_discoverer_.run(p2p_port, transport_address_);
}

bool TCPv4Agent::close_p2p()
{
#ifdef UAGENT_DISCOVERY_PROFILE
    discovery_server_.set_filter_port(0);
#endif
    return agent_discoverer_.stop();
}
#endif

bool TCPv4Agent::recv_message(
        InputPacket& input_packet,
        int timeout)
{
    bool rv = true;
    if (messages_queue_.empty() && !read_message(timeout))
    {
        rv = false;
    }
    else
    {
        input_packet = std::move(messages_queue_.front());
        messages_queue_.pop();
        UXR_AGENT_LOG_MESSAGE(
            UXR_DECORATE_YELLOW("[==>> TCP <<==]"),
            conversion::clientkey_to_raw(get_client_key(input_packet.source.get())),
            input_packet.message->get_buf(),
            input_packet.message->get_len());
    }
    return rv;
}

bool TCPv4Agent::send_message(OutputPacket output_packet)
{
    bool rv = false;
    uint8_t msg_size_buf[2];
    const IPv4EndPoint* destination = static_cast<const IPv4EndPoint*>(output_packet.destination.get());
    uint64_t source_id = (uint64_t(destination->get_addr()) << 16) | destination->get_port();

    std::unique_lock<std::mutex> lock(connections_mtx_);
    auto it = source_to_connection_map_.find(source_id);
    if (it != source_to_connection_map_.end())
    {
        TCPConnection& connection = connections_.at(it->second);
        lock.unlock();

        msg_size_buf[0] = uint8_t(0x00FF & output_packet.message->get_len());
        msg_size_buf[1] = uint8_t((0xFF00 & output_packet.message->get_len()) >> 8);
        uint8_t n_attemps = 0;
        uint16_t bytes_sent = 0;

        /* Send message size. */
        bool size_sent = false;
        do
        {
            uint8_t errcode;
            size_t send_rv = send_locking(connection, msg_size_buf, 2, errcode);
            if (0 < send_rv)
            {
                bytes_sent += uint16_t(send_rv);
                size_sent = (bytes_sent == 2);
            }
            else
            {
                if (0 < errcode)
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
                uint8_t errcode;
                size_t send_rv = send_locking(connection,
                                              output_packet.message->get_buf() + bytes_sent,
                                              output_packet.message->get_len() - bytes_sent,
                                              errcode);
                if (0 < send_rv)
                {
                    bytes_sent += uint16_t(send_rv);
                    payload_sent = (bytes_sent == uint16_t(output_packet.message->get_len()));
                }
                else
                {
                    if (0 < errcode)
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
            UXR_AGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[** <<TCP>> **]"),
                conversion::clientkey_to_raw(get_client_key(output_packet.destination.get())),
                output_packet.message->get_buf(),
                output_packet.message->get_len());
            rv = true;
        }
        else
        {
            close_connection(connection);
        }
    }

    return rv;
}

int TCPv4Agent::get_error()
{
    return errno;
}

bool TCPv4Agent::open_connection(
        int fd,
        struct sockaddr_in* sockaddr)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(connections_mtx_);
    if (!free_connections_.empty())
    {
        uint32_t id = free_connections_.front();
        TCPConnectionPlatform& connection = connections_[size_t(id)];
        connection.poll_fd->fd = fd;
        connection.addr = sockaddr->sin_addr.s_addr;
        connection.port = sockaddr->sin_port;
        connection.active = true;
        init_input_buffer(connection.input_buffer);

        uint64_t source_id = (uint64_t(connection.addr) << 16) | connection.port;
        source_to_connection_map_[source_id] = connection.id;
        active_connections_.insert(id);
        free_connections_.pop_front();
        rv = true;
    }
    return rv;
}

bool TCPv4Agent::close_connection(TCPConnection& connection)
{
    bool rv = false;
    TCPConnectionPlatform& connection_platform = static_cast<TCPConnectionPlatform&>(connection);
    std::unique_lock<std::mutex> lock(connections_mtx_);
    auto it_conn = active_connections_.find(connection.id);
    if (it_conn != active_connections_.end())
    {
        lock.unlock();
        /* Add lock for close. */
        std::unique_lock<std::mutex> conn_lock(connection.mtx);
        if (0 == ::close(connection_platform.poll_fd->fd))
        {
            connection_platform.poll_fd->fd = -1;
            connection.active = false;
            conn_lock.unlock();

            uint64_t source_id = (uint64_t(connection.addr) << 16) | connection.port;

            /* Clear connections map and lists. */
            lock.lock();
            source_to_connection_map_.erase(source_id);
            active_connections_.erase(it_conn);
            free_connections_.push_back(connection.id);
            lock.unlock();

            std::unique_lock<std::mutex> client_lock(clients_mtx_);
            auto it_client = source_to_client_map_.find(source_id);
            if (it_client != source_to_client_map_.end())
            {
                client_to_source_map_.erase(it_client->second);
                source_to_client_map_.erase(it_client->first);
            }
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

bool TCPv4Agent::read_message(int timeout)
{
    bool rv = false;
    int poll_rv = poll(poll_fds_.data(), poll_fds_.size(), timeout);
    if (0 < poll_rv)
    {
        for (auto& conn : connections_)
        {
            if (POLLIN == (POLLIN & conn.poll_fd->revents))
            {
                uint16_t bytes_read = read_data(conn);
                if (0 < bytes_read)
                {
                    InputPacket input_packet;
                    input_packet.message.reset(new InputMessage(conn.input_buffer.buffer.data(), bytes_read));
                    input_packet.source.reset(new IPv4EndPoint(conn.addr, conn.port));
                    messages_queue_.push(std::move(input_packet));
                    rv = true;
                }
            }
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

void TCPv4Agent::listener_loop()
{
    while (running_cond_)
    {
        int poll_rv = poll(&listener_poll_, 1, 100);
        if (0 < poll_rv)
        {
            if (POLLIN == (POLLIN & listener_poll_.revents))
            {
                if (connection_available())
                {
                    /* New client connection. */
                    struct sockaddr client_addr;
                    socklen_t client_addr_len = sizeof(client_addr);
                    int incoming_fd = accept(listener_poll_.fd, &client_addr, &client_addr_len);
                    if (-1 != incoming_fd)
                    {
                        /* Open connection. */
                        open_connection(incoming_fd, (struct sockaddr_in*)&client_addr);
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

size_t TCPv4Agent::recv_locking(
        TCPConnection& connection,
        uint8_t* buffer,
        size_t len,
        uint8_t& errcode)
{
    size_t rv = 0;
    TCPConnectionPlatform& connection_platform = static_cast<TCPConnectionPlatform&>(connection);
    std::lock_guard<std::mutex> lock(connection.mtx);
    if (connection.active)
    {
        int poll_rv = poll(connection_platform.poll_fd, 1, 0);
        if (0 < poll_rv)
        {
            ssize_t bytes_received = recv(connection_platform.poll_fd->fd, (void*)buffer, len, 0);
            if (0 < bytes_received)
            {
                rv = size_t(bytes_received);
                errcode = 0;
            }
            else
            {
                errcode = 1;
            }
        }
        else
        {
            errcode = (0 == poll_rv) ? 0 : 1;
        }
    }
    return rv;
}

size_t TCPv4Agent::send_locking(
        TCPConnection& connection,
        uint8_t* buffer,
        size_t len,
        uint8_t& errcode)
{
    size_t rv = 0;
    TCPConnectionPlatform& connection_platform = static_cast<TCPConnectionPlatform&>(connection);
    std::lock_guard<std::mutex> lock(connection.mtx);
    if (connection.active)
    {
        ssize_t bytes_sent = send(connection_platform.poll_fd->fd, (void*)buffer, len, 0);
        if (-1 != bytes_sent)
        {
            rv = size_t(bytes_sent);
            errcode = 0;
        }
        else
        {
            errcode = 1;
        }
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
