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

#include <micrortps/agent/transport/SerialServerLinux.hpp>
#include <unistd.h>

namespace eprosima {
namespace micrortps {

SerialServer::SerialServer(int fd, uint8_t addr)
    : addr_(addr),
      poll_fd_(),
      buffer_{0},
      serial_io_(),
      errno_(0),
      source_to_client_map_{},
      client_to_source_map_{}
{
    poll_fd_.fd = fd;
}

void SerialServer::on_create_client(EndPoint* source, const dds::xrce::ClientKey& client_key)
{
    SerialEndPoint* endpoint = static_cast<SerialEndPoint*>(source);
    uint8_t source_id = endpoint->get_addr();
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

void SerialServer::on_delete_client(EndPoint* source)
{
    SerialEndPoint* endpoint = static_cast<SerialEndPoint*>(source);
    uint8_t source_id = endpoint->get_addr();

    /* Update maps. */
    auto it = source_to_client_map_.find(source_id);
    if (it != source_to_client_map_.end())
    {
        client_to_source_map_.erase(it->second);
        source_to_client_map_.erase(it->first);
    }
}

const dds::xrce::ClientKey SerialServer::get_client_key(EndPoint* source)
{
    dds::xrce::ClientKey client_key;
    SerialEndPoint* endpoint = static_cast<SerialEndPoint*>(source);
    auto it = source_to_client_map_.find(endpoint->get_addr());
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

std::unique_ptr<EndPoint> SerialServer::get_source(const dds::xrce::ClientKey& client_key)
{
    std::unique_ptr<EndPoint> source;
    uint32_t client_id = client_key.at(0) + (client_key.at(1) << 8) + (client_key.at(2) << 16) + (client_key.at(3) <<24);
    auto it = client_to_source_map_.find(client_id);
    if (it != client_to_source_map_.end())
    {
        uint64_t source_id = it->second;
        source.reset(new SerialEndPoint(source_id));
    }
    return source;
}

bool SerialServer::init()
{
    bool rv = false;

    /* Init serial IO. */
    serial_io_.init();

    /* Send init flag. */
    uint8_t flag = MICRORTPS_FRAMING_END_FLAG;
    ssize_t bytes_written = write(poll_fd_.fd, &flag, 1);
    if (0 < bytes_written && 1 == bytes_written)
    {
        /* Poll setup. */
        poll_fd_.events = POLLIN;
        rv = true;
    }
    else
    {
        errno_ = -1;
    }
    return rv;
}

bool SerialServer::close()
{
    return 0 == poll_fd_.fd; //Check this.
}

bool SerialServer::recv_message(InputPacket& input_packet, int timeout)
{
    bool rv = true;
    uint8_t src_addr;
    uint8_t rmt_addr;
    uint16_t bytes_read = serial_io_.read_serial_msg(read_data, this, buffer_, sizeof(buffer_), &src_addr, &rmt_addr, timeout);
    if (0 < bytes_read && rmt_addr == addr_)
    {
        input_packet.message.reset(new InputMessage(buffer_, static_cast<size_t>(bytes_read)));
        input_packet.source.reset(new SerialEndPoint(src_addr));
    }
    else
    {
        errno_ = -1;
        rv = false;
    }
    return rv;
}

bool SerialServer::send_message(OutputPacket output_packet)
{
    bool rv = false;
    const SerialEndPoint* destination = static_cast<const SerialEndPoint*>(output_packet.destination.get());
    uint16_t bytes_written = serial_io_.write_serial_msg(output_packet.message->get_buf(),
                                                         output_packet.message->get_len(),
                                                         addr_,
                                                         destination->get_addr());
    if (0 < bytes_written)
    {
        ssize_t bytes_sent = write(poll_fd_.fd, serial_io_.get_output_buffer(), static_cast<size_t>(bytes_written));
        if (0 < bytes_sent && bytes_sent == bytes_written)
        {
            rv = true;
        }
    }
    errno_ = rv ? 0 : -1;
    return rv;
}

int SerialServer::get_error()
{
    return errno_;
}

uint16_t SerialServer::read_data(void* instance, uint8_t* buf, size_t len, int timeout)
{
    uint16_t rv = 0;
    SerialServer* server = static_cast<SerialServer*>(instance);
    int poll_rv = poll(&server->poll_fd_, 1, timeout);
    if (0 < poll_rv)
    {
        ssize_t bytes_read = read(server->poll_fd_.fd, buf, len);
        if (0 < bytes_read)
        {
            rv = static_cast<uint16_t>(bytes_read);
        }
    }
    return rv;
}

} // namespace micrortps
} // namespace eprosima
