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

#ifndef DATA_READER_TESTS_
#define DATA_READER_TESTS_

#include "uxr/agent/datareader/DataReader.hpp"

#include <gtest/gtest.h>

namespace eprosima {
namespace uxr {
namespace testing {

class DataReaderTests : public ::testing::Test
{
  protected:
    DataReaderTests();

    virtual ~DataReaderTests() = default;

    eprosima::uxr::DataReader data_reader_;
    bool data_reader_init_ = false;
    unsigned int read_count_ = 0;
    dds::xrce::StreamId stream_id_;
    dds::xrce::ObjectId object_id_;
    dds::xrce::RequestId req_id_;
    const dds::xrce::RequestId fixed_request_id = {{1, 2}};
    const dds::xrce::ObjectId fixed_object_id   = {{10, 20}};
};
} // namespace testing
} // namespace uxr
} // namespace eprosima
#endif // !DATA_READER_TESTS_
