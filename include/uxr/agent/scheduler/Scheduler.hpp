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

#ifndef _UXR_AGENT_SCHEDULER_SCHEDULER_HPP_
#define _UXR_AGENT_SCHEDULER_SCHEDULER_HPP_

#include <cstdint>

namespace eprosima {
namespace uxr {

template<class T>
class Scheduler
{
public:
    Scheduler() = default;
    virtual ~Scheduler() {}

    virtual void init() = 0;
    virtual void deinit() = 0;
    virtual void push(T&& element, uint8_t priority) = 0;
    virtual bool pop(T& element) = 0;
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_SCHEDULER_SCHEDULER_HPP_
