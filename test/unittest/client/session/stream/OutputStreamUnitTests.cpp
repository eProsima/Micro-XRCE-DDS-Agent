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

const dds::xrce::SessionId session_id = 0x01;
const dds::xrce::ClientKey client_key = {0xAA, 0xBB, 0xCC, 0xDD};
const size_t mtu = 512;

/****************************************************************************************
 * None Output Stream.
 ****************************************************************************************/
class NoneOutputStreamUnitTests : public ::testing::Test
{
public:
    NoneOutputStreamUnitTests()
        : none_stream_(session, session_id, client_key, mtu) {}

public:
    Session session_;
    NoneOutputStream none_stream_;
};

TEST_F(NoneOutputStreamUnitTests, StreamCapacity)
{
    dds::xrce::WRITE_DATA_Payload_Data write_data_payload{};
    for (int i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_EQ(none_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), true);
    }
    ASSERT_EQ(none_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), false);

    OutputMessagePtr output_message;
    for (int i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_EQ(none_stream_.get_next_message(output_message), true);
    }
    ASSERT_EQ(none_stream_.get_next_message(output_message), false);
}

TEST_F(NoneOutputStreamUnitTests, MessageCapacity)
{
    dds::xrce::MessageHeader message_header{};
    dds::xrce::SubmessageHeader submessage_header{};
    dds::xrce::WRITE_DATA_Payload_Data write_data_payload{};

    size_t headers_size = message_header.getCdrSerializedSize() + submessage_header.getCdrSerializedSize();
    size_t bounded_size = mtu - headers_size - write_data_payload.BaseObjectRequest::getCdrSerializedSize();

    write_data_payload.data().serialized_data().resize(bounded_size);
    ASSERT_EQ(none_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), true);

    write_data_payload.data().serialized_data().resize(bounded_size + 1);
    ASSERT_EQ(none_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), false);
}

/****************************************************************************************
 * Best-Effort Output Stream.
 ****************************************************************************************/
class BestEffortOutputStreamUnitTests : public ::testing::Test
{
public:
    BestEffortOutputStreamUnitTests()
        : best_effort_stream_(session_id, 0x01, client_key, mtu) {}

public:
    BestEffortOutputStream best_effort_stream_;
};

TEST_F(BestEffortOutputStreamUnitTests, StreamCapacity)
{
    dds::xrce::WRITE_DATA_Payload_Data write_data_payload{};
    for (int i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_EQ(best_effort_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), true);
    }
    ASSERT_EQ(best_effort_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), false);

    OutputMessagePtr output_message;
    for (int i = 0; i < BEST_EFFORT_STREAM_DEPTH; ++i)
    {
        ASSERT_EQ(best_effort_stream_.get_next_message(output_message), true);
    }
    ASSERT_EQ(best_effort_stream_.get_next_message(output_message), false);
}

TEST_F(BestEffortOutputStreamUnitTests, MessageCapacity)
{
    dds::xrce::MessageHeader message_header{};
    dds::xrce::SubmessageHeader submessage_header{};
    dds::xrce::WRITE_DATA_Payload_Data write_data_payload{};

    size_t headers_size = message_header.getCdrSerializedSize() + submessage_header.getCdrSerializedSize();
    size_t bounded_size = mtu - headers_size - write_data_payload.BaseObjectRequest::getCdrSerializedSize();

    write_data_payload.data().serialized_data().resize(bounded_size);
    ASSERT_EQ(best_effort_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), true);

    write_data_payload.data().serialized_data().resize(bounded_size + 1);
    ASSERT_EQ(best_effort_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), false);
}

/****************************************************************************************
 * Reliable Output Stream.
 ****************************************************************************************/
class ReliableOutputStreamUnitTests : public ::testing::Test
{
public:
    ReliableOutputStreamUnitTests()
        : reliable_stream_(session_id, 0x01, client_key, mtu) {}

public:
    ReliableOutputStream reliable_stream_;
};

TEST_F(ReliableOutputStreamUnitTests, StreamCapacity)
{
    dds::xrce::WRITE_DATA_Payload_Data write_data_payload{};
    for (int i = 0; i < RELIABLE_STREAM_DEPTH; ++i)
    {
        ASSERT_EQ(reliable_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), true);
    }
    ASSERT_EQ(reliable_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), false);

    OutputMessagePtr output_message;
    for (int i = 0; i < RELIABLE_STREAM_DEPTH; ++i)
    {
        ASSERT_EQ(reliable_stream_.get_next_message(output_message), true);
    }
    ASSERT_EQ(reliable_stream_.get_next_message(output_message), false);
}

TEST_F(ReliableOutputStreamUnitTests, MessageCapacity)
{
    dds::xrce::MessageHeader message_header{};
    dds::xrce::SubmessageHeader submessage_header{};
    dds::xrce::WRITE_DATA_Payload_Data write_data_payload{};

    const size_t headers_size = message_header.getCdrSerializedSize() + submessage_header.getCdrSerializedSize();
    const size_t bounded_size = mtu - headers_size - write_data_payload.BaseObjectRequest::getCdrSerializedSize();

    write_data_payload.data().serialized_data().resize(bounded_size);
    ASSERT_EQ(reliable_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), true);

    write_data_payload.data().serialized_data().resize(bounded_size + 1);
    ASSERT_EQ(reliable_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), true);
}

TEST_F(ReliableOutputStreamUnitTests, FragmentedMessages)
{
    dds::xrce::MessageHeader message_header{};
    message_header.session_id(session_id);
    message_header.client_key(client_key);
    dds::xrce::SubmessageHeader submessage_header{};
    dds::xrce::WRITE_DATA_Payload_Data write_data_payload{};

    const size_t headers_size = message_header.getCdrSerializedSize() + submessage_header.getCdrSerializedSize();
    const size_t first_fragment_size =
            mtu
            - headers_size
            - submessage_header.getCdrSerializedSize()
            - write_data_payload.BaseObjectRequest::getCdrSerializedSize();
    const size_t max_fragment_size =  size_t(mtu - headers_size);

    SeqNum msg_counter = 0xFFFF;
    for (int i = 1; i < RELIABLE_STREAM_DEPTH; ++i)
    {
        write_data_payload.data().serialized_data().resize(first_fragment_size + (size_t(i) * max_fragment_size));
        ASSERT_EQ(reliable_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload), true);

        const int n_fragments = i + 1;
        OutputMessagePtr output_message;
        for (int j = 0; j < n_fragments; ++j)
        {
            ASSERT_EQ(reliable_stream_.get_next_message(output_message), true);
            msg_counter += 1;
        }
        ASSERT_EQ(reliable_stream_.get_next_message(output_message), false);

        reliable_stream_.update_from_acknack(msg_counter);
    }
}

TEST_F(ReliableOutputStreamUnitTests, EmptyStream)
{
    SeqNum expected_first_available = 0x0000;
    SeqNum expected_last_available = 0xFFFF;
    SeqNum expected_next_message = 0x0000;

    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);

    reliable_stream_.update_from_acknack(0xFFFF);
    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);

    reliable_stream_.update_from_acknack(0x0001);
    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);

    reliable_stream_.update_from_acknack(0x0000);
    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);
}

TEST_F(ReliableOutputStreamUnitTests, UpdateFromAcknack)
{
    SeqNum expected_first_available = 0x0000;
    SeqNum expected_last_available = 0xFFFF;
    SeqNum expected_next_message = 0x0000;

    reliable_stream_.update_from_acknack(0x0000);
    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);

    reliable_stream_.update_from_acknack(0xFFFF);
    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);

    reliable_stream_.update_from_acknack(0x0001);
    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);

    dds::xrce::WRITE_DATA_Payload_Data write_data_payload{};

    for (int i = 0; i < RELIABLE_STREAM_DEPTH; ++i)
    {
        if (reliable_stream_.push_submessage(dds::xrce::WRITE_DATA, write_data_payload))
        {
            expected_last_available += 1;
        }
        ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
        ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
        ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);
    }

    for (int i = 0; i < RELIABLE_STREAM_DEPTH - 2; ++i)
    {
        OutputMessagePtr output_message;
        if (reliable_stream_.get_next_message(output_message))
        {
            expected_next_message += 1;
        }
        ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
        ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
        ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);
    }

    int first_unacked = expected_first_available;
    reliable_stream_.update_from_acknack(first_unacked);
    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);

    first_unacked = expected_next_message + 1;
    reliable_stream_.update_from_acknack(first_unacked);
    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);

    first_unacked = expected_first_available + 1;
    expected_first_available += 1;
    reliable_stream_.update_from_acknack(first_unacked);
    ASSERT_EQ(reliable_stream_.get_first_available(), expected_first_available);
    ASSERT_EQ(reliable_stream_.get_last_available(), expected_last_available);
    ASSERT_EQ(reliable_stream_.next_message(), expected_next_message);
}

} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(int args, char** argv)
{
    ::testing::InitGoogleTest(&args, argv);
    return RUN_ALL_TESTS();
}
