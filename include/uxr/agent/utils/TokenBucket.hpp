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
#include <thread>

namespace eprosima {
namespace uxr {
namespace utils {

class TokenBucket
{
public:
    explicit TokenBucket(
            size_t rate,
            size_t capacity = 0);

    TokenBucket(TokenBucket&&) = delete;
    TokenBucket(const TokenBucket&) = delete;
    TokenBucket& operator=(TokenBucket&&) = delete;
    TokenBucket& operator=(const TokenBucket&) = delete;

    template<typename T>
    bool consume_tokens(
            size_t required_tokens,
            T&& timeout);

    size_t get_rate() { return rate_; }
    size_t get_capacity() { return capacity_; }
    size_t get_available_tokens() { return tokens_; }

private:
    const size_t rate_;
    const size_t capacity_;
    size_t tokens_;
    std::chrono::steady_clock::time_point timestamp_;
};

inline TokenBucket::TokenBucket(
        size_t rate,
        size_t capacity)
    : rate_(rate)
    , capacity_((capacity == 0) ? rate : capacity)
{
    tokens_    = capacity_;
    timestamp_ = std::chrono::steady_clock::now();
}

template<typename T>
inline bool TokenBucket::consume_tokens(
        size_t required_tokens,
        T&& timeout)
{
    using namespace std::chrono;

    if (required_tokens > capacity_)
    {
        return false;
    }

    bool rv = false;
    const steady_clock::time_point init_time = steady_clock::now();
    const size_t current_tokens = std::min(
        capacity_,
        tokens_ + size_t((rate_ * uint64_t(duration_cast<milliseconds>(init_time - timestamp_).count())) / std::milli::den));

    if (current_tokens < required_tokens)
    {
        const steady_clock::time_point final_time =
            init_time +
            std::min(
                milliseconds(uint64_t((std::milli::den * (tokens_ - required_tokens)) / rate_)),
                duration_cast<milliseconds>(std::forward<T>(timeout)));
        const size_t expected_tokens = std::min(
            capacity_,
            tokens_ + size_t((rate_ * uint64_t(duration_cast<milliseconds>(final_time - timestamp_).count())) / std::milli::den));

        if (expected_tokens >= required_tokens)
        {
            tokens_ = expected_tokens - required_tokens;
            timestamp_ = final_time;
            rv = true;
        }

        std::this_thread::sleep_until(final_time);
    }
    else
    {
        tokens_ = current_tokens - required_tokens;
        timestamp_ = init_time;
        rv = true;
    }

    return rv;
}

} // namespace utils
} // namespace uxr
} // namespace eprosima

#endif // UXR_UTILS_DATAREADER_TOKENBUCKET_HPP_
