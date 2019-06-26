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

#ifndef UXR_AGENT_MESSAGE_PACKET_HPP_
#define UXR_AGENT_MESSAGE_PACKET_HPP_

#include <uxr/agent/message/InputMessage.hpp>
#include <uxr/agent/message/OutputMessage.hpp>
#include <memory>

namespace eprosima {
namespace uxr {

typedef std::unique_ptr<InputMessage> InputMessagePtr;

template<typename EndPoint>
struct InputPacket
{
    EndPoint source;
    InputMessagePtr message;
};

typedef std::shared_ptr<OutputMessage> OutputMessagePtr;

template<typename EndPoint>
struct OutputPacket
{
    EndPoint destination;
    OutputMessagePtr message;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_MESSAGE_PACKET_HPP_
