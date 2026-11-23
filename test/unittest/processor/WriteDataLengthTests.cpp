// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifdef _WIN32
#include <uxr/agent/transport/udp/UDPv4AgentWindows.hpp>
#else
#include <uxr/agent/transport/udp/UDPv4AgentLinux.hpp>
#endif // ifdef _WIN32
#include <uxr/agent/middleware/Middleware.hpp>

#include <gtest/gtest.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif // ifdef _WIN32

#include <cstdint>
#include <memory>
#include <vector>

namespace eprosima {
namespace uxr {
namespace testing {

using namespace eprosima::uxr;

// Wrappers over the BSD socket calls that differ between POSIX and Winsock.
#ifdef _WIN32
using socket_t = SOCKET;
static const socket_t invalid_socket = INVALID_SOCKET;
#else
using socket_t = int;
static const socket_t invalid_socket = -1;
#endif // ifdef _WIN32

static bool set_receive_timeout(
        socket_t socket_fd,
        int seconds)
{
#ifdef _WIN32
    DWORD timeout = static_cast<DWORD>(seconds) * 1000;
    return 0 == setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&timeout), static_cast<int>(sizeof(timeout)));
#else
    struct timeval timeout = {seconds, 0};
    return 0 == setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif // ifdef _WIN32
}

static bool send_datagram(
        socket_t socket_fd,
        const std::vector<uint8_t>& packet,
        const struct sockaddr_in& addr)
{
#ifdef _WIN32
    int sent = sendto(socket_fd, reinterpret_cast<const char*>(packet.data()), static_cast<int>(packet.size()), 0,
                    reinterpret_cast<const struct sockaddr*>(&addr), static_cast<int>(sizeof(addr)));
    return static_cast<int>(packet.size()) == sent;
#else
    ssize_t sent = sendto(socket_fd, packet.data(), packet.size(), 0,
                    reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr));
    return static_cast<ssize_t>(packet.size()) == sent;
#endif // ifdef _WIN32
}

static bool receive_datagram(
        socket_t socket_fd)
{
    char reply[512];
#ifdef _WIN32
    return 0 < recv(socket_fd, reply, static_cast<int>(sizeof(reply)), 0);
#else
    return 0 < recv(socket_fd, reply, sizeof(reply), 0);
#endif // ifdef _WIN32
}

static void close_socket(
        socket_t socket_fd)
{
#ifdef _WIN32
    closesocket(socket_fd);
#else
    close(socket_fd);
#endif // ifdef _WIN32
}

/*
 * A WRITE_DATA submessage (FORMAT_DATA) whose effective length is smaller than its
 * BaseObjectRequest (4 bytes) must be dropped. It must not terminate the Agent.
 */
class WriteDataLengthTests : public ::testing::Test
{
protected:

    static constexpr uint16_t agent_port_ = 38027;

    void SetUp() override
    {
#ifdef _WIN32
        // The Agent does not initialize Winsock itself, WSAStartup is reference counted.
        WSADATA wsa_data;
        ASSERT_EQ(0, WSAStartup(MAKEWORD(2, 2), &wsa_data));
        winsock_initialized_ = true;
#endif // ifdef _WIN32

        agent_.reset(new UDPv4Agent(agent_port_, Middleware::Kind::FASTDDS));
        agent_->set_verbose_level(6);
        ASSERT_TRUE(agent_->start());

        socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        ASSERT_NE(invalid_socket, socket_fd_);
        ASSERT_TRUE(set_receive_timeout(socket_fd_, 2));

        agent_addr_.sin_family = AF_INET;
        agent_addr_.sin_port = htons(agent_port_);
        agent_addr_.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }

    void TearDown() override
    {
        if (invalid_socket != socket_fd_)
        {
            close_socket(socket_fd_);
        }
        if (agent_)
        {
            agent_->stop();
        }
#ifdef _WIN32
        if (winsock_initialized_)
        {
            WSACleanup();
        }
#endif // ifdef _WIN32
    }

    void send_packet(
            const std::vector<uint8_t>& packet)
    {
        ASSERT_TRUE(send_datagram(socket_fd_, packet, agent_addr_));
    }

    /*
     * Sends a CREATE_CLIENT and returns whether the Agent answered it.
     * Used both to establish the session and to check that the Agent is still alive.
     */
    bool create_client(
            uint8_t session_id)
    {
        std::vector<uint8_t> packet = {
            0x00, 0x00, 0x00, 0x00,     // MessageHeader: SESSIONID_NONE_WITH_CLIENT_KEY, stream 0, seq 0
            0xAA, 0xBB, 0xCC, 0xDD,     // client_key
            0x00, 0x01, 0x10, 0x00,     // SubmessageHeader: CREATE_CLIENT, little endian, length 16
            0x58, 0x52, 0x43, 0x45,     // xrce_cookie "XRCE"
            0x01, 0x00,                 // xrce_version 1.0
            0x0F, 0x0F,                 // xrce_vendor_id
            0xAA, 0xBB, 0xCC, 0xDD,     // client_key
            session_id,                 // session_id
            0x00,                       // properties: absent
            0x00, 0x02                  // mtu = 512
        };
        send_packet(packet);

        return receive_datagram(socket_fd_);
    }

    /*
     * Builds a FORMAT_DATA WRITE_DATA submessage that declares `declared_length` and carries
     * `payload_length` payload bytes.
     */
    std::vector<uint8_t> write_data(
            uint8_t session_id,
            uint16_t declared_length,
            uint16_t payload_length)
    {
        std::vector<uint8_t> packet = {session_id, 0x00, 0x00, 0x00};   // stream 0, seq 0
        if (128 > session_id)
        {
            packet.insert(packet.end(), {0xAA, 0xBB, 0xCC, 0xDD});      // client_key
        }
        packet.push_back(0x07);                                         // WRITE_DATA
        packet.push_back(0x01);                                         // little endian, FORMAT_DATA
        packet.push_back(static_cast<uint8_t>(declared_length & 0xFF)); // submessage_length
        packet.push_back(static_cast<uint8_t>(declared_length >> 8));
        packet.insert(packet.end(), payload_length, 0x00);
        return packet;
    }

    /*
     * Establishes a session, sends the WRITE_DATA and checks that the Agent still answers.
     * session_id >= 128 carries no client_key on the wire: the client is resolved from the source address.
     */
    void check_agent_survives(
            uint8_t session_id,
            uint16_t declared_length,
            uint16_t payload_length)
    {
        ASSERT_TRUE(create_client(session_id));
        send_packet(write_data(session_id, declared_length, payload_length));
        EXPECT_TRUE(create_client(session_id));
    }

    // The submessage declares `length` and carries exactly `length` payload bytes.
    void check_agent_survives(
            uint8_t session_id,
            uint16_t length)
    {
        check_agent_survives(session_id, length, length);
    }

    static constexpr uint8_t keyed_session_ = 0x01;
    static constexpr uint8_t keyless_session_ = 0x81;

    std::unique_ptr<UDPv4Agent> agent_;
    socket_t socket_fd_ = invalid_socket;
    struct sockaddr_in agent_addr_ = {};
#ifdef _WIN32
    bool winsock_initialized_ = false;
#endif // ifdef _WIN32
};

constexpr uint16_t WriteDataLengthTests::agent_port_;
constexpr uint8_t WriteDataLengthTests::keyed_session_;
constexpr uint8_t WriteDataLengthTests::keyless_session_;

// Lengths 0-3 are shorter than the BaseObjectRequest; 4 is the boundary (empty sample).
// With UAGENT_TWEAK_XRCE_WRITE_LIMIT, a declared length of 0 is recomputed from the datagram,
// which also yields 0 here because no payload follows the subheader.

TEST_F(WriteDataLengthTests, KeyedSessionLength0)
{
    check_agent_survives(keyed_session_, 0);
}

TEST_F(WriteDataLengthTests, KeyedSessionLength1)
{
    check_agent_survives(keyed_session_, 1);
}

TEST_F(WriteDataLengthTests, KeyedSessionLength2)
{
    check_agent_survives(keyed_session_, 2);
}

TEST_F(WriteDataLengthTests, KeyedSessionLength3)
{
    check_agent_survives(keyed_session_, 3);
}

TEST_F(WriteDataLengthTests, KeyedSessionLength4)
{
    check_agent_survives(keyed_session_, 4);
}

TEST_F(WriteDataLengthTests, KeylessSessionLength0)
{
    check_agent_survives(keyless_session_, 0);
}

TEST_F(WriteDataLengthTests, KeylessSessionLength1)
{
    check_agent_survives(keyless_session_, 1);
}

TEST_F(WriteDataLengthTests, KeylessSessionLength2)
{
    check_agent_survives(keyless_session_, 2);
}

TEST_F(WriteDataLengthTests, KeylessSessionLength3)
{
    check_agent_survives(keyless_session_, 3);
}

TEST_F(WriteDataLengthTests, KeylessSessionLength4)
{
    check_agent_survives(keyless_session_, 4);
}

// A declared length of 0 followed by trailing bytes. With UAGENT_TWEAK_XRCE_WRITE_LIMIT the effective
// length is recomputed from the datagram and equals the number of trailing bytes, so 1-3 are shorter
// than the BaseObjectRequest and 4 is the boundary. Without the tweak the effective length stays 0.

TEST_F(WriteDataLengthTests, KeyedSessionDeclared0Trailing1)
{
    check_agent_survives(keyed_session_, 0, 1);
}

TEST_F(WriteDataLengthTests, KeyedSessionDeclared0Trailing2)
{
    check_agent_survives(keyed_session_, 0, 2);
}

TEST_F(WriteDataLengthTests, KeyedSessionDeclared0Trailing3)
{
    check_agent_survives(keyed_session_, 0, 3);
}

TEST_F(WriteDataLengthTests, KeyedSessionDeclared0Trailing4)
{
    check_agent_survives(keyed_session_, 0, 4);
}

TEST_F(WriteDataLengthTests, KeylessSessionDeclared0Trailing1)
{
    check_agent_survives(keyless_session_, 0, 1);
}

TEST_F(WriteDataLengthTests, KeylessSessionDeclared0Trailing2)
{
    check_agent_survives(keyless_session_, 0, 2);
}

TEST_F(WriteDataLengthTests, KeylessSessionDeclared0Trailing3)
{
    check_agent_survives(keyless_session_, 0, 3);
}

TEST_F(WriteDataLengthTests, KeylessSessionDeclared0Trailing4)
{
    check_agent_survives(keyless_session_, 0, 4);
}

} // namespace testing
} // namespace uxr
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
