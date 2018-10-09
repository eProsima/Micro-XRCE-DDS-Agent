// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _UXR_UTILS_DATAREADER_TOKENBUCKET_HPP_
#define _UXR_UTILS_DATAREADER_TOKENBUCKET_HPP_

#include <chrono>
#include <mutex>

namespace eprosima {
namespace uxr {
namespace utils {

class TokenBucket
{
public:
    explicit TokenBucket(size_t rate, size_t burst = min_rate_);
    TokenBucket(TokenBucket&& other) noexcept;
    TokenBucket(const TokenBucket& other);
    TokenBucket& operator=(TokenBucket&& other) noexcept;
    TokenBucket& operator=(const TokenBucket& other);
    ~TokenBucket()       = default;

    bool get_tokens(size_t tokens);

private:
    size_t available_tokens();

private:
    static constexpr size_t min_rate_ = 64000; // 64KB
    size_t capacity_;
    size_t tokens_;
    size_t rate_;
    std::chrono::steady_clock::time_point timestamp_;

    std::mutex data_mutex_;
};
} // namespace utils
} // namespace uxr
} // namespace eprosima

#endif //_UXR_UTILS_DATAREADER_TOKENBUCKET_HPP_
