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

#ifndef _UXR_AGENT_UTILS_CALLBACKS_HPP_
#define _UXR_AGENT_UTILS_CALLBACKS_HPP_

#include <uxr/agent/types/XRCETypes.hpp>
#include <vector>
#include <functional>

namespace eprosima {
namespace uxr {

using onCreateCallback = std::function<void(const dds::xrce::ObjectKind&, const dds::GUID_t&)>;
using onCreateCallbackVector = std::vector<onCreateCallback>;

using onDeleteCallback = std::function<void(const dds::xrce::ObjectKind, const dds::GUID_t)>;
using onDeleteCallbackVector = std::vector<onDeleteCallback>;

}
}

#endif // !_UXR_AGENT_UTILS_CALLBACKS_HPP_
