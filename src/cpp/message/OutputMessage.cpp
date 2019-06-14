// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/message/OutputMessage.hpp>
#include <uxr/agent/logger/Logger.hpp>

namespace eprosima {
namespace uxr {

void OutputMessage::log_error()
{
    UXR_AGENT_LOG_ERROR(
        UXR_DECORATE_RED("serialization error"),
        "buffer: {:X}",
        UXR_AGENT_LOG_TO_HEX(buf_, buf_ + len_));
}

} // namespace uxr
} // namespace eprosima
