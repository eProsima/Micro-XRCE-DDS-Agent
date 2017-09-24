#ifndef _MESSAGE_QUEUE_
#define _MESSAGE_QUEUE_

#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace eprosima {
namespace micrortps {

const size_t max_message_size = 2048;

class Message
{
public:
    void set_real_size(size_t size){size_ = size;}
    size_t get_real_size(void){return size_;}
    std::array<char, max_message_size>& get_buffer() {return buffer_;};
private:
    size_t size_;
    std::array<char, max_message_size> buffer_;
};

class MessageQueue
{
public:
    MessageQueue();
    MessageQueue(const MessageQueue& other) = delete;
    MessageQueue& operator=(const MessageQueue& other) = delete;
    
    Message pop();
    void push(const Message& new_message);
    void abort();
  
 private:
    std::queue<Message> internal_queue_;
    std::mutex data_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> aborted_;
};
} // micrortps
} // eprosima
#endif
