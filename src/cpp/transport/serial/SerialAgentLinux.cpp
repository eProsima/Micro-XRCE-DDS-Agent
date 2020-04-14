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

#include <uxr/agent/transport/serial/SerialAgentLinux.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <unistd.h>

namespace eprosima {
namespace uxr {

SerialAgent::SerialAgent(
        uint8_t addr,
        Middleware::Kind middleware_kind)
    : Server<SerialEndPoint>{middleware_kind}
    , addr_{addr}
    , poll_fd_{}
    , buffer_{0}
    , serial_io_(
          addr,
          std::bind(&SerialAgent::write_data, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
          std::bind(&SerialAgent::read_data, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4))
{}

size_t SerialAgent::write_data(
        uint8_t* buf,
        size_t len,
        TransportRc& transport_rc)
{
    size_t rv = 0;
    ssize_t bytes_written = ::write(poll_fd_.fd, buf, len);
    if ((0 < bytes_written)  && size_t(bytes_written) == len)
    {
        rv = size_t(bytes_written);
    }
    else
    {
        transport_rc = TransportRc::server_error;
    }
    return rv;
}

size_t SerialAgent::read_data(
        uint8_t* buf,
        size_t len,
        int timeout,
        TransportRc& transport_rc)
{
    size_t rv = 0;
    int poll_rv = poll(&poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_read = read(poll_fd_.fd, buf, len);
        if (0 < bytes_read)
        {
            rv = size_t(bytes_read);
        }
        else
        {
            transport_rc = TransportRc::server_error;
        }
    }
    else
    {
        transport_rc = (poll_rv == 0) ? TransportRc::timeout_error : TransportRc::server_error;
    }
    return rv;
}

bool SerialAgent::recv_message(
        InputPacket<SerialEndPoint>& input_packet,
        int timeout,
        TransportRc& transport_rc)
{
    bool rv = false;
    uint8_t remote_addr;
    size_t bytes_read = serial_io_.read_msg(buffer_,sizeof (buffer_), remote_addr, timeout, transport_rc);
    if (0 < bytes_read)
    {
        input_packet.message.reset(new InputMessage(buffer_, static_cast<size_t>(bytes_read)));
        input_packet.source = SerialEndPoint(remote_addr);
        rv = true;

        uint32_t raw_client_key;
        if (Server<SerialEndPoint>::get_client_key(input_packet.source, raw_client_key))
        {
            UXR_AGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[==>> SER <<==]"),
                raw_client_key,
                input_packet.message->get_buf(),
                input_packet.message->get_len());
        }
    }
    return rv;
}

bool SerialAgent::send_message(
        OutputPacket<SerialEndPoint> output_packet,
        TransportRc& transport_rc)
{
    bool rv = false;
    size_t bytes_written =
            serial_io_.write_msg(
                output_packet.message->get_buf(),
                output_packet.message->get_len(),
                output_packet.destination.get_addr(),
                transport_rc);
    if ((0 < bytes_written) && (bytes_written == output_packet.message->get_len()))
    {
        rv = true;

        uint32_t raw_client_key;
        if (Server<SerialEndPoint>::get_client_key(output_packet.destination, raw_client_key))
        {
            UXR_AGENT_LOG_MESSAGE(
                UXR_DECORATE_YELLOW("[** <<SER>> **]"),
                raw_client_key,
                output_packet.message->get_buf(),
                output_packet.message->get_len());
        }
    }
    return rv;
}

} // namespace uxr
} // namespace eprosima
