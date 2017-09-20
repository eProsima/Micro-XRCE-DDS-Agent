#ifndef _MESSAGE_QUEUE_
#define _MESSAGE_QUEUE_

#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace eprosima {
namespace micrortps {
class Message
{
public:
    
private:

};

class MessageQueue
{
public:
    MessageQueue();
    MessageQueue(const MessageQueue& other) = delete;
    MessageQueue& operator=(const MessageQueue& other) = delete;
    
    Message pop();
    void push(const Message& item);
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