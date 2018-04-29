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

#include <agent/datareader/TokenBucket.h>

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
};

TEST_F(TokenBucketTests, RateNoBurst)
{
    // Rate and capacity should be automatically adjusted to be equal.
    const unsigned int rate = 70;
    TokenBucket bucket{rate,0};
    ASSERT_TRUE(bucket.get_tokens(rate));
    ASSERT_FALSE(bucket.get_tokens(10));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_FALSE(bucket.get_tokens(rate * 2));
    ASSERT_TRUE(bucket.get_tokens(10));
    ASSERT_TRUE(bucket.get_tokens(1));
    ASSERT_FALSE(bucket.get_tokens(60));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(bucket.get_tokens(1));
    ASSERT_FALSE(bucket.get_tokens(rate));
}

TEST_F(TokenBucketTests, NoRateNoBurst)
{
    const unsigned int min_rate = 64000;
    TokenBucket bucket{0};
    ASSERT_TRUE(bucket.get_tokens(min_rate));
    ASSERT_FALSE(bucket.get_tokens(10));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_FALSE(bucket.get_tokens(min_rate * 2));
    ASSERT_TRUE(bucket.get_tokens(63000));
    ASSERT_TRUE(bucket.get_tokens(1));
    ASSERT_FALSE(bucket.get_tokens(1000));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(bucket.get_tokens(1));
    ASSERT_FALSE(bucket.get_tokens(min_rate));
}

TEST_F(TokenBucketTests, AdjustedBurst)
{
    const unsigned int min_rate = 64000;
    const unsigned int rate     = (unsigned int) (min_rate * 0.5);
    TokenBucket bucket{rate, 10};
    ASSERT_TRUE(bucket.get_tokens(rate));
    ASSERT_FALSE(bucket.get_tokens(10));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_TRUE(bucket.get_tokens(rate * 2));
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
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_FALSE(bucket.get_tokens(udp_size * 2));
    ASSERT_TRUE(bucket.get_tokens(udp_size));
    ASSERT_FALSE(bucket.get_tokens(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(bucket.get_tokens(udp_size));
    ASSERT_FALSE(bucket.get_tokens(10));
}

TEST_F(TokenBucketTests, RateMeassure)
{
    const size_t test_package = 300;
    const unsigned int bucket_size = 100;
    const size_t tokens = 50;
    TokenBucket bucket{bucket_size,0};

    std::cout << "Testing package size: " << test_package << " Rate: " << bucket_size << " Token: " << tokens
              << std::endl;

    size_t sended_size = 0;
    int count_tokens   = 0;
    auto start         = std::chrono::steady_clock::now();
    while (sended_size < test_package)
    {
        if (bucket.get_tokens(tokens))
        {
            ++count_tokens;
            sended_size += tokens;
        }
    }
    auto end     = std::chrono::steady_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    ASSERT_EQ(test_package / bucket_size - 1, (size_t)seconds);
    ASSERT_EQ(test_package / tokens, count_tokens);
}

} // namespace testing
} // namespace micrortps
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
