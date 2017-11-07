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