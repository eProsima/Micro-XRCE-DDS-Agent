
#include <datareader/TokenBucket.h>

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

    virtual ~TokenBucketTests() = default;
};

TEST_F(TokenBucketTests, RateSizeBucket)
{
    const int rate = 70;
    TokenBucket bucket{rate};
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

TEST_F(TokenBucketTests, SmallSizeBucket)
{
    const int size = 65;
    const int rate = 70;
    TokenBucket bucket{rate, size};
    ASSERT_FALSE(bucket.get_tokens(rate));
    ASSERT_TRUE(bucket.get_tokens(size));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    ASSERT_FALSE(bucket.get_tokens(rate));
    ASSERT_TRUE(bucket.get_tokens(size));
    ASSERT_FALSE(bucket.get_tokens(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(bucket.get_tokens(1));
    ASSERT_FALSE(bucket.get_tokens(size));
}

TEST_F(TokenBucketTests, BurstBucket)
{
    const int size = 70;
    const int rate = 66;
    TokenBucket bucket{rate, size};
    ASSERT_TRUE(bucket.get_tokens(rate / 2));
    ASSERT_TRUE(bucket.get_tokens(rate / 2));
    ASSERT_FALSE(bucket.get_tokens(2));
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ASSERT_FALSE(bucket.get_tokens(size + 1));
    ASSERT_TRUE(bucket.get_tokens(size));
    ASSERT_FALSE(bucket.get_tokens(1));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(bucket.get_tokens(rate));
    ASSERT_FALSE(bucket.get_tokens(1));
}

TEST_F(TokenBucketTests, LimitToUDPBucket)
{
    const int udp_size = 64;
    TokenBucket bucket{1};
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
    const int bucket_size     = 100;
    const size_t tokens       = 50;
    TokenBucket bucket{bucket_size};

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
    ASSERT_EQ(test_package / bucket_size - 1, seconds);
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