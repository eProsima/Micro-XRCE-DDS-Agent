// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/transport/serial/baud_rate_table_linux.h>

#include <gtest/gtest.h>

namespace eprosima {
namespace uxr {
namespace testing {


class BaudRateTableUnitTests :  public ::testing::Test
{
public:
    BaudRateTableUnitTests() : table{}
    {
        table.push_back(std::make_tuple("0",          B0));
        table.push_back(std::make_tuple("50",         B50));
        table.push_back(std::make_tuple("75",         B75));
        table.push_back(std::make_tuple("110",        B110));
        table.push_back(std::make_tuple("134",        B134));
        table.push_back(std::make_tuple("150",        B150));
        table.push_back(std::make_tuple("200",        B200));
        table.push_back(std::make_tuple("300",        B300));
        table.push_back(std::make_tuple("600",        B600));
        table.push_back(std::make_tuple("1200",       B1200));
        table.push_back(std::make_tuple("1800",       B1800));
        table.push_back(std::make_tuple("2400",       B2400));
        table.push_back(std::make_tuple("4800",       B4800));
        table.push_back(std::make_tuple("9600",       B9600));
        table.push_back(std::make_tuple("19200",      B19200));
        table.push_back(std::make_tuple("38400",      B38400));
        table.push_back(std::make_tuple("57600",      B57600));
        table.push_back(std::make_tuple("115200",     B115200));
        table.push_back(std::make_tuple("230400",     B230400));
        table.push_back(std::make_tuple("460800",     B460800));
        table.push_back(std::make_tuple("500000",     B500000));
        table.push_back(std::make_tuple("576000",     B576000));
        table.push_back(std::make_tuple("921600",     B921600));
        table.push_back(std::make_tuple("1000000",    B1000000));
        table.push_back(std::make_tuple("1152000",    B1152000));
        table.push_back(std::make_tuple("1500000",    B1500000));
        table.push_back(std::make_tuple("2000000",    B2000000));
        table.push_back(std::make_tuple("2500000",    B2500000));
        table.push_back(std::make_tuple("3000000",    B3000000));
        table.push_back(std::make_tuple("3500000",    B3500000));
        table.push_back(std::make_tuple("4000000",    B4000000));
        table.push_back(std::make_tuple("11",         speed_t(11)));
    }

    virtual ~BaudRateTableUnitTests() = default;

public:
    std::vector<std::tuple<const char*, speed_t>> table;
};

TEST_F(BaudRateTableUnitTests, CreateClientOk)
{
    for (auto& b : table)
    {
        ASSERT_EQ(getBaudRate(std::get<0>(b)), std::get<1>(b));
    }
}

} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
