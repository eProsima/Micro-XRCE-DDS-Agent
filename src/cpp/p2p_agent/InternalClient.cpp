#include <uxr/agent/p2p_agent/InternalClient.hpp>

using namespace eprosima::uxr;

void InternalClient::connect()
{
    main_thread_ = std::thread(&InternalClient::run, this);
}

void InternalClient::add_topic(/* topic */)
{
    // enque topic
}

void InternalClient::run()
{
    // init session
    // Create participant
    // set as session initialized
    subscription_thread_ = std::thread(&InternalClient::subscribe, this);
    // run
}

void InternalClient::subscribe(/* middleware, or DataWritter already created. */)
{
    //while(1)
        // deque topic (this wait)
        //Create topic
        //Create sub
        //Create data_read
        //Create DataWriter into the Middleware
        //Create read request
}

void InternalClient::read_topic_callback()
{
    // Call the
}
