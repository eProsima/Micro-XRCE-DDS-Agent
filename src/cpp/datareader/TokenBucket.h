#ifndef _DATAREADER_TOKENBUCKET_H
#define _DATAREADER_TOKENBUCKET_H

#include <chrono>
#include <mutex>

namespace eprosima {
namespace micrortps {
namespace utils {

class TokenBucket
{
  public:
    TokenBucket(size_t rate, size_t burst = 0);
    TokenBucket(TokenBucket&& other) noexcept;
    TokenBucket(const TokenBucket& other);
    TokenBucket& operator=(TokenBucket&& other) noexcept;
    TokenBucket& operator=(const TokenBucket& other);
    ~TokenBucket()       = default;

    bool get_tokens(size_t tokens);

  private:
    void update_tokens();

    static constexpr size_t min_rate_ = 64;
    double capacity_;
    double tokens_;
    size_t rate_;
    std::chrono::system_clock::time_point timestamp_;

    std::mutex data_mutex_;
};
} // namespace utils
} // namespace micrortps
} // namespace eprosima

#endif // _HOME_BORJA_DEV_MICRORTPS_DEV_SRC_CPP_DATAREADER_TOKENBUCKET_H