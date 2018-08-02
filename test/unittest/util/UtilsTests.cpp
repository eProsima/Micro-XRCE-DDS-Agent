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

#include <micrortps/agent/utils/TokenBucket.hpp>

#include <gtest/gtest.h>

#include <random>
#include <thread>

namespace eprosima {
namespace micrortps {
namespace testing {

using eprosima::micrortps::utils::TokenBucket;

class TokenBucketTests : public ::testing::Test
{
protected:
    TokenBucketTests() = default;
    ~TokenBucketTests() override = default;

public:
    size_t sleep_thread(int64_t millisecons)
    {
        auto start = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(millisecons));
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
        return static_cast<size_t>(duration);
    }
};

TEST_F(TokenBucketTests, RateNoBurst)
{
    // Rate and capacity should be automatically adjusted to be equal.
    const unsigned int rate = 70;
    TokenBucket bucket{rate,0};
    ASSERT_TRUE(bucket.get_tokens(rate));
    ASSERT_FALSE(bucket.get_tokens(10));
    ASSERT_FALSE(bucket.get_tokens(rate * sleep_thread(2100)));
    ASSERT_TRUE(bucket.get_tokens(10));
    ASSERT_TRUE(bucket.get_tokens(1));
    ASSERT_FALSE(bucket.get_tokens(60));
    ASSERT_TRUE(bucket.get_tokens(sleep_thread(1100)));
    ASSERT_FALSE(bucket.get_tokens(rate));
}

TEST_F(TokenBucketTests, NoRateNoBurst)
{
    const unsigned int min_rate = 64000;
    TokenBucket bucket{0};
    ASSERT_TRUE(bucket.get_tokens(min_rate));
    ASSERT_FALSE(bucket.get_tokens(10));
    ASSERT_FALSE(bucket.get_tokens(min_rate * sleep_thread(2100)));
    ASSERT_TRUE(bucket.get_tokens(63000));
    ASSERT_TRUE(bucket.get_tokens(1));
    ASSERT_FALSE(bucket.get_tokens(1000));
    ASSERT_TRUE(bucket.get_tokens(sleep_thread(1100)));
    ASSERT_FALSE(bucket.get_tokens(min_rate));
}

TEST_F(TokenBucketTests, AdjustedBurst)
{
    const unsigned int min_rate = 64000;
    const unsigned int rate     = (unsigned int) (min_rate * 0.5);
    TokenBucket bucket{rate, 10};
    ASSERT_TRUE(bucket.get_tokens(rate));
    ASSERT_FALSE(bucket.get_tokens(10));
    ASSERT_TRUE(bucket.get_tokens(rate * sleep_thread(2100)));
    ASSERT_FALSE(bucket.get_tokens(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(bucket.get_tokens((unsigned int) (rate * 0.5)));
    ASSERT_FALSE(bucket.get_tokens(rate));
}

TEST_F(TokenBucketTests, LimitToUDPBucket)
{
    const int udp_size = 64000;
    TokenBucket bucket{0, 1};
    ASSERT_FALSE(bucket.get_tokens(udp_size + 1));
    ASSERT_TRUE(bucket.get_tokens(udp_size));
    ASSERT_FALSE(bucket.get_tokens(udp_size * sleep_thread(2100)));
    ASSERT_TRUE(bucket.get_tokens(udp_size));
    ASSERT_FALSE(bucket.get_tokens(1));
    ASSERT_TRUE(bucket.get_tokens(udp_size * sleep_thread(1100)));
    ASSERT_FALSE(bucket.get_tokens(10));
}

TEST_F(TokenBucketTests, RateMeassure)
{
    const size_t requested_tokens = 300;
    const size_t bucket_rate = 100;
    const size_t bunch_size = 50;
    TokenBucket bucket{bucket_rate, 0};

    std::cout << "Testing getting " << requested_tokens << " tokens at a rate of " << bucket_rate;
    std::cout << " tokens per second, reading by " << bunch_size << std::endl;

    size_t sended_size  = 0;
    int reading_counter = 0;
    auto start         = std::chrono::steady_clock::now();
    while (sended_size < requested_tokens)
    {
        if (bucket.get_tokens(bunch_size))
        {
            ++reading_counter;
            sended_size += bunch_size;
        }
    }
    auto end     = std::chrono::steady_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    ASSERT_EQ((requested_tokens / bucket_rate) - 1, static_cast<size_t>(seconds));
    ASSERT_EQ(requested_tokens / bunch_size, reading_counter);
}

} // namespace testing
} // namespace micrortps
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
