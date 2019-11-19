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


#include <uxr/agent/client/session/stream/OutputStream.hpp>
#include <map>
#include <queue>
#include <mutex>

#include <gtest/gtest.h>

namespace eprosima {
namespace uxr {
namespace testing {

constexpr dds::xrce::SessionId session_id = 0x01;
constexpr dds::xrce::ClientKey client_key = {0xAA, 0xBB, 0xCC, 0xDD};
constexpr size_t mtu = 512;

/****************************************************************************************
 * None Output Stream.
 ****************************************************************************************/
class NoneOutputStreamTest : public ::testing::Test
{
public:
    NoneOutputStreamTest()
        : none_stream_{}
        , session_info_{client_key, session_id, mtu}
    {}

public:
    NoneOutputStream none_stream_;
    SessionInfo session_info_;
};

/**
 * @brief   This test checks the capacity of the stream.
 *          No more than BEST_EFFORT_STREAM_DEPTH message shall be pushed in a none stream.
 */
TEST_F(NoneOutputStreamTest, StreamCapacity)
{
    dds::xrce::WRITE_DATA_Payload_Data write_data{};
    for (int i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_TRUE(none_stream_.push_submessage(session_info_, dds::xrce::WRITE_DATA, write_data));
    }
    ASSERT_FALSE(none_stream_.push_submessage(session_info_, dds::xrce::WRITE_DATA, write_data));

    OutputMessagePtr output_message;
    for (int i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_TRUE(none_stream_.pop_message(output_message));
    }
    ASSERT_FALSE(none_stream_.pop_message(output_message));
}

/**
 * @brief   This test checks the maximum message size of the stream.
 *          The none stream shall be not able to push messages larger than the MTU.
 */
TEST_F(NoneOutputStreamTest, MessageCapacity)
{
    dds::xrce::MessageHeader header{};
    dds::xrce::SubmessageHeader subheader{};
    dds::xrce::WRITE_DATA_Payload_Data write_data{};

    size_t headers_size = header.getCdrSerializedSize() + subheader.getCdrSerializedSize();
    size_t bounded_size = mtu - headers_size - write_data.BaseObjectRequest::getCdrSerializedSize();

    write_data.data().serialized_data().resize(bounded_size);
    ASSERT_TRUE(none_stream_.push_submessage(session_info_, dds::xrce::WRITE_DATA, write_data));

    write_data.data().serialized_data().resize(bounded_size + 1);
    ASSERT_FALSE(none_stream_.push_submessage(session_info_, dds::xrce::WRITE_DATA, write_data));
}

/****************************************************************************************
 * Best-Effort Output Stream.
 ****************************************************************************************/
class BestEffortOutputStreamTest : public ::testing::Test
{
public:
    BestEffortOutputStreamTest()
        : best_effort_stream_{}
        , session_info_{client_key, session_id, mtu}
        , stream_id_{dds::xrce::STREAMID_BUILTIN_BEST_EFFORTS}
    {}

public:
    BestEffortOutputStream best_effort_stream_;
    SessionInfo session_info_;
    dds::xrce::StreamId stream_id_;
};

/**
 * @brief   This test checks the capacity of the stream.
 *          No more than BEST_EFFORT_STREAM_DEPTH message shall be pushed in a best-effort stream.
 */
TEST_F(BestEffortOutputStreamTest, StreamCapacity)
{
    dds::xrce::WRITE_DATA_Payload_Data write_data{};
    for (int i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_TRUE(best_effort_stream_.push_submessage(session_info_, stream_id_, dds::xrce::WRITE_DATA, write_data));
    }
    ASSERT_FALSE(best_effort_stream_.push_submessage(session_info_, stream_id_, dds::xrce::WRITE_DATA, write_data));

    OutputMessagePtr output_message;
    for (int i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_TRUE(best_effort_stream_.pop_message(output_message));
    }
    ASSERT_FALSE(best_effort_stream_.pop_message(output_message));
}

/**
 * @brief   This test checks the maximum message size of the stream.
 *          The best-effort stream shall be not able to push messages larger than the MTU.
 */
TEST_F(BestEffortOutputStreamTest, MaximumMessageSize)
{
    dds::xrce::MessageHeader header{};
    dds::xrce::SubmessageHeader subheader{};
    dds::xrce::WRITE_DATA_Payload_Data write_data{};

    size_t headers_size = header.getCdrSerializedSize() + subheader.getCdrSerializedSize();
    size_t bounded_size = mtu - headers_size - write_data.BaseObjectRequest::getCdrSerializedSize();

    write_data.data().serialized_data().resize(bounded_size);
    ASSERT_TRUE(best_effort_stream_.push_submessage(session_info_, stream_id_, dds::xrce::WRITE_DATA, write_data));

    write_data.data().serialized_data().resize(bounded_size + 1);
    ASSERT_FALSE(best_effort_stream_.push_submessage(session_info_, stream_id_, dds::xrce::WRITE_DATA, write_data));
}

/****************************************************************************************
 * Reliable Output Stream.
 ****************************************************************************************/
class ReliableOutputStreamTest : public ::testing::Test
{
public:
    ReliableOutputStreamTest()
        : reliable_stream_{}
        , session_info_{client_key, session_id, mtu}
        , stream_id_{dds::xrce::STREAMID_BUILTIN_RELIABLE}
    {}

public:
    ReliableOutputStream reliable_stream_;
    SessionInfo session_info_;
    dds::xrce::StreamId stream_id_;
};

/**
 * @brief   This test checks the capacity of the stream.
 *          No more than RELIABLE_STREAM_DEPTH message shall be pushed in a reliable stream.
 */
TEST_F(ReliableOutputStreamTest, StreamCapacity)
{
    dds::xrce::WRITE_DATA_Payload_Data write_data{};
    for (int i = 0; i < RELIABLE_STREAM_DEPTH; ++i)
    {
        ASSERT_TRUE(reliable_stream_.push_submessage(
            session_info_,
            stream_id_,
            dds::xrce::WRITE_DATA,
            write_data,
            std::chrono::milliseconds(500)));
    }
    ASSERT_FALSE(reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA,
        write_data,
        std::chrono::milliseconds(500)));

    OutputMessagePtr output_message;
    for (int i = 0; i < RELIABLE_STREAM_DEPTH; ++i)
    {
        ASSERT_TRUE(reliable_stream_.get_next_message(output_message));
    }
    ASSERT_FALSE(reliable_stream_.get_next_message(output_message));
}

/**
 * @brief   This test checks the maximum message size of the stream.
 *          The reliable stream shall be able to push messages larger than the MTU.
 */
TEST_F(ReliableOutputStreamTest, MaximumMessageSize)
{
    dds::xrce::MessageHeader header{};
    dds::xrce::SubmessageHeader subheader{};
    dds::xrce::WRITE_DATA_Payload_Data write_data{};

    const size_t headers_size = header.getCdrSerializedSize() + subheader.getCdrSerializedSize();
    const size_t bounded_size = mtu - headers_size - write_data.BaseObjectRequest::getCdrSerializedSize();

    write_data.data().serialized_data().resize(bounded_size);
    ASSERT_TRUE(reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA,
        write_data,
        std::chrono::milliseconds(500)));

    write_data.data().serialized_data().resize(bounded_size + 1);
    ASSERT_TRUE(reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA,
        write_data,
        std::chrono::milliseconds(500)));
}

/**
 * @brief   This test checks that the reliable stream fragments the messages properly.
 *          There are two border case:
 *          * Message fit:
 *              when all the fragments fit the MTU of the stream.
 *          * Message unfit:
 *              when the last fragment only contains the last byte of the message.
 */
TEST_F(ReliableOutputStreamTest, Fragmentation)
{
    dds::xrce::MessageHeader header{};
    header.session_id(session_id);
    header.client_key(client_key);
    dds::xrce::SubmessageHeader subheader{};
    dds::xrce::WRITE_DATA_Payload_Data write_data{};

    const size_t headers_size = header.getCdrSerializedSize() + subheader.getCdrSerializedSize();
    const size_t first_fragment_size =
            mtu
            - headers_size
            - subheader.getCdrSerializedSize()
            - write_data.BaseObjectRequest::getCdrSerializedSize();
    const size_t max_fragment_size =  size_t(mtu - headers_size);

    SeqNum msg_counter = 0xFFFF;

    /*
     * Message fit.
     */
    for (int i = 1; i < RELIABLE_STREAM_DEPTH; ++i)
    {
        write_data.data().serialized_data().resize(first_fragment_size + (size_t(i) * max_fragment_size));
        ASSERT_TRUE(reliable_stream_.push_submessage(
            session_info_,
            stream_id_,
            dds::xrce::WRITE_DATA,
            write_data,
            std::chrono::milliseconds(500)));

        const int n_fragments = i + 1;
        OutputMessagePtr output_message;
        for (int j = 0; j < n_fragments; ++j)
        {
            ASSERT_TRUE(reliable_stream_.get_next_message(output_message));
            msg_counter += 1;
        }
        ASSERT_FALSE(reliable_stream_.get_next_message(output_message));

        reliable_stream_.update_from_acknack(msg_counter);
    }

    /*
     * Message unfit.
     */
    for (int i = 2; i < RELIABLE_STREAM_DEPTH; ++i)
    {
        write_data.data().serialized_data().resize(first_fragment_size + (size_t(i) * max_fragment_size) + 1);
        ASSERT_TRUE(reliable_stream_.push_submessage(
            session_info_,
            stream_id_,
            dds::xrce::WRITE_DATA,
            write_data,
            std::chrono::milliseconds(500)));

        const int n_fragments = i + 2;
        OutputMessagePtr output_message;
        for (int j = 0; j < n_fragments; ++j)
        {
            ASSERT_TRUE(reliable_stream_.get_next_message(output_message));
            msg_counter += 1;
        }
        ASSERT_FALSE(reliable_stream_.get_next_message(output_message));

        reliable_stream_.update_from_acknack(msg_counter);
    }
}

/**
 * @brief   This test checks the initial conditions of the reliable stream.
 */
TEST_F(ReliableOutputStreamTest, InitialCondition)
{
    SeqNum expected_first_unacked = 0x0000;
    SeqNum expected_last_unacked = 0xFFFF;
    dds::xrce::HEARTBEAT_Payload hearbeat;

    reliable_stream_.fill_heartbeat(hearbeat);
    ASSERT_EQ(hearbeat.first_unacked_seq_nr(), expected_first_unacked);
    ASSERT_EQ(hearbeat.last_unacked_seq_nr(), expected_last_unacked);
}

/**
 * @brief   This test checks that the reliable stream is promoted properly when messages are pushed.
 *          The last_unacked shall increase by one for each pushed message.
 */
TEST_F(ReliableOutputStreamTest, PushMessages)
{
    SeqNum expected_first_unacked = 0x0000;
    SeqNum expected_last_unacked = 0xFFFF;
    dds::xrce::HEARTBEAT_Payload hearbeat;

    dds::xrce::WRITE_DATA_Payload_Data write_data{};
    ASSERT_TRUE(reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA, write_data,
        std::chrono::milliseconds(500)));
    expected_last_unacked += 1;

    reliable_stream_.fill_heartbeat(hearbeat);
    ASSERT_EQ(hearbeat.first_unacked_seq_nr(), expected_first_unacked);
    ASSERT_EQ(hearbeat.last_unacked_seq_nr(), expected_last_unacked);

    ASSERT_TRUE(reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA, write_data,
        std::chrono::milliseconds(500)));
    expected_last_unacked += 1;

    reliable_stream_.fill_heartbeat(hearbeat);
    ASSERT_EQ(hearbeat.first_unacked_seq_nr(), expected_first_unacked);
    ASSERT_EQ(hearbeat.last_unacked_seq_nr(), expected_last_unacked);
}

/**
 * @brief   This test checks that the reliable stream is promoted properly when messages are popped.
 *          The first_unacked shall increase by one for each popped message.
 */
TEST_F(ReliableOutputStreamTest, GetMessages)
{
    SeqNum expected_first_unacked = 0x0000;
    SeqNum expected_last_unacked = 0xFFFF;
    dds::xrce::HEARTBEAT_Payload hearbeat;

    dds::xrce::WRITE_DATA_Payload_Data write_data{};
    reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA,
        write_data,
        std::chrono::milliseconds(500));
    reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA,
        write_data,
        std::chrono::milliseconds(500));
    expected_last_unacked += 2;

    OutputMessagePtr output_message;
    ASSERT_TRUE(reliable_stream_.get_next_message(output_message));
    ASSERT_TRUE(reliable_stream_.get_next_message(output_message));
    ASSERT_FALSE(reliable_stream_.get_next_message(output_message));

    reliable_stream_.fill_heartbeat(hearbeat);
    ASSERT_EQ(hearbeat.first_unacked_seq_nr(), expected_first_unacked);
    ASSERT_EQ(hearbeat.last_unacked_seq_nr(), expected_last_unacked);
}

/**
 * @brief   This test checks that the reliable stream is update properly from acknacks.
 *          There are two border cases:
 *          * Lower border case:
 *                when the first_unacked of the acknack is less or equal than the first_unacked of the stream,
 *                the acknack shall not have effect.
 *          * Upper border case:
 *                when the first_unacked of the acknack is greater than the last_sent + 1 of the stream,
 *                the acknack shall not have effect.
 */
TEST_F(ReliableOutputStreamTest, UpdateFromAcknack)
{
    SeqNum expected_first_unacked = 0x0000;
    SeqNum expected_last_unacked = 0xFFFF;
    SeqNum last_sent = 0xFFFF;
    dds::xrce::HEARTBEAT_Payload hearbeat;

    dds::xrce::WRITE_DATA_Payload_Data write_data{};
    reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA,
        write_data,
        std::chrono::milliseconds(500));

    reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA,
        write_data,
        std::chrono::milliseconds(500));

    reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA,
        write_data,
        std::chrono::milliseconds(500));

    reliable_stream_.push_submessage(
        session_info_,
        stream_id_,
        dds::xrce::WRITE_DATA,
        write_data,
        std::chrono::milliseconds(500));

    expected_last_unacked += 4;

    OutputMessagePtr output_message;
    reliable_stream_.get_next_message(output_message);
    reliable_stream_.get_next_message(output_message);
    reliable_stream_.get_next_message(output_message);
    last_sent += 3;

    /*
     * Lower border case.
     */
    reliable_stream_.update_from_acknack(expected_first_unacked);
    reliable_stream_.fill_heartbeat(hearbeat);
    ASSERT_EQ(hearbeat.first_unacked_seq_nr(), expected_first_unacked);
    ASSERT_EQ(hearbeat.last_unacked_seq_nr(), expected_last_unacked);

    reliable_stream_.update_from_acknack(expected_first_unacked + 1);
    expected_first_unacked += 1;
    reliable_stream_.fill_heartbeat(hearbeat);
    ASSERT_EQ(hearbeat.first_unacked_seq_nr(), expected_first_unacked);
    ASSERT_EQ(hearbeat.last_unacked_seq_nr(), expected_last_unacked);

    /*
     * Upper border case.
     */
    reliable_stream_.update_from_acknack(last_sent + 2);
    reliable_stream_.fill_heartbeat(hearbeat);
    ASSERT_EQ(hearbeat.first_unacked_seq_nr(), expected_first_unacked);
    ASSERT_EQ(hearbeat.last_unacked_seq_nr(), expected_last_unacked);

    reliable_stream_.update_from_acknack(last_sent + 1);
    expected_first_unacked = last_sent + 1;
    reliable_stream_.fill_heartbeat(hearbeat);
    ASSERT_EQ(hearbeat.first_unacked_seq_nr(), expected_first_unacked);
    ASSERT_EQ(hearbeat.last_unacked_seq_nr(), expected_last_unacked);
}

} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
