#include "TokenBucket.h"

#include <algorithm>

#include <iostream>

using eprosima::micrortps::utils::TokenBucket;

TokenBucket::TokenBucket(size_t rate, size_t burst) : capacity_(burst), rate_(rate)
{
    if (rate_ < min_rate_)
    {
        rate_ = min_rate_;
    }

    if (burst == 0)
    {
        capacity_ = rate_;
    }

    tokens_ = std::min(capacity_, static_cast<double>(rate_));

    timestamp_ = std::chrono::system_clock::now();
}

TokenBucket::TokenBucket::TokenBucket(const TokenBucket& other)
    : capacity_(other.capacity_), tokens_(other.tokens_), rate_(other.rate_), timestamp_(other.timestamp_)
{
}

TokenBucket::TokenBucket(TokenBucket&& other) noexcept
    : capacity_(other.capacity_), tokens_(other.tokens_), rate_(other.rate_), timestamp_(other.timestamp_)
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
    update_tokens();
    if (tokens <= tokens_)
    {
        tokens_ -= tokens;
    }
    else
    {
        return false;
    }
    return true;
}

void TokenBucket::update_tokens()
{
    auto now = std::chrono::system_clock::now();
    if (tokens_ < capacity_)
    {
        auto seconds = std::chrono::duration<double>(now - timestamp_).count();
        // std::cout << " seconds: " << seconds << std::endl;
        double delta_tokens = rate_ * seconds;

        // std::cout << " tokens: " << seconds << std::endl;

        tokens_ = std::min(capacity_, tokens_ + delta_tokens);
    }
    timestamp_ = now;
}
