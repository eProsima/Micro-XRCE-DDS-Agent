// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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


#include <uxr/agent/utils/SeqNum.hpp>
#include <map>
#include <queue>
#include <mutex>

#include <gtest/gtest.h>

namespace eprosima {
namespace uxr {
namespace testing {

class SeqNumUnitTests : public ::testing::Test
{
public:
    SeqNumUnitTests() :
        foo_seqnum(0xFFFF),
        bar_seqnum(0xFFFF) {}

public:
    SeqNum foo_seqnum;
    SeqNum bar_seqnum;
};

TEST_F(SeqNumUnitTests, AdditionOperations)
{
    int to_add = 11;
    int pre_foo_seqnum = foo_seqnum;
    foo_seqnum += to_add;
    ASSERT_EQ(foo_seqnum, SeqNum(int(uint32_t(pre_foo_seqnum + to_add) % uint32_t(1 << 16))));

    to_add = -1;
    pre_foo_seqnum = foo_seqnum;
    foo_seqnum += (to_add);
    ASSERT_EQ(foo_seqnum, SeqNum(pre_foo_seqnum));

    to_add = (1 << 15) - 1;
    pre_foo_seqnum = foo_seqnum;
    foo_seqnum += to_add;
    ASSERT_EQ(foo_seqnum, SeqNum(int(uint32_t(pre_foo_seqnum + to_add) % uint32_t(1 << 16))));

    to_add = (1 << 15);
    pre_foo_seqnum = foo_seqnum;
    foo_seqnum += to_add;
    ASSERT_EQ(foo_seqnum, SeqNum(pre_foo_seqnum));
}

TEST_F(SeqNumUnitTests, ComparisonOperations)
{
    ASSERT_TRUE(foo_seqnum == bar_seqnum);
    ASSERT_TRUE(foo_seqnum == 0xFFFF);
    ASSERT_TRUE(0xFFFF == foo_seqnum);
    ASSERT_TRUE(foo_seqnum >= bar_seqnum);
    ASSERT_TRUE(foo_seqnum <= bar_seqnum);

    foo_seqnum += 1;
    ASSERT_EQ(foo_seqnum, 0x0000);
    ASSERT_TRUE(foo_seqnum > bar_seqnum);
    ASSERT_TRUE(bar_seqnum < foo_seqnum);

    foo_seqnum = 0;
    bar_seqnum = (1 << 15) - 1;
    ASSERT_TRUE(foo_seqnum < bar_seqnum);
    ASSERT_FALSE(foo_seqnum > bar_seqnum);

    /* Undefined behaviour (RFC-1035). */
    foo_seqnum = 0;
    bar_seqnum = (1 << 15);
    ASSERT_FALSE(foo_seqnum > bar_seqnum);
    ASSERT_FALSE(foo_seqnum < bar_seqnum);
    ASSERT_FALSE(foo_seqnum == bar_seqnum);
}

} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
