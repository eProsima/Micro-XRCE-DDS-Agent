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

#ifndef _MICRORTPS_AGENT_TRANSPORT_UART_SERVER_HPP_
#define _MICRORTPS_AGENT_TRANSPORT_UART_SERVER_HPP_

#include <micrortps/agent/transport/Server.hpp>
#include <micrortps/agent/transport/SerialLayer.hpp>
#include <unordered_map>
#include <cstdint>
#include <cstddef>
#include <sys/poll.h>

namespace eprosima {
namespace micrortps {

/**************************************************************************************************
 * UART EndPoint.
 **************************************************************************************************/
class UARTEndPoint : public EndPoint
{
public:
    UARTEndPoint(uint8_t addr) { addr_ = addr; }
    ~UARTEndPoint() {}

    uint8_t get_addr() const { return addr_; }

public:
    uint8_t addr_;
};

/**************************************************************************************************
 * UART Server.
 **************************************************************************************************/
class UARTServer : public Server
{
public:
    UARTServer(int fd, uint8_t addr);
    ~UARTServer() = default;

    virtual void on_create_client(EndPoint* source, const dds::xrce::ClientKey& client_key) override;
    virtual void on_delete_client(EndPoint* source) override;
    virtual const dds::xrce::ClientKey get_client_key(EndPoint* source) override;
    virtual std::unique_ptr<EndPoint> get_source(const dds::xrce::ClientKey& client_key) override;

private:
    virtual bool init() override;
    virtual bool recv_message(InputPacket& input_packet, int timeout) override;
    virtual bool send_message(OutputPacket output_packet) override;
    virtual int get_error() override;
    static uint16_t read_data(void* instance, uint8_t* buf, size_t len, int timeout);

private:
    uint8_t addr_;
    struct pollfd poll_fd_;
    uint8_t buffer_[MICRORTPS_SERIAL_MTU];
    SerialIO serial_io_;
    int errno_;
    std::unordered_map<uint8_t, uint32_t> source_to_client_map_;
    std::unordered_map<uint32_t, uint8_t> client_to_source_map_;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_UART_SERVER_HPP_
