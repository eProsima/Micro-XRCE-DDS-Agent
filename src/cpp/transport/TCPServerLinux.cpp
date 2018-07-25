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

#include <micrortps/agent/transport/TCPServerLinux.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

namespace eprosima {
namespace micrortps {

TCPServer::TCPServer(uint16_t port)
    : port_(port),
      connections_{},
      active_connections_(),
      free_connections_(),
      last_connection_read_(),
      poll_fds_{},
      buffer_{0},
      source_to_connection_map_{},
      source_to_client_map_{},
      client_to_source_map_{}
{}

void TCPServer::on_create_client(EndPoint* source, const dds::xrce::ClientKey& client_key)
{
    TCPEndPoint* endpoint = static_cast<TCPEndPoint*>(source);
    uint64_t source_id = ((uint64_t)endpoint->get_addr() << 16) | endpoint->get_port();
    uint32_t client_id = client_key.at(0) + (client_key.at(1) << 8) + (client_key.at(2) << 16) + (client_key.at(3) <<24);

    /* Update maps. */
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

    /* Socket initialization. */
    poll_fds_[0].fd = socket(PF_INET, SOCK_STREAM, 0);

    if (-1 == poll_fds_[0].fd)
    {
        rv = errno;
    }
    else
    {
        /* IP and Port setup. */
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(port_);
        address.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(address.sin_zero, '\0', sizeof(address.sin_zero));
        if (-1 != bind(poll_fds_[0].fd, (struct sockaddr*)&address, sizeof(address)))
        {
            /* Listen Poll setup. */
            poll_fds_[0].events = POLLIN;

            /* Client polls setup. */
            for (unsigned int i = 1; i < poll_fds_.size(); ++i)
            {
                poll_fds_[i].fd = -1;
                poll_fds_[i].events = POLLIN;
            }

            /* Listener setup. */
            if (-1 != listen(poll_fds_[0].fd, MICRORTPS_MAX_BACKLOG_TCP_CONNECTIONS))
            {
                /* Client setup. */
                connections_[0].poll_fd = &poll_fds_[1];
                connections_[0].id = 0;
                init_input_buffer(&connections_[0].input_buffer);
                for (unsigned int i = 1; i < poll_fds_.size() - 1; ++i)
                {
                    connections_[i].poll_fd = &poll_fds_[i + 1];
                    init_input_buffer(&connections_[i].input_buffer);
                    connections_[i - 1].next = &connections_[i];
                    connections_[i].prev = &connections_[i - 1];
                    connections_[i].id = static_cast<uint32_t>(i);
                }
                connections_.back().next = &connections_.front();
                connections_.front().prev = &connections_.back();
                free_connections_ = &connections_[0];
                last_connection_read_ = free_connections_;
                rv = true;
            }
        }
    }
    return rv;
}

bool TCPServer::close()
{
    //TODO
    return true;
}

bool TCPServer::recv_message(InputPacket& input_packet, int timeout)
{
    bool rv = false;
    int poll_rv = poll(poll_fds_.data(), poll_fds_.size(), timeout);
    if (0 < poll_rv)
    {
        if (POLLIN == (POLLIN & poll_fds_[0].revents) && nullptr != free_connections_)
        {
            /* New client connection. */
            struct sockaddr client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int incoming_fd = accept(poll_fds_[0].fd, &client_addr, &client_addr_len);
            if (-1 != incoming_fd)
            {
                /* Update available clients list. */
                TCPConnection* incoming_connection = free_connections_;
                incoming_connection->poll_fd->fd = incoming_fd;
                incoming_connection->addr = ((struct sockaddr_in*)&client_addr)->sin_addr.s_addr;
                incoming_connection->port = ((struct sockaddr_in*)&client_addr)->sin_port;
                if (free_connections_ != free_connections_->next)
                {
                    free_connections_->prev->next = free_connections_->next;
                    free_connections_->next->prev = free_connections_->prev;
                    free_connections_ = free_connections_->next;
                }
                else
                {
                    free_connections_ = nullptr;
                }

                /* Update connected client list. */
                if (nullptr != active_connections_)
                {
                    incoming_connection->prev = active_connections_->prev;
                    incoming_connection->next = active_connections_;
                    active_connections_->prev->next = incoming_connection;
                    active_connections_->prev = incoming_connection;
                }
                else
                {
                    incoming_connection->prev = incoming_connection;
                    incoming_connection->next = incoming_connection;
                    active_connections_ = incoming_connection;
                    last_connection_read_ = incoming_connection;
                }
                uint64_t source_id = ((uint64_t)incoming_connection->addr << 16) | incoming_connection->port;
                source_to_connection_map_.insert(std::make_pair(source_id, incoming_connection->id));
            }
        }
        else
        {
            if (nullptr != active_connections_)
            {
                /**
                 * Receive Scheduler: the first client in attemp to read will be the neighbour of
                 *                    the last client form which data was read.
                 */
                TCPConnection* base_connection = last_connection_read_->next;
                TCPConnection* reader = last_connection_read_->next;
                do
                {
                    if (POLLIN == (POLLIN & reader->poll_fd->revents))
                    {
                        uint16_t bytes_read = read_data(reader);
                        if (0 < bytes_read)
                        {
                            input_packet.message.reset(new InputMessage(reader->input_buffer.buffer.data(), bytes_read));
                            last_connection_read_ = reader;
                            input_packet.source.reset(new TCPEndPoint(reader->addr, reader->port));
                            rv = true;
                            break;
                        }
                    }
                    reader = reader->next;

                }
                while (reader != base_connection);
            }
        }
    }
    else if (0 == poll_rv)
    {
        errno = ETIME;
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

    auto it = source_to_connection_map_.find(source_id);
    if (it != source_to_connection_map_.end())
    {
        TCPConnection& connection = connections_.at(it->second);

        /* Send message size. */
        msg_size_buf[0] = (uint8_t)(0x00FF & output_packet.message->get_len());
        msg_size_buf[1] = (uint8_t)((0xFF00 & output_packet.message->get_len()) >> 8);
        do
        {
            send_rv = send(connection.poll_fd->fd, msg_size_buf, 2, 0);
            if (0 <= send_rv)
            {
                bytes_sent += send_rv;
            }
            else
            {
                disconnect_client(&connection);
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
                send_rv = send(connection.poll_fd->fd,
                               output_packet.message->get_buf() + bytes_sent,
                               output_packet.message->get_len() - bytes_sent, 0);
                if (0 <= send_rv)
                {
                    bytes_sent += send_rv;
                }
                else
                {
                    disconnect_client(&connection);
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

uint16_t TCPServer::read_data(TCPConnection* connection)
{
    uint16_t rv = 0;
    bool exit_flag = false;

    /* State Machine. */
    while(!exit_flag)
    {
        switch (connection->input_buffer.state)
        {
            case TCP_BUFFER_EMPTY:
            {
                connection->input_buffer.position = 0;
                uint8_t size_buf[2];
                ssize_t bytes_received = recv(connection->poll_fd->fd, size_buf, 2, 0);
                if (0 < bytes_received)
                {
                    connection->input_buffer.msg_size = 0;
                    if (2 == bytes_received)
                    {
                        connection->input_buffer.msg_size = (uint16_t)(size_buf[1] << 8) | (uint16_t)size_buf[0];
                        if (connection->input_buffer.msg_size != 0)
                        {
                            connection->input_buffer.state = TCP_SIZE_READ;
                        }
                        else
                        {
                            connection->input_buffer.state = TCP_BUFFER_EMPTY;
                        }
                    }
                    else
                    {
                        connection->input_buffer.msg_size = (uint16_t)size_buf[0];
                        connection->input_buffer.state = TCP_SIZE_INCOMPLETE;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    disconnect_client(connection);
                    exit_flag = true;
                }
                exit_flag = (0 == poll(connection->poll_fd, 1, 0));
                break;
            }
            case TCP_SIZE_INCOMPLETE:
            {
                uint8_t size_msb;
                ssize_t bytes_received = recv(connection->poll_fd->fd, &size_msb, 1, 0);
                if (0 < bytes_received)
                {
                    connection->input_buffer.msg_size = (uint16_t)(size_msb << 8) | connection->input_buffer.msg_size;
                    if (connection->input_buffer.msg_size != 0)
                    {
                        connection->input_buffer.state = TCP_SIZE_READ;
                    }
                    else
                    {
                        connection->input_buffer.state = TCP_BUFFER_EMPTY;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    disconnect_client(connection);
                    exit_flag = true;
                }
                exit_flag = (0 == poll(connection->poll_fd, 1, 0));
                break;
            }
            case TCP_SIZE_READ:
            {
                connection->input_buffer.buffer.resize(connection->input_buffer.msg_size);
                ssize_t bytes_received = recv(connection->poll_fd->fd,
                                              connection->input_buffer.buffer.data(),
                                              connection->input_buffer.buffer.size(), 0);
                if (0 < bytes_received)
                {
                    if ((uint16_t)bytes_received == connection->input_buffer.msg_size)
                    {
                        connection->input_buffer.state = TCP_MESSAGE_AVAILABLE;
                    }
                    else
                    {
                        connection->input_buffer.position = (uint16_t)bytes_received;
                        connection->input_buffer.state = TCP_MESSAGE_INCOMPLETE;
                        exit_flag = true;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    disconnect_client(connection);
                    exit_flag = true;
                }
                break;
            }
            case TCP_MESSAGE_INCOMPLETE:
            {
                ssize_t bytes_received = recv(connection->poll_fd->fd,
                                              connection->input_buffer.buffer.data() + connection->input_buffer.position,
                                              connection->input_buffer.buffer.size() - connection->input_buffer.position, 0);
                if (0 < bytes_received)
                {
                    connection->input_buffer.position += (uint16_t)bytes_received;
                    if (connection->input_buffer.position == connection->input_buffer.msg_size)
                    {
                        connection->input_buffer.state = TCP_MESSAGE_AVAILABLE;
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
                    disconnect_client(connection);
                    exit_flag = true;
                }
                break;
            }
            case TCP_MESSAGE_AVAILABLE:
            {
                rv = connection->input_buffer.msg_size;
                connection->input_buffer.state = TCP_BUFFER_EMPTY;
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

void TCPServer::disconnect_client(TCPConnection* connection)
{
    ::close(connection->poll_fd->fd);
    if (nullptr != free_connections_)
    {
        if (connection != connection->next)
        {
            /* Remove from connected_clients_. */
            connection->prev->next = connection->next;
            connection->next->prev = connection->prev;
        }
        else
        {
            active_connections_ = nullptr;
        }

        /* Add to free connections. */
        connection->next = free_connections_;
        connection->prev = free_connections_->prev;
        free_connections_->prev->next = connection;
        free_connections_->prev = connection;

        /* Reset last connection read if necessary. */
        if (connection == last_connection_read_)
        {
            last_connection_read_ = active_connections_;
        }
    }
    else
    {
        connection->next = connection;
        connection->prev = connection;
        free_connections_ = connection;
    }
    connection->poll_fd->fd = -1;

    /* Free connection maps. */
    uint64_t source_id = ((uint64_t)connection->addr << 16) | connection->port;
    auto it = source_to_client_map_.find(source_id);
    if (it != source_to_client_map_.end())
    {
        source_to_connection_map_.erase(it->first);
        source_to_client_map_.erase(it->first);
        client_to_source_map_.erase(it->second);
    }
}

void TCPServer::init_input_buffer(TCPInputBuffer* buffer)
{
    buffer->state = TCP_BUFFER_EMPTY;
    buffer->msg_size = 0;
}

} // namespace micrortps
} // namespace eprosima
