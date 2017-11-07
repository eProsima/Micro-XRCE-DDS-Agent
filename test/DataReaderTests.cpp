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

DataReaderTests::DataReaderTests() : data_reader_(nullptr, this)
{
    data_reader_.init();
}

void DataReaderTests::on_read_data(const ObjectId& object_id, const RequestId& req_id,
                                   const std::vector<unsigned char>& buffer)
{
    object_id_ = object_id;
    req_id_    = req_id;
    ++read_count_;
}

TEST_F(DataReaderTests, ReadOnce)
{
    READ_DATA_Payload read_conf;
    read_conf.object_id(fixed_object_id);
    read_conf.request_id(fixed_request_id);
    read_conf.read_specification().delivery_config().data_format(FORMAT_DATA);
    int tries = 300;
    while (!data_reader_.has_message() && tries > 0)
    {
        --tries;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    data_reader_.read(read_conf);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_EQ(read_count_, 1);
    ASSERT_EQ(req_id_, fixed_request_id);
    ASSERT_EQ(object_id_, fixed_object_id);
}

} // namespace testing
} // namespace micrortps
} // namespace eprosima