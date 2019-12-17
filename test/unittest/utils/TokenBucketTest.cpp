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

#include <uxr/agent/utils/TokenBucket.hpp>

#include <gtest/gtest.h>

#include <random>
#include <thread>

namespace eprosima {
namespace uxr {
namespace testing {

using eprosima::uxr::utils::TokenBucket;

class TokenBucketTest : public ::testing::Test
{
protected:
    TokenBucketTest() = default;
    ~TokenBucketTest() override = default;
};

TEST_F(TokenBucketTest, initial_condition)
{
    const size_t rate = 64000;
    const size_t capacity = 10 * rate;

    /* Constructor with rate and capacity. */
    TokenBucket bucket_one{rate, capacity};
    ASSERT_EQ(bucket_one.get_rate(), rate);
    ASSERT_EQ(bucket_one.get_capacity(), capacity);
    ASSERT_EQ(bucket_one.get_available_tokens(), bucket_one.get_capacity());

    /* Constructor with rate only. */
    TokenBucket bucket_two{rate};
    ASSERT_EQ(bucket_two.get_rate(), rate);
    ASSERT_EQ(bucket_two.get_capacity(), bucket_two.get_rate());
    ASSERT_EQ(bucket_two.get_available_tokens(), bucket_two.get_capacity());
}

TEST_F(TokenBucketTest, capacity)
{
    const std::chrono::milliseconds timeout{0};
    const size_t rate = 64000;
    const size_t capacity = 10 * rate;

    /* At the beginning the token bucket is full. */
    TokenBucket bucket{rate, capacity};

    /* Try to get more than capacity. */
    ASSERT_FALSE(bucket.consume_tokens(capacity + 1, timeout));

    /* Try to get just the capacity. */
    ASSERT_TRUE(bucket.consume_tokens(capacity, timeout));
}

TEST_F(TokenBucketTest, timeout)
{
    const size_t rate = 10000000;
    TokenBucket bucket{rate};

    /* Empty bucket. */
    bucket.consume_tokens(rate, std::chrono::milliseconds(0));

    /* Not enough time. */
    ASSERT_FALSE(bucket.consume_tokens(rate, std::chrono::milliseconds(900)));

    /* Empty bucket. */
    ASSERT_TRUE(bucket.consume_tokens(rate, std::chrono::milliseconds(100)));

    /* Take all with timeout. */
    ASSERT_TRUE(bucket.consume_tokens(rate, std::chrono::seconds(1)));
}

TEST_F(TokenBucketTest, rate_with_full_bucket)
{
    const std::chrono::seconds timeout{0};
    const size_t rate = 100;
    const size_t bunch_size = 50;
    const uint8_t expected_elapsed_time = 3;
    const size_t requested_tokens = expected_elapsed_time * rate;
    TokenBucket bucket{rate};

    std::cout << "Testing getting " << requested_tokens << " tokens at a rate of " << rate;
    std::cout << " tokens per second, reading by " << bunch_size << std::endl;

    size_t sent_size  = 0;
    int reading_counter = 0;
    auto start = std::chrono::steady_clock::now();

    while (sent_size < requested_tokens)
    {
        if (bucket.consume_tokens(bunch_size, timeout))
        {
            ++reading_counter;
            sent_size += bunch_size;
        }
    }
    auto end = std::chrono::steady_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    ASSERT_LT(static_cast<uint8_t>(seconds), expected_elapsed_time);
    ASSERT_EQ(requested_tokens / bunch_size, reading_counter);
}

TEST_F(TokenBucketTest, rate_with_empty_bucket)
{
    const std::chrono::seconds timeout{0};
    const size_t rate = 100;
    const size_t bunch_size = 50;
    const uint8_t expected_elapsed_time = 3;
    const size_t requested_tokens = expected_elapsed_time * rate;
    TokenBucket bucket{rate};

    std::cout << "Testing getting " << requested_tokens << " tokens at a rate of " << rate;
    std::cout << " tokens per second, reading by " << bunch_size << std::endl;

    size_t sent_size  = 0;
    int reading_counter = 0;
    auto start = std::chrono::steady_clock::now();

    /* Empty bucket. */
    bucket.consume_tokens(bucket.get_capacity(), std::chrono::seconds(0));

    while (sent_size < requested_tokens)
    {
        if (bucket.consume_tokens(bunch_size, timeout))
        {
            ++reading_counter;
            sent_size += bunch_size;
        }
    }
    auto end = std::chrono::steady_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    ASSERT_GE(static_cast<uint8_t>(seconds), expected_elapsed_time);
    ASSERT_EQ(requested_tokens / bunch_size, reading_counter);
}

} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
