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

#ifndef _UXR_AGENT_TYPES_OPTIONAL_HPP_
#define _UXR_AGENT_TYPES_OPTIONAL_HPP_

#include <utility>

namespace eprosima {

template <typename T>
class Optional
{
  public:
    Optional() = default;

    Optional(const T& value)
    {
        present_ = true;
        data_    = value;
    }

    Optional(T&& value)
    {
        present_ = true;
        data_    = std::move(value);
    }

    Optional& operator=(const T& value)
    {
        present_ = true;
        data_    = value;
        return *this;
    }

    Optional& operator=(T&& value)
    {
        present_ = true;
        data_    = std::move(value);
        return *this;
    }

    const T* operator->() const
    {
        return present_ ? &data_ : nullptr;
    }

    T* operator->()
    {
        return present_ ? &data_ : nullptr;
    }

    const T& operator*() const &
    {
        return data_;
    }

    T& operator*() &
    {
        return data_;
    }

    const T&& operator*() const &&
    {
        return std::move(data_);
    }

    T&& operator*() &&
    {
        return std::move(data_);
    }

    explicit operator bool() const noexcept
    {
        return present_;
    }

  private:
    bool present_ = false;
    T data_;
};

} // namespace eprosima

#endif //_UXR_AGENT_TYPES_OPTIONAL_HPP_
