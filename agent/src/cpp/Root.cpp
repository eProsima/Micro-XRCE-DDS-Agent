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

#include "agent/Root.h"

#include "agent/Payloads.h"
#include "agent/MessageHeader.h"

using namespace eprosima::micrortps;

Agent::Agent() {}

Agent& root()
{
    static Agent xrce_agent;
    return xrce_agent;
}

Status Agent::create_client(int32_t client_key,  const ObjectVariant& client_representation)
{
    Status status;
    status.result().request_id();
    status.result().status(STATUS_LAST_OP_CREATE);
    status.object_id(); // TODO que object id?

    if ((client_representation.discriminator() == OBJK_CLIENT) && 
        (client_representation.client().xrce_cookie() == std::array<uint8_t, 4>XRCE_COOKIE))
    {
        if (client_representation.client().xrce_version()[0] <= XRCE_VERSION_MAJOR)
        {
            // TODO The Agent shall check the ClientKey to ensure it is authorized to connect to the Agent
            // If this check fails the operation shall fail and returnValue is set to {STATUS_LAST_OP_CREATE,STATUS_ERR_DENIED}.
            //TODO Check if there is an existing DdsXrce::ProxyClient object associated with the same ClientKey and if
            // so compare the session_id of the existing ProxyClient with the one in the object_representation:
            // o If a ProxyClient exists and has the same session_id then the operation shall not perform any action
            // and shall set the returnValue to {STATUS_LAST_OP_CREATE,STATUS_OK}.
            // o If a ProxyClient exist and has a different session_id then the operation shall delete the existing
            // DdsXrce::ProxyClient object and shall proceed as if the ProxyClient did not exist.
            // Check there are sufficient internal resources to complete the create operation. If there are not, then the operation
            // shall fail and set the returnValue to {STATUS_LAST_OP_CREATE, STATUS_ERR_RESOURCES}.
            clients_[client_key] = ProxyClient{client_representation.client()};
            status.result().implementation_status(STATUS_OK);
        }
        else{
            status.result().implementation_status(STATUS_ERR_INCOMPATIBLE);
        }
    }
    else
    {        
        status.result().implementation_status(STATUS_ERR_INVALID_DATA);
    }
    return status;
}

Status Agent::delete_client(int32_t client_key)
{
    Status status;
    status.result().request_id();
    status.result().status(STATUS_LAST_OP_DELETE);
    if (0 == clients_.erase(client_key))
    {
        status.result().implementation_status(STATUS_ERR_INVALID_DATA);
        // TODO en el documento se menciona STATUS_ERR_INVALID pero no existe.
    }
    else
    {
        status.result().implementation_status(STATUS_OK);
    }
    return status;
}

void Agent::init()
{
    // Init transport
    loc = locator_t{LOC_SERIAL, "/dev/ttyACM0"};
    ch_id = add_locator(&loc);
}

void Agent::run()
{
    int loops = 1000;
    size_t ret = 0;
    while (loops--)
    {
        if (0 < (ret = send(out_buffer, 0, loc.kind, ch_id)))
        {
            printf("SEND: %d bytes\n", ret);
        }
        else
        {
            printf("SEND ERROR: %d\n", ret);
        }


        if (0 < (ret = receive(in_buffer, buffer_len, loc.kind, ch_id)))
        {
            printf("RECV: %d bytes\n", ret);
            XRCEParser myParser{reinterpret_cast<char*>(in_buffer), ret, this};
            myParser.parse();
        }
        else
        {
            printf("RECV ERROR: %d\n", ret);
        }


    }
}

void Agent::on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const CREATE_PAYLOAD& create_payload)
{
    if (ProxyClient* client = get_client(header.client_key()))
    {
        CreationMode creation_mode;
        creation_mode.reuse(false);
        creation_mode.replace(true);

        // TODO get sub_header flags
        // Bit 1, the ‘Reuse’ bit, encodes the value of the CreationMode reuse field.
        // Bit 2, the ‘Replace’ bit, encodes the value of the CreationMode replace field.
        Status result_status = client->create(creation_mode, create_payload);
    }
    else
    {
        
    }
}

void Agent::on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const DELETE_PAYLOAD& delete_payload)
{
    if (ProxyClient* client = get_client(header.client_key()))
    {
        Status result_status = client->delete_object(delete_payload);
    }
    else
    {
        std::cerr << "Write message rejected" << std::endl;
    }
}

void Agent::on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const WRITE_DATA_PAYLOAD&  write_payload)
{
    if (ProxyClient* client = get_client(header.client_key()))
    {
        Status result_status = client->write(write_payload.object_id(), write_payload);
    }
    else
    {
        std::cerr << "Write message rejected" << std::endl;
    }
}

void Agent::on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const READ_DATA_PAYLOAD&   read_payload)
{
    if (ProxyClient* client = get_client(header.client_key()))
    {
        Status result_status = client->read(read_payload.object_id(), read_payload);
    }
    else
    {
        std::cerr << "Read message rejected" << std::endl;
    }
} 

ProxyClient* Agent::get_client(int32_t client_key)
{
    try
    {
        return &clients_.at(client_key);
    } catch (const std::out_of_range& e)
    {
        std::cerr << "Client " << client_key << "not found" << std::endl;
        return nullptr;
    }
}
