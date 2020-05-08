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

#ifndef UXR_AGENT_TRANSPORT_TCP_CONNECTION_HPP_
#define UXR_AGENT_TRANSPORT_TCP_CONNECTION_HPP_

#include <uxr/agent/transport/endpoint/IPv4EndPoint.hpp>
#include <uxr/agent/transport/endpoint/IPv6EndPoint.hpp>

#include <stdint.h>
#include <vector>
#include <mutex>

namespace eprosima {
namespace uxr {

typedef enum TCPInputBufferState
{
    TCP_BUFFER_EMPTY,
    TCP_SIZE_INCOMPLETE,
    TCP_SIZE_READ,
    TCP_MESSAGE_INCOMPLETE,
    TCP_MESSAGE_AVAILABLE

} TCPInputBufferState;

struct TCPInputBuffer
{
    std::vector<uint8_t> buffer;
    uint16_t position;
    TCPInputBufferState state;
    uint16_t msg_size;
};

struct TCPConnection
{
    TCPInputBuffer input_buffer;
    uint32_t id;
    bool active;
    std::mutex mtx;
};

struct TCPv4Connection : public TCPConnection
{
    IPv4EndPoint endpoint;
};

struct TCPv6Connection : public TCPConnection
{
    IPv6EndPoint endpoint;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_TCP_CONNECTION_HPP_
