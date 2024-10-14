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

#include "DataReaderTests.h"

#include <gtest/gtest.h>

namespace eprosima {
namespace uxr {
namespace testing {

DataReaderTests::DataReaderTests()
{
}

void DataReaderTests::on_read_data(const dds::xrce::StreamId& stream_id,
                                   const dds::xrce::ObjectId& object_id,
                                   const dds::xrce::RequestId& req_id,
                                   const std::vector<unsigned char>& buffer)
{
    // TODO.
    (void) buffer;
    stream_id_ = stream_id;
    object_id_ = object_id;
    req_id_    = req_id;
    ++read_count_;
}

TEST_F(DataReaderTests, DISABLED_ReadFormatData)
{
    ASSERT_TRUE(data_reader_init_);
    dds::xrce::READ_DATA_Payload read_conf;
    read_conf.object_id(fixed_object_id);
    read_conf.request_id(fixed_request_id);
    read_conf.read_specification().data_format(dds::xrce::FORMAT_DATA);
    int tries = 300;
    data_reader_->read(read_conf, stream_id_);
    while (!data_reader_->has_message() && tries > 0)
    {
        --tries;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    ASSERT_EQ(read_count_, 1u);
    ASSERT_EQ(req_id_, fixed_request_id);
    ASSERT_EQ(object_id_, fixed_object_id);
}

TEST_F(DataReaderTests, DISABLED_ReadFormatDataSeq)
{
    ASSERT_TRUE(data_reader_init_);
    dds::xrce::READ_DATA_Payload read_conf;
    read_conf.object_id(fixed_object_id);
    read_conf.request_id(fixed_request_id);
    dds::xrce::DataDeliveryControl control;
    control.max_elapsed_time(20000);
    control.max_samples(10);
    control.max_bytes_per_second(100);
    read_conf.read_specification().data_format(dds::xrce::FORMAT_DATA_SEQ);
    read_conf.read_specification().delivery_control(control);
    int tries = 300;
    data_reader_->read(read_conf, stream_id_);
    // Waits for the first message.
    while (!data_reader_->has_message() && tries > 0)
    {
        --tries;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    // Wait to the read operation
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ASSERT_EQ(read_count_, 10u);
    ASSERT_EQ(req_id_, fixed_request_id);
    ASSERT_EQ(object_id_, fixed_object_id);
}

} // namespace testing
} // namespace uxr
} // namespace eprosima
