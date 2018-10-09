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

#include <uxr/agent/utils/TokenBucket.hpp>

#include <algorithm>
#include <iostream>

using eprosima::uxr::utils::TokenBucket;

TokenBucket::TokenBucket(size_t rate, size_t burst)
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

TokenBucket::TokenBucket::TokenBucket(const TokenBucket& other)
    : capacity_(other.capacity_),
      tokens_(other.tokens_),
      rate_(other.rate_),
      timestamp_(other.timestamp_)
{
}

TokenBucket::TokenBucket(TokenBucket&& other) noexcept
    : capacity_(other.capacity_),
      tokens_(other.tokens_),
      rate_(other.rate_),
      timestamp_(other.timestamp_)
{
}

TokenBucket& TokenBucket::operator=(const TokenBucket& other)
{
    capacity_  = other.capacity_;
    tokens_    = other.tokens_;
    rate_      = other.rate_;
    timestamp_ = other.timestamp_;
    return *this;
}

TokenBucket& TokenBucket::TokenBucket::operator=(TokenBucket&& other) noexcept
{
    capacity_  = other.capacity_;
    tokens_    = other.tokens_;
    rate_      = other.rate_;
    timestamp_ = other.timestamp_;
    return *this;
}

bool TokenBucket::get_tokens(size_t tokens)
{
    std::lock_guard<std::mutex> lock(data_mutex_);
    if (tokens <= available_tokens())
    {
        tokens_ = available_tokens() - tokens;
        timestamp_ = std::chrono::steady_clock::now();
    }
    else
    {
        return false;
    }
    return true;
}

size_t TokenBucket::available_tokens()
{
    auto now = std::chrono::steady_clock::now();
    auto delta_sec = std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp_).count();
    return std::min(capacity_, tokens_ + (size_t)((rate_ * delta_sec) / 1000));
}
