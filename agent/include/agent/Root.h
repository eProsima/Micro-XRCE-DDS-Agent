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

#ifndef _ROOT_H
#define _ROOT_H

#include <agent/ObjectVariant.h>
#include <agent/client/ProxyClient.h>
#include <agent/XRCEFactory.h>
#include <agent/XRCEParser.h>

#include <transport/ddsxrce_transport.h>

#include <map>

namespace eprosima{
namespace micrortps{

class Agent;
Agent& root();

class Agent : public XRCEListener
{
public:
    Agent();
    ~Agent() = default;

    void init();

    Status create_client(int32_t client_key,  const ObjectVariant& client_representation);
    Status delete_client(int32_t client_key);

    void run();

    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const CREATE_PAYLOAD&      create_payload) override;
    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const DELETE_PAYLOAD&      delete_payload) override;
    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const WRITE_DATA_PAYLOAD&  write_payload)  override;
    void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const READ_DATA_PAYLOAD&   read_payload)   override;

    ProxyClient* get_client(int32_t client_key);

private:
    channel_id_t ch_id;
    static const size_t buffer_len = 1024;
    octet out_buffer[buffer_len];
    octet in_buffer[buffer_len];
    locator_t loc;

    std::map<int32_t, ProxyClient> clients_;
};

} // eprosima
} // micrortps

#endif //_ROOT_H