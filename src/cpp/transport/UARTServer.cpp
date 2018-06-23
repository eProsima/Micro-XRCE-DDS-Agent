#include <agent/transport/UARTServer.hpp>
#include <unistd.h>

namespace eprosima {
namespace micrortps {

UARTServer* UARTServer::create(int fd, uint8_t addr)
{
    UARTServer* server = new UARTServer();
    return (0 == server->init(fd, addr)) ? server : nullptr;
}

bool UARTServer::send_msg(const uint8_t* buf, const size_t len, uint8_t addr)
{
    bool rv = false;

    uint16_t bytes_written = write_serial_msg(&serial_io_, buf, len, addr);
    if (0 < bytes_written)
    {
        ssize_t bytes_sent = write(fd_, serial_io_.output.buffer, size_t(bytes_written));
        if (0 < bytes_sent && bytes_sent == bytes_written)
        {
            rv = true;
        }
    }
    errno_ = rv ? 0 : -1;

    return rv;
}

bool UARTServer::recv_msg(uint8_t** buf, size_t* len, uint8_t* addr, int timeout)
{
    bool rv = true;

    uint8_t bytes_read = read_serial_msg(&serial_io_, read_data, this, buffer_, sizeof(buffer_), addr, timeout);
    if (0 < bytes_read)
    {
        *len = bytes_read;
        *buf = buffer_;
    }
    else
    {
        rv = false;
        errno_ = -1;
    }

    return rv;
}

int UARTServer::get_error()
{
    return errno_;
}

int UARTServer::init(int fd, uint8_t addr)
{
    int rv = 0;

    /* File descriptor setup. */
    fd_ = fd;
    addr_ = addr;

    /* Init SerialIO. */
    init_serial_io(&serial_io_);

    /* Send init flag. */
    uint8_t flag = MICRORTPS_FRAMING_END_FLAG;
    ssize_t bytes_written = write(fd_, &flag, 1);
    if (0 < bytes_written && 1 == bytes_written)
    {
        /* Poll setup. */
        poll_fd_.fd = fd_;
        poll_fd_.events = POLLIN;
    }
    else
    {
        rv = -1;
    }

    return rv;
}

uint16_t UARTServer::read_data(void* instance, uint8_t* buf, size_t len, int timeout)
{
    uint16_t rv = 0;
    UARTServer* server = static_cast<UARTServer*>(instance);

    int poll_rv = poll(&server->poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_read = read(server->fd_, buf, len);
        if (0 < bytes_read)
        {
            rv = static_cast<uint16_t>(bytes_read);
        }
    }

    return rv;
}

} // namespace micrortps
} // namespace eprosima
