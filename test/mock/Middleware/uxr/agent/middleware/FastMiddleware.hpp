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


#ifndef _UXR_AGENT_MIDDLEWARE_FAST_MIDDLEWARE_HPP_
#define _UXR_AGENT_MIDDLEWARE_FAST_MIDDLEWARE_HPP_

#include <uxr/agent/middleware/Middleware.hpp>

#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <memory>

namespace eprosima {
namespace uxr {

class FastMiddleware : public Middleware
{
public:
    FastMiddleware() = default;
    ~FastMiddleware() = default;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_MIDDLEWARE_FAST_MIDDLEWARE_HPP_
