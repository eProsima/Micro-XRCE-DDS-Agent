#include <agent/transport/UDPServer.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

namespace eprosima {
namespace micrortps {

bool UDPServer::send_msg(const uint8_t* buf, const size_t len, TransportClient* client)
{
    bool rv = true;
    UDPClient* udp_client = static_cast<UDPClient*>(client);
    struct sockaddr_in client_addr;

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = udp_client->port_;
    client_addr.sin_addr.s_addr = udp_client->addr_;
    ssize_t bytes_sent = sendto(poll_fd_.fd, buf, len, 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
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

bool UDPServer::recv_msg(uint8_t** buf, size_t* len, int timeout, TransportClient** client)
{
    bool rv = true;
    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int poll_rv = poll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_received = recvfrom(poll_fd_.fd, buffer_, sizeof(buffer_), 0, &client_addr, &client_addr_len);
        if (0 < bytes_received)
        {
            *len = (size_t)bytes_received;
            *buf = buffer_;
            client_.addr_ = ((struct sockaddr_in*)&client_addr)->sin_addr.s_addr;
            client_.port_ = ((struct sockaddr_in*)&client_addr)->sin_port;
            *client = &client_;
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

int UDPServer::get_error()
{
    return errno;
}

int UDPServer::launch(uint16_t port)
{
    int rv = 0;

    /* Socker initialization. */
    poll_fd_.fd = socket(PF_INET, SOCK_DGRAM, 0);

    if (-1 == poll_fd_.fd)
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
        if (-1 == bind(poll_fd_.fd, (struct sockaddr*)&address, sizeof(address)))
        {
            rv = errno;
        }
        else
        {
            /* Poll setup. */
            poll_fd_.events = POLLIN;
        }
    }

    return rv;
}

} // namespace micrortps
} // namespace eprosima
