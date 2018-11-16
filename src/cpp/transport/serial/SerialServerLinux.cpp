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

#include <uxr/agent/transport/serial/SerialServerLinux.hpp>
#include <unistd.h>

namespace eprosima {
namespace uxr {

SerialServer::SerialServer(int fd, uint8_t addr)
    : SerialServerBase(addr),
      poll_fd_(),
      buffer_{0},
      serial_io_(),
      errno_(0)
{
    poll_fd_.fd = fd;
}

bool SerialServer::init(bool discovery_enabled)
{
    (void) discovery_enabled;

    /* Init serial IO. */
    uxr_init_serial_io(&serial_io_, addr_);

    /* Poll setup. */
    poll_fd_.events = POLLIN;

    return true;
}

bool SerialServer::close()
{
    return 0 == ::close(poll_fd_.fd);
}

size_t SerialServer::write_data(void* instance, uint8_t* buf, size_t len)
{
    size_t rv = 0;
    SerialServer* server = static_cast<SerialServer*>(instance);
    ssize_t bytes_written = write(server->poll_fd_.fd, (void*)buf, len);
    if ((0 < bytes_written)  && size_t(bytes_written) == len)
    {
        rv = size_t(bytes_written);
    }
    return rv;
}

size_t SerialServer::read_data(void* instance, uint8_t* buf, size_t len, int timeout)
{
    size_t rv = 0;
    SerialServer* server = static_cast<SerialServer*>(instance);
    int poll_rv = poll(&server->poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_read = read(server->poll_fd_.fd, buf, len);
        if (0 < bytes_read)
        {
            rv = size_t(bytes_read);
        }
    }
    return rv;
}

bool SerialServer::recv_message(InputPacket& input_packet, int timeout)
{
    bool rv = false;
    uint8_t remote_addr;
    size_t bytes_read = uxr_read_serial_msg(&serial_io_,
                                            read_data,
                                            this,
                                            buffer_,
                                            sizeof(buffer_),
                                            &remote_addr,
                                            timeout);
    if (0 < bytes_read)
    {
        input_packet.message.reset(new InputMessage(buffer_, static_cast<size_t>(bytes_read)));
        input_packet.source.reset(new SerialEndPoint(remote_addr));
        rv = true;
    }
    else
    {
        errno_ = -1;
    }
    return rv;
}

bool SerialServer::send_message(OutputPacket output_packet)
{
    bool rv = false;
    const SerialEndPoint* destination = static_cast<const SerialEndPoint*>(output_packet.destination.get());
    size_t bytes_written = uxr_write_serial_msg(&serial_io_,
                                                write_data,
                                                this,
                                                output_packet.message->get_buf(),
                                                output_packet.message->get_len(),
                                                destination->get_addr());
    if ((0 < bytes_written) && (bytes_written == output_packet.message->get_len()))
    {
        rv = true;
    }
    errno_ = rv ? 0 : -1;
    return rv;
}

int SerialServer::get_error()
{
    return errno_;
}

} // namespace uxr
} // namespace eprosima
