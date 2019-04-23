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


#include <uxr/agent/client/session/stream/InputStream.hpp>
#include <map>
#include <queue>
#include <mutex>

#include <gtest/gtest.h>

namespace eprosima {
namespace uxr {
namespace testing {

/****************************************************************************************
 * None Input Stream.
 ****************************************************************************************/
class NoneInputStreamTest : public ::testing::Test
{
public:
    NoneInputStreamTest()
        : none_stream_{}
    {}

public:
    NoneInputStream none_stream_;
};

TEST_F(NoneInputStreamTest, PushMessage)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    for (uint16_t i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        ASSERT_TRUE(none_stream_.push_message(std::move(input_message), 0xFFFF));
    }
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(none_stream_.push_message(std::move(input_message), 0xFFFF));
}

TEST_F(NoneInputStreamTest, PopMessage)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    for (uint16_t i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        none_stream_.push_message(std::move(input_message), 0xFFFF);
    }

    for (uint16_t i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_TRUE(none_stream_.pop_message(input_message));
    }
    ASSERT_FALSE(none_stream_.pop_message(input_message));
}

TEST_F(NoneInputStreamTest, Reset)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    for (uint16_t i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        none_stream_.push_message(std::move(input_message), 0xFFFF);
    }

    ASSERT_TRUE(none_stream_.pop_message(input_message));
    none_stream_.reset();
    ASSERT_FALSE(none_stream_.pop_message(input_message));
}

/****************************************************************************************
 * Best-Effort Input Stream.
 ****************************************************************************************/
class BestEffortInputStreamTest : public ::testing::Test
{
public:
    BestEffortInputStreamTest()
        : best_effort_stream_{}
    {}

public:
    BestEffortInputStream best_effort_stream_;
};

TEST_F(BestEffortInputStreamTest, PushMessage)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(best_effort_stream_.push_message(std::move(input_message), 0xFFFF));
    for (uint16_t i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        ASSERT_TRUE(best_effort_stream_.push_message(std::move(input_message), i));
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        ASSERT_FALSE(best_effort_stream_.push_message(std::move(input_message), i));
    }
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(best_effort_stream_.push_message(std::move(input_message), BEST_EFFORT_STREAM_DEPTH));
}

TEST_F(BestEffortInputStreamTest, PopMessage)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    for (uint16_t i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        best_effort_stream_.push_message(std::move(input_message), i);
    }

    for (uint16_t i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_TRUE(best_effort_stream_.pop_message(input_message));
    }
    ASSERT_FALSE(best_effort_stream_.pop_message(input_message));
}

TEST_F(BestEffortInputStreamTest, Reset)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    best_effort_stream_.push_message(std::move(input_message), 0x0000);

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(best_effort_stream_.push_message(std::move(input_message), 0x0000));

    best_effort_stream_.reset();

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_TRUE(best_effort_stream_.push_message(std::move(input_message), 0x0000));

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    best_effort_stream_.push_message(std::move(input_message), 0x0001);

    best_effort_stream_.reset();

    ASSERT_FALSE(best_effort_stream_.pop_message(input_message));
}

TEST_F(BestEffortInputStreamTest, BorderCases)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message(new InputMessage(buf, sizeof(buf)));

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(best_effort_stream_.push_message(std::move(input_message), SeqNum::ADD_RANGE[1]));

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_TRUE(best_effort_stream_.push_message(std::move(input_message), SeqNum::ADD_RANGE[1] - 1));
}

/****************************************************************************************
 * Reliable Input Stream.
 ****************************************************************************************/
class ReliableInputStreamTest : public ::testing::Test
{
public:
    ReliableInputStreamTest()
        : reliable_stream_{}
    {}

public:
    ReliableInputStream reliable_stream_;
};

TEST_F(ReliableInputStreamTest, NextMessage)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(reliable_stream_.push_message(std::move(input_message), 0xFFFF));
    for (uint16_t i = 0; i < RELIABLE_STREAM_DEPTH; ++i)
    {
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        ASSERT_TRUE(reliable_stream_.push_message(std::move(input_message), i));

        input_message.reset(new InputMessage(buf, sizeof(buf)));
        ASSERT_FALSE(reliable_stream_.push_message(std::move(input_message), i));
    }
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(reliable_stream_.push_message(std::move(input_message), RELIABLE_STREAM_DEPTH));
}

TEST_F(ReliableInputStreamTest, PopMessage)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    reliable_stream_.push_message(std::move(input_message), 0x0000);
    ASSERT_TRUE(reliable_stream_.pop_message(input_message));

    for (uint16_t i = 1; i < RELIABLE_STREAM_DEPTH + 1; ++i)
    {
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        reliable_stream_.push_message(std::move(input_message), i);
    }

    for (uint16_t i = 1; i < RELIABLE_STREAM_DEPTH + 1; ++i)
    {
        ASSERT_TRUE(reliable_stream_.pop_message(input_message));
    }
    ASSERT_FALSE(reliable_stream_.pop_message(input_message));
}

TEST_F(ReliableInputStreamTest, PushPopMessage)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    reliable_stream_.push_message(std::move(input_message), 0x0000);
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    reliable_stream_.push_message(std::move(input_message), 0x0002);
    reliable_stream_.pop_message(input_message);

    ASSERT_FALSE(reliable_stream_.pop_message(input_message));
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    reliable_stream_.push_message(std::move(input_message), 0x0001);
    ASSERT_TRUE(reliable_stream_.pop_message(input_message));
    ASSERT_TRUE(reliable_stream_.pop_message(input_message));
    ASSERT_FALSE(reliable_stream_.pop_message(input_message));
}

TEST_F(ReliableInputStreamTest, Reset)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    reliable_stream_.push_message(std::move(input_message), 0x0000);
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(reliable_stream_.push_message(std::move(input_message), 0x0000));
    reliable_stream_.reset();
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_TRUE(reliable_stream_.push_message(std::move(input_message), 0x0000));

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    reliable_stream_.push_message(std::move(input_message), 0x0001);
    reliable_stream_.reset();
    ASSERT_FALSE(reliable_stream_.pop_message(input_message));
}

TEST_F(ReliableInputStreamTest, BorderCases)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(reliable_stream_.push_message(std::move(input_message), RELIABLE_STREAM_DEPTH));
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_TRUE(reliable_stream_.push_message(std::move(input_message), RELIABLE_STREAM_DEPTH - 1));
}

TEST_F(ReliableInputStreamTest, UpdateFromHeartbeat)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    reliable_stream_.update_from_heartbeat(0x0001, 0xFFFF);
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_FALSE(reliable_stream_.push_message(std::move(input_message), 0x0000));
    input_message.reset(new InputMessage(buf, sizeof(buf)));
    ASSERT_TRUE(reliable_stream_.push_message(std::move(input_message), 0x0001));
}

TEST_F(ReliableInputStreamTest, FillAcknack)
{
    uint8_t buf[128] = {0};
    InputMessagePtr input_message;

    dds::xrce::ACKNACK_Payload acknack;
    reliable_stream_.fill_acknack(acknack);
    ASSERT_EQ(acknack.first_unacked_seq_num(), 0x0000);
    ASSERT_EQ(acknack.nack_bitmap().at(0), 0x00);
    ASSERT_EQ(acknack.nack_bitmap().at(1), 0x00);

    for (int i = 0; i < 16; ++i)
    {
        reliable_stream_.reset();
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        reliable_stream_.push_message(std::move(input_message), i);
        reliable_stream_.fill_acknack(acknack);
        ASSERT_EQ(acknack.first_unacked_seq_num(), 0x0000);

        uint16_t raw_bitmap = uint16_t((1 << i) - 1);
        ASSERT_EQ(acknack.nack_bitmap().at(0), (raw_bitmap & 0xFF00) >> 8);
        ASSERT_EQ(acknack.nack_bitmap().at(1), raw_bitmap & 0x00FF);
    }

    for (int i = 2; i < 16; ++i)
    {
        reliable_stream_.reset();
        input_message.reset(new InputMessage(buf, sizeof(buf)));
        reliable_stream_.push_message(std::move(input_message), i);

        for (int j = 0; j < i - 1; ++j)
        {
            input_message.reset(new InputMessage(buf, sizeof(buf)));
            reliable_stream_.push_message(std::move(input_message), j);
        }

        reliable_stream_.fill_acknack(acknack);
        ASSERT_EQ(acknack.first_unacked_seq_num(), 0x0000);

        uint16_t raw_bitmap = uint16_t(1 << (i - 1));
        ASSERT_EQ(acknack.nack_bitmap().at(0), (raw_bitmap & 0xFF00) >> 8);
        ASSERT_EQ(acknack.nack_bitmap().at(1), raw_bitmap & 0x00FF);
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
