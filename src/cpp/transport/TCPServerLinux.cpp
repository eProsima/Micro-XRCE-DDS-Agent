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

#include <uxr/agent/transport/TCPServerLinux.hpp>
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

TCPServer::TCPServer(uint16_t port)
    : port_(port),
      connections_{},
      active_connections_(),
      free_connections_(),
      listener_poll_{},
      poll_fds_{},
      buffer_{0},
      source_to_connection_map_{},
      source_to_client_map_{},
      client_to_source_map_{},
      clients_mtx_(),
      listener_thread_(),
      running_cond_(false),
      messages_queue{}
{}

void TCPServer::on_create_client(EndPoint* source, const dds::xrce::ClientKey& client_key)
{
    TCPEndPoint* endpoint = static_cast<TCPEndPoint*>(source);
    uint64_t source_id = ((uint64_t)endpoint->get_addr() << 16) | endpoint->get_port();
    uint32_t client_id = client_key.at(0) + (client_key.at(1) << 8) + (client_key.at(2) << 16) + (client_key.at(3) <<24);

    /* Update maps. */
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it_client = client_to_source_map_.find(client_id);
    if (it_client != client_to_source_map_.end())
    {
        source_to_client_map_.erase(it_client->second);
        it_client->second = source_id;
    }
    else
    {
        client_to_source_map_.insert(std::make_pair(client_id, source_id));
    }

    auto it_source = source_to_client_map_.find(source_id);
    if (it_source != source_to_client_map_.end())
    {
        it_source->second = client_id;
    }
    else
    {
        source_to_client_map_.insert(std::make_pair(source_id, client_id));
    }
}

void TCPServer::on_delete_client(EndPoint* source)
{
    TCPEndPoint* endpoint = static_cast<TCPEndPoint*>(source);
    uint64_t source_id = (endpoint->get_addr() << 16) | endpoint->get_port();

    /* Update maps. */
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it = source_to_client_map_.find(source_id);
    if (it != source_to_client_map_.end())
    {
        client_to_source_map_.erase(it->second);
        source_to_client_map_.erase(it->first);
    }
}

const dds::xrce::ClientKey TCPServer::get_client_key(EndPoint* source)
{
    dds::xrce::ClientKey client_key;
    TCPEndPoint* endpoint = static_cast<TCPEndPoint*>(source);
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it = source_to_client_map_.find(((uint64_t)endpoint->get_addr() << 16) | endpoint->get_port());
    if (it != source_to_client_map_.end())
    {
        client_key.at(0) = it->second & 0x000000FF;
        client_key.at(1) = (it->second & 0x0000FF00) >> 8;
        client_key.at(2) = (it->second & 0x00FF0000) >> 16;
        client_key.at(3) = (it->second & 0xFF000000) >> 24;
    }
    else
    {
        client_key = dds::xrce::CLIENTKEY_INVALID;
    }
    return client_key;
}

std::unique_ptr<EndPoint> TCPServer::get_source(const dds::xrce::ClientKey& client_key)
{
    std::unique_ptr<EndPoint> source;
    uint32_t client_id = client_key.at(0) + (client_key.at(1) << 8) + (client_key.at(2) << 16) + (client_key.at(3) <<24);
    std::lock_guard<std::mutex> lock(clients_mtx_);
    auto it = client_to_source_map_.find(client_id);
    if (it != client_to_source_map_.end())
    {
        uint64_t source_id = it->second;
        source.reset(new TCPEndPoint(source_id >> 16, source_id & 0xFFFF));
    }
    return source;
}

bool TCPServer::init()
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
        address.sin_port = htons(port_);
        address.sin_addr.s_addr = INADDR_ANY;
        memset(address.sin_zero, '\0', sizeof(address.sin_zero));
        if (-1 != bind(listener_poll_.fd, (struct sockaddr*)&address, sizeof(address)))
        {
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
                listener_thread_.reset(new std::thread(std::bind(&TCPServer::listener_loop, this)));
                rv = true;
            }
        }
    }
    return rv;
}

bool TCPServer::close()
{
    bool rv = false;

    /* Stop listener thread. */
    running_cond_ = false;
    if (listener_thread_ && listener_thread_->joinable())
    {
        /* Close listener. */
        listener_thread_->join();
        if (0 == ::close(listener_poll_.fd))
        {
            /* Disconnect clients. */
            for (auto& conn : connections_)
            {
                close_connection(conn);
            }
            std::lock_guard<std::mutex> lock(connections_mtx_);
            rv = active_connections_.empty();
        }
    }

    return rv;
}

bool TCPServer::recv_message(InputPacket& input_packet, int timeout)
{
    bool rv = true;
    if (messages_queue.empty() && !read_message(timeout))
    {
        rv = false;
    }
    else
    {
        input_packet = std::move(messages_queue.front());
        messages_queue.pop();
    }
    return rv;
}

bool TCPServer::send_message(OutputPacket output_packet)
{
    bool rv = true;
    uint16_t bytes_sent = 0;
    ssize_t send_rv = 0;
    uint8_t msg_size_buf[2];
    const TCPEndPoint* destination = static_cast<const TCPEndPoint*>(output_packet.destination.get());
    uint64_t source_id = ((uint64_t)destination->get_addr() << 16) | destination->get_port();

    std::unique_lock<std::mutex> lock(connections_mtx_);
    auto it = source_to_connection_map_.find(source_id);
    if (it != source_to_connection_map_.end())
    {
        TCPConnection& connection = connections_.at(it->second);
        lock.unlock();

        /* Send message size. */
        msg_size_buf[0] = (uint8_t)(0x00FF & output_packet.message->get_len());
        msg_size_buf[1] = (uint8_t)((0xFF00 & output_packet.message->get_len()) >> 8);
        do
        {
            send_rv = send_locking(connection, msg_size_buf, 2);
            if (0 <= send_rv)
            {
                bytes_sent += send_rv;
            }
            else
            {
                close_connection(connection);
                rv = false;
            }
        }
        while (rv && bytes_sent != 2);

        /* Send message payload. */
        if (rv)
        {
            bytes_sent = 0;
            do
            {
                send_rv = send_locking(connection,
                                       output_packet.message->get_buf() + bytes_sent,
                                       output_packet.message->get_len() - bytes_sent);
                if (0 <= send_rv)
                {
                    bytes_sent += send_rv;
                }
                else
                {
                    close_connection(connection);
                    rv = false;
                }
            }
            while (rv && bytes_sent != static_cast<uint16_t>(output_packet.message->get_len()));
        }
    }

    return rv;
}

int TCPServer::get_error()
{
    return errno;
}

uint16_t TCPServer::read_data(TCPConnection& connection)
{
    uint16_t rv = 0;
    bool exit_flag = false;

    /* State Machine. */
    while(!exit_flag)
    {
        switch (connection.input_buffer.state)
        {
            case TCP_BUFFER_EMPTY:
            {
                connection.input_buffer.position = 0;
                uint8_t size_buf[2];
                ssize_t bytes_received = recv_locking(connection, size_buf, 2);
                if (0 < bytes_received)
                {
                    connection.input_buffer.msg_size = 0;
                    if (2 == bytes_received)
                    {
                        connection.input_buffer.msg_size = (uint16_t)(size_buf[1] << 8) | (uint16_t)size_buf[0];
                        if (connection.input_buffer.msg_size != 0)
                        {
                            connection.input_buffer.state = TCP_SIZE_READ;
                        }
                        else
                        {
                            connection.input_buffer.state = TCP_BUFFER_EMPTY;
                        }
                    }
                    else
                    {
                        connection.input_buffer.msg_size = (uint16_t)size_buf[0];
                        connection.input_buffer.state = TCP_SIZE_INCOMPLETE;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    close_connection(connection);
                    exit_flag = true;
                }
                break;
            }
            case TCP_SIZE_INCOMPLETE:
            {
                uint8_t size_msb;
                ssize_t bytes_received = recv_locking(connection, &size_msb, 1);
                if (0 < bytes_received)
                {
                    connection.input_buffer.msg_size = (uint16_t)(size_msb << 8) | connection.input_buffer.msg_size;
                    if (connection.input_buffer.msg_size != 0)
                    {
                        connection.input_buffer.state = TCP_SIZE_READ;
                    }
                    else
                    {
                        connection.input_buffer.state = TCP_BUFFER_EMPTY;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    close_connection(connection);
                    exit_flag = true;
                }
                exit_flag = (0 == poll(connection.poll_fd, 1, 0));
                break;
            }
            case TCP_SIZE_READ:
            {
                connection.input_buffer.buffer.resize(connection.input_buffer.msg_size);
                ssize_t bytes_received = recv_locking(connection,
                                                      connection.input_buffer.buffer.data(),
                                                      connection.input_buffer.buffer.size());
                if (0 < bytes_received)
                {
                    if ((uint16_t)bytes_received == connection.input_buffer.msg_size)
                    {
                        connection.input_buffer.state = TCP_MESSAGE_AVAILABLE;
                    }
                    else
                    {
                        connection.input_buffer.position = (uint16_t)bytes_received;
                        connection.input_buffer.state = TCP_MESSAGE_INCOMPLETE;
                        exit_flag = true;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    close_connection(connection);
                    exit_flag = true;
                }
                break;
            }
            case TCP_MESSAGE_INCOMPLETE:
            {
                ssize_t bytes_received = recv_locking(connection,
                                                      connection.input_buffer.buffer.data() + connection.input_buffer.position,
                                                      connection.input_buffer.buffer.size() - connection.input_buffer.position);
                if (0 < bytes_received)
                {
                    connection.input_buffer.position += (uint16_t)bytes_received;
                    if (connection.input_buffer.position == connection.input_buffer.msg_size)
                    {
                        connection.input_buffer.state = TCP_MESSAGE_AVAILABLE;
                    }
                    else
                    {
                        exit_flag = true;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    close_connection(connection);
                    exit_flag = true;
                }
                break;
            }
            case TCP_MESSAGE_AVAILABLE:
            {
                rv = connection.input_buffer.msg_size;
                connection.input_buffer.state = TCP_BUFFER_EMPTY;
                exit_flag = true;
                break;
            }
            default:
                rv = 0;
                exit_flag = true;
                break;
        }
    }

    return rv;
}

bool TCPServer::open_connection(int fd, struct sockaddr_in* sockaddr)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(connections_mtx_);
    if (!free_connections_.empty())
    {
        size_t id = size_t(free_connections_.front());
        TCPConnection& connection = connections_[id];
        connection.poll_fd->fd = fd;
        connection.addr = sockaddr->sin_addr.s_addr;
        connection.port = sockaddr->sin_port;
        connection.active = true;

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
    std::unique_lock<std::mutex> lock(connections_mtx_);
    auto it_conn = active_connections_.find(connection.id);
    if (it_conn != active_connections_.end())
    {
        lock.unlock();
        /* Add lock for close. */
        std::unique_lock<std::mutex> conn_lock(connection.mtx);
        if (0 == ::close(connection.poll_fd->fd))
        {
            connection.poll_fd->fd = -1;
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
                    input_packet.source.reset(new TCPEndPoint(conn.addr, conn.port));
                    messages_queue.push(std::move(input_packet));
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

void TCPServer::listener_loop()
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

bool TCPServer::connection_available()
{
    std::lock_guard<std::mutex> lock(connections_mtx_);
    return !free_connections_.empty();
}

ssize_t TCPServer::recv_locking(TCPConnection& connection, void* buffer, size_t len)
{
    ssize_t rv = 0;
    std::lock_guard<std::mutex> lock(connection.mtx);
    if (connection.active)
    {
        rv = recv(connection.poll_fd->fd, buffer, len, 0);
    }
    return rv;
}

ssize_t TCPServer::send_locking(TCPConnection& connection, void* buffer, size_t len)
{
    ssize_t rv = 0;
    std::lock_guard<std::mutex> lock(connection.mtx);
    if (connection.active)
    {
        rv = send(connection.poll_fd->fd, buffer, len, 0);
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
