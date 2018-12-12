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

#include <uxr/agent/transport/tcp/TCPServerWindows.hpp>
#include <string.h>

namespace eprosima {
namespace uxr {

const uint8_t max_attemps = 16;

TCPServer::TCPServer(uint16_t port)
    : TCPServerBase(port),
      connections_{},
      active_connections_(),
      free_connections_(),
      listener_poll_{},
      poll_fds_{},
      buffer_{0},
      listener_thread_(),
      running_cond_(false),
      messages_queue_{}
{}

bool TCPServer::init()
{
    bool rv = false;

    /* Socket initialization. */
    poll_fds_[0].fd = socket(PF_INET, SOCK_STREAM, 0);

    /* Listener socket initialization. */
    listener_poll_.fd = socket(PF_INET, SOCK_STREAM, 0);

    if (INVALID_SOCKET != listener_poll_.fd)
    {
        /* IP and Port setup. */
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(port_);
        address.sin_addr.s_addr = INADDR_ANY;
        memset(address.sin_zero, '\0', sizeof(address.sin_zero));
        if (SOCKET_ERROR != bind(listener_poll_.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)))
        {
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
                listener_thread_.reset(new std::thread(std::bind(&TCPServer::listener_loop, this)));
                rv = true;
            }
        }
    }
    return rv;
}

bool TCPServer::close()
{
    /* Stop listener thread. */
    running_cond_ = false;
    if (listener_thread_ && listener_thread_->joinable())
    {
        listener_thread_->join();
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
    return (INVALID_SOCKET == listener_poll_.fd) && (active_connections_.empty());
}

bool TCPServer::recv_message(InputPacket& input_packet, int timeout)
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
    }
    return rv;
}

bool TCPServer::send_message(OutputPacket output_packet)
{
    bool rv = false;
    uint8_t msg_size_buf[2];
    const TCPEndPoint* destination = static_cast<const TCPEndPoint*>(output_packet.destination.get());
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
            size_t send_rv = send_locking(connection, msg_size_buf, size_t(2), errcode);
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
            bytes_sent = 0;
            do
            {
                uint8_t errcode;
                size_t send_rv = send_locking(connection,
                                              (output_packet.message->get_buf() + bytes_sent),
                                              size_t(output_packet.message->get_len() - bytes_sent),
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
            }
            while (!payload_sent && n_attemps < max_attemps);
        }

        if (size_sent && payload_sent)
        {
            rv = true;
        }
        else
        {
            close_connection(connection);
        }
    }

    return rv;
}

int TCPServer::get_error()
{
    return WSAGetLastError();
}

bool TCPServer::open_connection(SOCKET fd, struct sockaddr_in* sockaddr)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(connections_mtx_);
    if (!free_connections_.empty())
    {
        uint32_t id = free_connections_.front();
        TCPConnectionPlatform& connection = connections_[id];
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

bool TCPServer::close_connection(TCPConnection& connection)
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
        if (0 == closesocket(connection_platform.poll_fd->fd))
        {
            connection_platform.poll_fd->fd = INVALID_SOCKET;
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

void TCPServer::init_input_buffer(TCPInputBuffer& buffer)
{
    buffer.state = TCP_BUFFER_EMPTY;
    buffer.msg_size = 0;
}

bool TCPServer::read_message(int timeout)
{
    bool rv = false;
    int poll_rv = WSAPoll(poll_fds_.data(), ULONG(poll_fds_.size()), timeout);
    if (0 < poll_rv)
    {
        for (auto& conn : connections_)
        {
            if (0 < (POLLIN & conn.poll_fd->revents))
            {
                uint16_t bytes_read = read_data(conn);
                if (0 < bytes_read)
                {
                    InputPacket input_packet;
                    input_packet.message.reset(new InputMessage(conn.input_buffer.buffer.data(), bytes_read));
                    input_packet.source.reset(new TCPEndPoint(conn.addr, conn.port));
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
            WSASetLastError(WAIT_TIMEOUT);
        }
    }
    return rv;
}

void TCPServer::listener_loop()
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
                    /* New client connection. */
                    struct sockaddr client_addr;
                    int client_addr_len = sizeof(client_addr);
                    SOCKET incoming_fd = accept(listener_poll_.fd, &client_addr, &client_addr_len);
                    if (INVALID_SOCKET != incoming_fd)
                    {
                        /* Open connection. */
                        open_connection(incoming_fd, reinterpret_cast<struct sockaddr_in*>(&client_addr));
                    }
                }
            }
        }
    }
}

bool TCPServer::connection_available()
{
    std::lock_guard<std::mutex> lock(connections_mtx_);
    return !free_connections_.empty();
}

size_t TCPServer::recv_locking(TCPConnection& connection, uint8_t* buffer, size_t len, uint8_t& errcode)
{
    size_t rv = 0;
    TCPConnectionPlatform& connection_platform = static_cast<TCPConnectionPlatform&>(connection);
    std::lock_guard<std::mutex> lock(connection.mtx);
    if (connection.active)
    {
        int poll_rv = WSAPoll(connection_platform.poll_fd, 1, 0);
        if (0 < poll_rv)
        {
            int bytes_received = recv(connection_platform.poll_fd->fd, reinterpret_cast<char*>(buffer), int(len), 0);
            if (SOCKET_ERROR != bytes_received)
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

size_t TCPServer::send_locking(TCPConnection& connection, uint8_t* buffer, size_t len, uint8_t& errcode)
{
    size_t rv = 0;
    TCPConnectionPlatform& connection_platform = static_cast<TCPConnectionPlatform&>(connection);
    std::lock_guard<std::mutex> lock(connection.mtx);
    if (connection.active)
    {
        int bytes_sent = send(connection_platform.poll_fd->fd, reinterpret_cast<char*>(buffer), int(len), 0);
        if (SOCKET_ERROR != bytes_sent)
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
