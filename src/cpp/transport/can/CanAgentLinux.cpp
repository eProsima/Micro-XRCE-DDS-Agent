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

#include <uxr/agent/transport/can/CanAgentLinux.hpp>
#include <uxr/agent/utils/Conversion.hpp>
#include <uxr/agent/logger/Logger.hpp>

#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

namespace eprosima {
namespace uxr {

CanAgent::CanAgent(
        char const* dev,
        uint32_t can_id,
        Middleware::Kind middleware_kind)
    : Server<CanEndPoint>{middleware_kind}
    , dev_{dev}
    , can_id_{can_id}
{
}

CanAgent::~CanAgent()
{
    try
    {
        stop();
    }
    catch (std::exception& e)
    {
        UXR_AGENT_LOG_CRITICAL(
            UXR_DECORATE_RED("error stopping server"),
            "exception: {}",
            e.what());
    }
}

bool CanAgent::init()
{
    static int enable_canfd = 1;
    bool rv = false;

    poll_fd_.fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (-1 != poll_fd_.fd)
    {
        struct sockaddr_can address {};
        struct ifreq ifr;

        // Get interface index by name
        strcpy(ifr.ifr_name, dev_.c_str());
        ioctl(poll_fd_.fd, SIOCGIFINDEX, &ifr);

        memset(&address, 0, sizeof(address));
        address.can_family = AF_CAN;
        address.can_ifindex = ifr.ifr_ifindex;

        if (-1 != bind(poll_fd_.fd,
                reinterpret_cast<struct sockaddr*>(&address),
                sizeof(address)))
        {
            // Enable CAN FD
            if (-1 != setsockopt(poll_fd_.fd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES,
                    &enable_canfd, sizeof(enable_canfd)))
            {
                poll_fd_.events = POLLIN;
                rv = true;

                UXR_AGENT_LOG_INFO(
                    UXR_DECORATE_GREEN("running..."),
                    "device: {}, fd: {}",
                    dev_, poll_fd_.fd);


                // TODO: add filter for micro-ROS devices
            }
            else
            {
                UXR_AGENT_LOG_ERROR(
                    UXR_DECORATE_RED("Enable CAN FD failed"),
                    "device: {},errno: {}",
                    dev_, errno);
            }
        }
        else
        {
            UXR_AGENT_LOG_ERROR(
                UXR_DECORATE_RED("SocketCan bind error"),
                "device: {}, errno: {}",
                dev_, errno);
        }
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("SocketCan error"),
            "errno: {}",
            errno);
    }

    return rv;
}

bool CanAgent::fini()
{
    if (-1 == poll_fd_.fd)
    {
        return true;
    }

    bool rv = false;
    if (0 == ::close(poll_fd_.fd))
    {
        UXR_AGENT_LOG_INFO(
            UXR_DECORATE_GREEN("server stopped"),
            "fd: {}, device: {}",
            poll_fd_.fd, dev_);
        rv = true;
    }
    else
    {
        UXR_AGENT_LOG_ERROR(
            UXR_DECORATE_RED("close server error"),
            "fd: {}, device: {}, errno: {}",
            poll_fd_.fd, dev_, errno);
    }

    poll_fd_.fd = -1;
    return rv;
}

bool CanAgent::recv_message(
        InputPacket<CanEndPoint>& input_packet,
        int timeout,
        TransportRc& transport_rc)
{
    bool rv = false;
    struct canfd_frame frame = {};

    int poll_rv = poll(&poll_fd_, 1, timeout);

    if (0 < poll_rv)
    {
        if (0 < read(poll_fd_.fd, &frame, sizeof(struct canfd_frame)))
        {
            // Omit EFF, RTR, ERR flags (Assume EFF on CAN FD)
            uint32_t can_id = frame.can_id & CAN_ERR_MASK;
            size_t len = frame.data[0];   // XRCE payload lenght

            if (len > (CANFD_MTU - 1))
            {
                // Overflow MTU (63 bytes)
                return false;
            }

            input_packet.message.reset(new InputMessage(&frame.data[1], len));
            input_packet.source = CanEndPoint(can_id);
            rv = true;

            uint32_t raw_client_key;
            if (Server<CanEndPoint>::get_client_key(input_packet.source, raw_client_key))
            {
                UXR_AGENT_LOG_MESSAGE(
                    UXR_DECORATE_YELLOW("[==>> CAN <<==]"),
                    raw_client_key,
                    input_packet.message->get_buf(),
                    input_packet.message->get_len());
            }
        }
        else
        {
            transport_rc = TransportRc::server_error;
        }
    }
    else
    {
        transport_rc = (poll_rv == 0) ? TransportRc::timeout_error : TransportRc::server_error;
    }

    return rv;
}

bool CanAgent::send_message(
        OutputPacket<CanEndPoint> output_packet,
        TransportRc& transport_rc)
{
    bool rv = false;
    struct canfd_frame frame = {};
    struct pollfd poll_fd_write_;
    size_t packet_len = output_packet.message->get_len();

    if (packet_len > (CANFD_MTU - 1))
    {
        // Overflow MTU (63 bytes)
        return 0;
    }

    poll_fd_write_.fd = poll_fd_.fd;
    poll_fd_write_.events = POLLOUT;
    int poll_rv = poll(&poll_fd_write_, 1, 0);

    if (0 < poll_rv)
    {
        frame.can_id = output_packet.destination.get_can_id() | CAN_EFF_FLAG;
        frame.data[0] = (uint8_t) packet_len;   // XRCE payload lenght
        frame.len = (uint8_t) (packet_len + 1);   // CAN frame DLC

        memcpy(&frame.data[1], output_packet.message->get_buf(), packet_len);

        if (0 < ::write(poll_fd_.fd, &frame, sizeof(struct canfd_frame)))
        {
            rv = true;

            uint32_t raw_client_key;
            if (Server<CanEndPoint>::get_client_key(output_packet.destination, raw_client_key))
            {
                UXR_AGENT_LOG_MESSAGE(
                    UXR_DECORATE_YELLOW("[** <<CAN>> **]"),
                    raw_client_key,
                    output_packet.message->get_buf(),
                    packet_len);
            }
        }
        else
        {
            // Write failed
            transport_rc = TransportRc::server_error;
        }
    }
    else
    {
        // Can device is busy
        transport_rc = TransportRc::server_error;
    }

    return rv;
}

bool CanAgent::handle_error(
        TransportRc /*transport_rc*/)
{
    return fini() && init();
}

} // namespace uxr
} // namespace eprosima
