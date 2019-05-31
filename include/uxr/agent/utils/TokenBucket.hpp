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

#ifndef UXR_UTILS_DATAREADER_TOKENBUCKET_HPP_
#define UXR_UTILS_DATAREADER_TOKENBUCKET_HPP_

#include <chrono>
#include <mutex>
#include <algorithm>

namespace eprosima {
namespace uxr {
namespace utils {

class TokenBucket
{
public:
    explicit TokenBucket(
            size_t rate,
            size_t burst = min_rate_);

    TokenBucket(TokenBucket&&) = delete;
    TokenBucket(const TokenBucket&) = delete;
    TokenBucket& operator=(TokenBucket&&) = delete;
    TokenBucket& operator=(const TokenBucket&) = delete;

    std::chrono::milliseconds wait_time(size_t tokens_required);

    bool get_tokens(size_t tokens_required);

private:
    size_t get_tokens_available(const std::chrono::steady_clock::time_point& current_time);

    constexpr static uint64_t elapsed_time(
            const std::chrono::steady_clock::time_point& ref_time,
            const std::chrono::steady_clock::time_point& current_time);

private:
    static constexpr size_t min_rate_ = 64000; // 64KB
    size_t capacity_;
    size_t tokens_;
    size_t rate_;
    std::chrono::steady_clock::time_point timestamp_;

    std::mutex mtx_;
};

inline TokenBucket::TokenBucket(size_t rate, size_t burst)
    : capacity_(burst),
      rate_(rate)
{
    // Adjust to min rate
    if (rate_ == 0)
    {
        rate_ = min_rate_;
    }

    // Adjust capacity to rate
    if (rate_ > capacity_)
    {
        capacity_ = rate_;
    }

    // Adjust capacity to min rate if we use any kind of burst
    if ((burst != 0) && (capacity_ < min_rate_))
    {
        capacity_ = min_rate_;
    }

    tokens_    = std::min(capacity_, rate_);
    timestamp_ = std::chrono::steady_clock::now();
}

inline std::chrono::milliseconds TokenBucket::wait_time(size_t tokens_required)
{
    std::chrono::milliseconds rv(0);
    std::lock_guard<std::mutex> lock(mtx_);
    size_t tokens_available = get_tokens_available(std::chrono::steady_clock::now());
    if (tokens_required > tokens_available)
    {
        rv = std::chrono::milliseconds(
                    std::max(uint64_t(1),
                             uint64_t((std::milli::den * (tokens_available - tokens_required)) / rate_)));
    }
    return rv;
}

inline bool TokenBucket::get_tokens(size_t tokens_required)
{
    bool rv = false;
    std::lock_guard<std::mutex> lock(mtx_);
    auto current_time = std::chrono::steady_clock::now();
    size_t available_tokens = get_tokens_available(current_time);
    if (tokens_required <= available_tokens)
    {
        tokens_ = available_tokens - tokens_required;
        timestamp_ = current_time;
        rv = true;
    }
    return rv;
}

inline size_t TokenBucket::get_tokens_available(
        const std::chrono::steady_clock::time_point& current_time)
{
    return std::min(capacity_, tokens_ + size_t((rate_ * elapsed_time(timestamp_, current_time)) / std::milli::den));
}

inline constexpr uint64_t TokenBucket::elapsed_time(
        const std::chrono::steady_clock::time_point& ref_time,
        const std::chrono::steady_clock::time_point& current_time)
{
    return uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(current_time - ref_time).count());
}

} // namespace utils
} // namespace uxr
} // namespace eprosima

#endif // UXR_UTILS_DATAREADER_TOKENBUCKET_HPP_
