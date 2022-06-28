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

#include <uxr/agent/transport/serial/MultiSerialAgentLinux.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <unistd.h>

namespace eprosima {
namespace uxr {

MultiSerialAgent::MultiSerialAgent(
        uint8_t addr,
        Middleware::Kind middleware_kind)
    : Server<MultiSerialEndPoint>{middleware_kind}
    , framing_io{}
    , addr_{addr}
    , buffer_{0}
{
    FD_ZERO(&read_fds);
}

void MultiSerialAgent::insert_serial(int serial_fd)
{
    utils::ExclusiveLockPriority lk(framing_mtx);
    FD_SET(serial_fd, &read_fds);
    FramingIO aux_framing_io(addr_,
        std::bind(&MultiSerialAgent::write_data, this, (uint8_t) serial_fd, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        std::bind(&MultiSerialAgent::read_data, this, (uint8_t) serial_fd, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    
    framing_io.insert(std::pair<int, FramingIO>(serial_fd, aux_framing_io));
}

bool MultiSerialAgent::remove_serial(int serial_fd)
{
    bool rv = false;
    utils::ExclusiveLockPriority lk(framing_mtx);

    FD_CLR(serial_fd, &read_fds);
    framing_io.erase(serial_fd);

    if (0 == ::close(serial_fd))
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("serial port closed"),
            "fd: {}",
            serial_fd);
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("close serial error"),
            "fd: {}, errno: {}",
            serial_fd, errno);
    }

    return rv;
}

bool MultiSerialAgent::recv_message(
        std::vector<InputPacket<MultiSerialEndPoint>>& input_packet,
        int timeout,
        TransportRc& transport_rc)
{
    struct timeval timeout_;
    timeout_.tv_sec = timeout / 1000;
    timeout_.tv_usec = (timeout % 1000) * 1000;

    bool rv = false;

    utils::SharedLockPriority lk(framing_mtx);
    fd_set fds = read_fds;
    int ret = select(framing_io.rbegin()->first+1, &fds, NULL, NULL, &timeout_);

    if (0 < ret)
    {
        for (auto it = framing_io.begin(); it != framing_io.end(); it++)
        {
            if (FD_ISSET(it->first, &fds))
            {
                uint8_t remote_addr = 0x00;
                ssize_t bytes_read = 0;
                int timeout_ms = timeout_.tv_sec*1000 + timeout_.tv_usec/1000;

                do
                {
                    bytes_read = it->second.read_framed_msg(
                        buffer_, sizeof (buffer_), remote_addr, timeout_ms, transport_rc);
                }
                while ((0 == bytes_read) && (0 < timeout_ms));

                if (0 < bytes_read)
                {
                    struct InputPacket<MultiSerialEndPoint> aux_pack{};
                    aux_pack.message.reset(new InputMessage(buffer_, static_cast<size_t>(bytes_read)));
                    aux_pack.source = MultiSerialEndPoint(it->first, remote_addr);
                    rv = true;

                    uint32_t raw_client_key;
                    if (Server<MultiSerialEndPoint>::get_client_key(aux_pack.source, raw_client_key))
                    {
                        UXR_MULTIAGENT_LOG_MESSAGE(
                            UXR_DECORATE_YELLOW("[==>> SER <<==]"),
                            raw_client_key,
                            it->first,
                            aux_pack.message->get_buf(),
                            aux_pack.message->get_len());
                    }

                    input_packet.push_back(std::move(aux_pack));
                }
            }
        }
    }
    else
    {
        transport_rc = (ret == 0) ? TransportRc::timeout_error : TransportRc::server_error;
    }

    return rv;
}

bool MultiSerialAgent::send_message(
        OutputPacket<MultiSerialEndPoint> output_packet,
        TransportRc& transport_rc)
{
    bool rv = false;
    int client_fd = output_packet.destination.get_fd();

    utils::SharedLockPriority lk(framing_mtx);
    std::map<int, FramingIO>::iterator it = framing_io.find(client_fd);

    if (it == framing_io.end())
    {
        // Destination client not found on active ports
        return rv;
    }

    ssize_t bytes_written =
            it->second.write_framed_msg(
                output_packet.message->get_buf(),
                output_packet.message->get_len(),
                output_packet.destination.get_addr(),
                transport_rc);

    if ((0 < bytes_written) && (
         static_cast<size_t>(bytes_written) == output_packet.message->get_len()))
    {
        rv = true;

        uint32_t raw_client_key;
        if (Server<MultiSerialEndPoint>::get_client_key(output_packet.destination, raw_client_key))
        {
            UXR_MULTIAGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[** <<SER>> **]"),
                raw_client_key,
                output_packet.destination.get_fd(),
                output_packet.message->get_buf(),
                output_packet.message->get_len());
        }
    }
    return rv;
}

ssize_t MultiSerialAgent::read_data(
        uint8_t serial_fd,
        uint8_t* buf,
        size_t len,
        int timeout,
        TransportRc& transport_rc)
{
    // Modify timeout?
    ssize_t bytes_read = 0;
    pollfd read_file = {serial_fd, POLLIN, 0};

    int poll_rv = poll(&read_file, 1, timeout);

    if(read_file.revents & (POLLERR+POLLHUP))
    {
        transport_rc = TransportRc::server_error;;
    }
    else if (0 < poll_rv)
    {
        bytes_read = read(serial_fd, buf, len);
        if (0 > bytes_read)
        {
            transport_rc = TransportRc::server_error;
        }
    }
    else
    {
        transport_rc = (poll_rv == 0) ? TransportRc::timeout_error : TransportRc::server_error;
    }

    if (transport_rc == TransportRc::server_error)
    {
        std::unique_lock<std::mutex> lk(error_mtx);
        error_fd.push_back(serial_fd);
    }

    return bytes_read;
}

ssize_t MultiSerialAgent::write_data(
        uint8_t serial_fd,
        uint8_t* buf,
        size_t len,
        TransportRc& transport_rc)
{
    size_t rv = 0;
    ssize_t bytes_written = ::write(serial_fd, buf, len);
    if (0 < bytes_written)
    {
        rv = size_t(bytes_written);
    }
    else
    {
        transport_rc = TransportRc::server_error;
        std::unique_lock<std::mutex> lk(error_mtx);
        error_fd.push_back(serial_fd);
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
