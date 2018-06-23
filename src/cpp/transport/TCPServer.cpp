#include <agent/transport/TCPServer.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

namespace eprosima {
namespace micrortps {

TCPServer* TCPServer::create(uint16_t port)
{
    TCPServer* server = new TCPServer();
    return (0 == server->init(port)) ? server : nullptr;
}

bool TCPServer::send_data(uint32_t addr, uint16_t port, const uint8_t* buf, const size_t len)
{
    bool rv = true;
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = port;
    client_addr.sin_addr.s_addr = addr;
    ssize_t bytes_sent = sendto(listen_fd_, buf, len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
    if (0 < bytes_sent)
    {
        if ((size_t)bytes_sent != len)
        {
            rv = false;
        }
    }
    else
    {
        rv = false;
    }

    return rv;
}

bool TCPServer::recv_data(uint32_t* addr, uint16_t* port, uint8_t** buf, size_t* len, int timeout)
{
    bool rv = true;
    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int poll_rv = poll(poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_received = recvfrom(listen_fd_, buffer_, sizeof(buffer_), 0, &client_addr, &client_addr_len);
        if (0 < bytes_received)
        {
            *len = (size_t)bytes_received;
            *buf = buffer_;
            *addr = ((struct sockaddr_in*)&client_addr)->sin_addr.s_addr;
            *port = ((struct sockaddr_in*)&client_addr)->sin_port;
        }
    }
    else if (0 == poll_rv)
    {
        errno = ETIME;
        rv = false;
    }
    else
    {
        rv = false;
    }

    return rv;
}

int TCPServer::get_error()
{
    return errno;
}

int TCPServer::init(uint16_t port)
{
    int rv = 0;

    /* Socker initialization. */
    listen_fd_ = socket(PF_INET, SOCK_DGRAM, 0);

    if (-1 == listen_fd_)
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
        if (-1 == bind(listen_fd_, (struct sockaddr*)&address, sizeof(address)))
        {
            rv = errno;
        }
        else
        {
            /* Poll setup. */
            poll_fd_[0].fd = listen_fd_;
            poll_fd_[0].events = POLLIN;
        }
    }

    return rv;
}

} // namespace micrortps
} // namespace eprosima
