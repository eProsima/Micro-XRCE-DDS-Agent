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

#include "agent/datareader/DataReader.h"

#include <gtest/gtest.h>

namespace eprosima {
namespace micrortps {
namespace testing {

class DataReaderTests : public ::testing::Test, public ReaderListener
{
  protected:
    DataReaderTests();

    virtual ~DataReaderTests() = default;

    void on_read_data(const ObjectId& object_id, const RequestId& req_id,
                      const std::vector<unsigned char>& buffer) override;

    eprosima::micrortps::DataReader data_reader_;
    bool data_reader_init_ = false;
    unsigned int read_count_ = 0;
    ObjectId object_id_;
    RequestId req_id_;
    const RequestId fixed_request_id = {{1, 2}};
    const ObjectId fixed_object_id   = {{10, 20}};
};
} // namespace testing
} // namespace micrortps
} // namespace eprosima
#endif // !DATA_READER_TESTS_