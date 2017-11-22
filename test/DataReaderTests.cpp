#include "DataReaderTests.h"

// DataReader(eprosima::fastrtps::Participant* rtps_participant, ReaderListener* read_list);
// ~DataReader() noexcept override;

// bool init();
// bool init(const std::string& xmlrep);
// int read(const READ_DATA_Payload& read_data);

// void on_max_timeout(const asio::error_code& error);
// void on_rate_timeout(const asio::error_code& error);
// void onSubscriptionMatched(eprosima::fastrtps::rtps::MatchingInfo& info);
// void onNewDataMessage(fastrtps::Subscriber* sub);

#include <gtest/gtest.h>

namespace eprosima {
namespace micrortps {
namespace testing {

DataReaderTests::DataReaderTests() : data_reader_(fixed_object_id, nullptr, this, "default_xrce_subscriber_profile")
{
    data_reader_init_ = data_reader_.init();
}

void DataReaderTests::on_read_data(const ObjectId& object_id, const RequestId& req_id,
                                   const std::vector<unsigned char>& buffer)
{
    object_id_ = object_id;
    req_id_    = req_id;
    ++read_count_;
}

TEST_F(DataReaderTests, ReadFormatData)
{
    ASSERT_TRUE(data_reader_init_);
    READ_DATA_Payload read_conf;
    read_conf.object_id(fixed_object_id);
    read_conf.request_id(fixed_request_id);
    read_conf.read_specification().delivery_config().data_format(FORMAT_DATA);
    int tries = 300;
    data_reader_.read(read_conf);
    while (!data_reader_.has_message() && tries > 0)
    {
        --tries;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    ASSERT_EQ(read_count_, 1);
    ASSERT_EQ(req_id_, fixed_request_id);
    ASSERT_EQ(object_id_, fixed_object_id);
}

TEST_F(DataReaderTests, ReadFormatDataSeq)
{
    ASSERT_TRUE(data_reader_init_);
    READ_DATA_Payload read_conf;
    read_conf.object_id(fixed_object_id);
    read_conf.request_id(fixed_request_id);
    eprosima::micrortps::DataDeliveryControl control;
    control.max_elapsed_time(20000);
    control.max_samples(10);
    control.max_rate(100);
    read_conf.read_specification().delivery_config().delivery_control(control, FORMAT_DATA_SEQ);
    int tries = 300;
    data_reader_.read(read_conf);
    // Waits for the first message.
    while (!data_reader_.has_message() && tries > 0)
    {
        --tries;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    // Wait to the read operation
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(read_count_, 10);
    ASSERT_EQ(req_id_, fixed_request_id);
    ASSERT_EQ(object_id_, fixed_object_id);
}

} // namespace testing
} // namespace micrortps
} // namespace eprosima