#include <agent/transport/TCPServer.hpp>
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

bool TCPServer::send_msg(const uint8_t* buf, const size_t len, TransportClient* client)
{
    bool rv = true;
    uint16_t bytes_sent = 0;
    ssize_t send_rv = 0;
    uint8_t msg_size_buf[2]; 
    TCPClient* tcp_client = static_cast<TCPClient*>(client);

    /* Send message size. */
    msg_size_buf[0] = (uint8_t)(0x00FF & len);
    msg_size_buf[1] = (uint8_t)((0xFF00 & len) >> 8);
    do
    {
        send_rv = send(tcp_client->poll_fd->fd, msg_size_buf, 2, 0);
        if (0 <= send_rv)
        {
            bytes_sent += send_rv;
        }
        else
        {
            disconnect_client(tcp_client);
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
            send_rv = send(tcp_client->poll_fd->fd, buf + bytes_sent, len - bytes_sent, 0);
            if (0 <= send_rv)
            {
                bytes_sent += send_rv;
            }
            else
            {
                disconnect_client(tcp_client);
                rv = false;
            }
        }
        while (rv && bytes_sent != (uint16_t)len);
    }

    return rv;
}

bool TCPServer::recv_msg(uint8_t** buf, size_t* len, int timeout, TransportClient** client)
{
    bool rv = false;

    int poll_rv = poll(poll_fds_.data(), poll_fds_.size(), timeout);
    if (0 < poll_rv)
    {
        if (POLLIN == (POLLIN & poll_fds_[0].revents) && nullptr != available_clients_)
        {
            /* New client connection. */
            struct sockaddr client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int incoming_fd = accept(poll_fds_[0].fd, &client_addr, &client_addr_len);
            if (-1 != incoming_fd)
            {
                /* Update available clients list. */
                TCPClient* incoming_client = available_clients_;
                incoming_client->poll_fd->fd = incoming_fd;
                if (available_clients_ != available_clients_->next)
                {
                    available_clients_->prev->next = available_clients_->next;
                    available_clients_->next->prev = available_clients_->prev;
                    available_clients_ = available_clients_->next;
                }
                else
                {
                    available_clients_ = nullptr;
                }

                /* Update connected client list. */
                if (nullptr != connected_clients_)
                {
                    incoming_client->prev = connected_clients_->prev;
                    incoming_client->next = connected_clients_;
                    connected_clients_->prev->next = incoming_client;
                    connected_clients_->prev = incoming_client;
                }
                else
                {
                    incoming_client->prev = incoming_client;
                    incoming_client->next = incoming_client;
                    connected_clients_ = incoming_client;
                    last_client_read_ = incoming_client;
                }
            }
        }
        else
        {
            if (nullptr != connected_clients_)
            {
                /**
                 * Receive Scheduler: the first client in attemp to read will be the neighbour of
                 *                    the last client form which data was read.
                 */
                TCPClient* base_client = last_client_read_->next;
                TCPClient* reader = last_client_read_->next;
                do
                {
                    if (POLLIN == (POLLIN & reader->poll_fd->revents))
                    {
                        uint16_t bytes_read = read_data(reader);
                        if (0 < bytes_read)
                        {
                            *len = bytes_read;
                            *buf = reader->input_buffer.buffer.data();
                            last_client_read_ = reader;
                            *client = reader;
                            rv = true;
                            break;
                        }
                    }
                    reader = reader->next;

                } 
                while (reader != base_client);
            }
        }
    }
    else if (0 == poll_rv)
    {
        errno = ETIME;
    }

    return rv;
}

int TCPServer::get_error()
{
    return errno;
}

int TCPServer::launch(uint16_t port)
{
    int rv = 0;

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
        address.sin_port = htons(port);
        address.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(address.sin_zero, '\0', sizeof(address.sin_zero));
        if (-1 == bind(poll_fds_[0].fd, (struct sockaddr*)&address, sizeof(address)))
        {
            rv = errno;
        }
        else
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
            if (-1 == listen(poll_fds_[0].fd, MICRORTPS_MAX_BACKLOG_TCP_CONNECTIONS))
            {
                rv = errno;
            }
            else
            {
                /* Client setup. */
                clients_[0].poll_fd = &poll_fds_[1];
                init_input_buffer(&clients_[0].input_buffer);
                for (unsigned int i = 1; i < poll_fds_.size() - 1; ++i)
                {
                    clients_[i].poll_fd = &poll_fds_[i + 1];
                    init_input_buffer(&clients_[i].input_buffer);
                    clients_[i - 1].next = &clients_[i];
                    clients_[i].prev = &clients_[i - 1];
                }
                clients_.back().next = &clients_.front();
                clients_.front().prev = &clients_.back();
                available_clients_ = &clients_[0];
                last_client_read_ = available_clients_;
            }
        }
    }

    return rv;
}

uint16_t TCPServer::read_data(TCPClient* client)
{
    uint16_t rv = 0;
    bool exit_flag = false;

    /* State Machine. */
    while(!exit_flag)
    {
        switch (client->input_buffer.state)
        {
            case TCP_BUFFER_EMPTY:
            {
                client->input_buffer.position = 0;
                uint8_t size_buf[2];
                ssize_t bytes_received = recv(client->poll_fd->fd, size_buf, 2, 0);
                if (0 < bytes_received)
                {
                    client->input_buffer.msg_size = 0;
                    if (2 == bytes_received)
                    {
                        client->input_buffer.msg_size = (uint16_t)(size_buf[1] << 8) | (uint16_t)size_buf[0];
                        if (client->input_buffer.msg_size != 0)
                        {
                            client->input_buffer.state = TCP_SIZE_READ;
                        }
                        else
                        {
                            client->input_buffer.state = TCP_BUFFER_EMPTY;
                        }
                    }
                    else
                    {
                        client->input_buffer.msg_size = (uint16_t)size_buf[0];
                        client->input_buffer.state = TCP_SIZE_INCOMPLETE;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    disconnect_client(client);
                    exit_flag = true;
                }
                exit_flag = (0 == poll(client->poll_fd, 1, 0));
                break;
            }
            case TCP_SIZE_INCOMPLETE:
            {
                uint8_t size_msb;
                ssize_t bytes_received = recv(client->poll_fd->fd, &size_msb, 1, 0);
                if (0 < bytes_received)
                {
                    client->input_buffer.msg_size = (uint16_t)(size_msb << 8) | client->input_buffer.msg_size;
                    if (client->input_buffer.msg_size != 0)
                    {
                        client->input_buffer.state = TCP_SIZE_READ;
                    }
                    else
                    {
                        client->input_buffer.state = TCP_BUFFER_EMPTY;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    disconnect_client(client);
                    exit_flag = true;
                }
                exit_flag = (0 == poll(client->poll_fd, 1, 0));
                break;
            }
            case TCP_SIZE_READ:
            {
                client->input_buffer.buffer.resize(client->input_buffer.msg_size);
                ssize_t bytes_received = recv(client->poll_fd->fd, 
                                              client->input_buffer.buffer.data(), 
                                              client->input_buffer.buffer.size(), 0);
                if (0 < bytes_received)
                {
                    if ((uint16_t)bytes_received == client->input_buffer.msg_size)
                    {
                        client->input_buffer.state = TCP_MESSAGE_AVAILABLE;
                    }
                    else
                    {
                        client->input_buffer.position = (uint16_t)bytes_received;
                        client->input_buffer.state = TCP_MESSAGE_INCOMPLETE;        
                        exit_flag = true;
                    }
                }
                else
                {
                    if (0 == bytes_received)
                    {
                        errno = ENOTCONN;
                    }
                    disconnect_client(client);
                    exit_flag = true;
                }
                break;
            }
            case TCP_MESSAGE_INCOMPLETE:
            {
                ssize_t bytes_received = recv(client->poll_fd->fd, 
                                              client->input_buffer.buffer.data() + client->input_buffer.position, 
                                              client->input_buffer.buffer.size() - client->input_buffer.position, 0);
                if (0 < bytes_received)
                {
                    client->input_buffer.position += (uint16_t)bytes_received;
                    if (client->input_buffer.position == client->input_buffer.msg_size)
                    {
                        client->input_buffer.state = TCP_MESSAGE_AVAILABLE;
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
                    disconnect_client(client);
                    exit_flag = true;
                }
                break;
            }
            case TCP_MESSAGE_AVAILABLE:
            {
                rv = client->input_buffer.msg_size;
                client->input_buffer.state = TCP_BUFFER_EMPTY;
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

void TCPServer::disconnect_client(TCPClient* client)
{
    close(client->poll_fd->fd);
    if (nullptr != available_clients_)
    {
        if (client != client->next)
        {
            /* Remove from connected_clients_. */
            client->prev->next = client->next;
            client->next->prev = client->prev;
        }
        else
        {
            connected_clients_ = nullptr;
        }

        /* Add to available_clients_. */
        client->next = available_clients_;
        client->prev = available_clients_->prev;
        available_clients_->prev->next = client;
        available_clients_->prev = client;

        /* Reset last_client_read_ if necessary. */
        if (client == last_client_read_)
        {
            last_client_read_ = connected_clients_;
        }
    }
    else
    {
        client->next = client;
        client->prev = client;
        available_clients_ = client;
    }
    client->poll_fd->fd = -1;
}

void TCPServer::init_input_buffer(TCPInputBuffer* buffer)
{
    buffer->state = TCP_BUFFER_EMPTY;
    buffer->msg_size = 0;
}

} // namespace micrortps
} // namespace eprosima
