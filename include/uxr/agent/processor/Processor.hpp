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

#ifndef UXR_AGENT_PROCESSOR_PROCESSOR_HPP_
#define UXR_AGENT_PROCESSOR_PROCESSOR_HPP_

#include <uxr/agent/middleware/Middleware.hpp>

#include <cstdint>
#include <vector>
#include <mutex>

namespace dds {
namespace xrce {

class TransportAddress;

}
}

namespace eprosima {
namespace uxr {

class Root;
class Server;
class ProxyClient;
struct InputPacket;
struct OutputPacket;
struct ReadCallbackArgs;

class Processor
{
public:
    Processor(
            Server& server,
            Root& root,
            Middleware::Kind middleware_kind);

    ~Processor();

    void process_input_packet(
            InputPacket&& input_packet);

    bool process_get_info_packet(
            InputPacket&& input_packet,
            dds::xrce::TransportAddress& address,
            OutputPacket& output_packet) const;

    void check_heartbeats();

private:
    void process_input_message(
            ProxyClient& client,
            InputPacket& input_packet);

    bool process_submessage(
            ProxyClient& client,
            InputPacket& input_packet);

    bool process_create_client_submessage(
            InputPacket& input_packet);

    bool process_create_submessage(
            ProxyClient& client,
            InputPacket& input_packet);

    bool process_delete_submessage(
            ProxyClient& client,
            InputPacket& input_packet);

    bool process_write_data_submessage(
            ProxyClient& client,
            InputPacket& input_packet);

    bool process_read_data_submessage(
            ProxyClient& client,
            InputPacket& input_packet);

    bool process_acknack_submessage(
            ProxyClient& client,
            InputPacket& input_packet);

    bool process_heartbeat_submessage(
            ProxyClient& client,
            InputPacket& input_packet);

    bool process_reset_submessage(
            ProxyClient& client,
            InputPacket&);

    bool process_fragment_submessage(
            ProxyClient& client,
            InputPacket& input_packet);

    bool process_timestamp_submessage(
            ProxyClient& client,
            InputPacket& input_packet);

//    bool process_performance_submessage(
//            ProxyClient& client,
//            InputPacket& input_packet);

    void read_data_callback(
            const ReadCallbackArgs& cb_args,
            const std::vector<uint8_t>& buffer);

private:
    Server& server_;
    Middleware::Kind middleware_kind_;
    Root& root_;
};

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_PROCESSOR_PROCESSOR_HPP_
